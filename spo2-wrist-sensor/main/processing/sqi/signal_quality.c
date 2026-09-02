#include "processing/sqi/signal_quality.h"

#include <math.h>
#include <string.h>

#include "storage/config_repo.h"

#define PPG_ADC_MAX_VALUE 262143.0f
#define MIN_DC_IR 5000.0f
#define MIN_AC_RMS 20.0f
#define MIN_PERFUSION_INDEX 0.0002f
#define MAX_CLIPPING_FRACTION 0.01f
#define MIN_RED_IR_CORRELATION 0.60f

static ppg_sample_t s_samples[SAMPLE_BUFFER_SIZE];
static float s_ir[SAMPLE_BUFFER_SIZE];
static float s_red[SAMPLE_BUFFER_SIZE];
static float s_ir_smooth[SAMPLE_BUFFER_SIZE];

static float clamp01(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

static void detrend_linear(const ppg_sample_t *samples, size_t n, bool ir_channel, float *out, float *dc) {
    double sum_y = 0.0;
    double sum_x = 0.0;
    double sum_xx = 0.0;
    double sum_xy = 0.0;
    const double center = ((double)n - 1.0) * 0.5;

    for (size_t i = 0; i < n; ++i) {
        double x = (double)i - center;
        double y = ir_channel ? (double)samples[i].ir : (double)samples[i].red;
        sum_y += y;
        sum_x += x;
        sum_xx += x * x;
        sum_xy += x * y;
    }

    double mean = sum_y / (double)n;
    double denominator = sum_xx - (sum_x * sum_x / (double)n);
    double slope = fabs(denominator) > 1e-12
        ? (sum_xy - (sum_x * sum_y / (double)n)) / denominator
        : 0.0;

    for (size_t i = 0; i < n; ++i) {
        double x = (double)i - center;
        out[i] = (float)((ir_channel ? (double)samples[i].ir : (double)samples[i].red) - (mean + slope * x));
    }
    *dc = (float)mean;
}

static float rms(const float *x, size_t n) {
    if (x == NULL || n == 0u) return 0.0f;
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) sum += (double)x[i] * (double)x[i];
    return (float)sqrt(sum / (double)n);
}

static float correlation(const float *a, const float *b, size_t n) {
    if (a == NULL || b == NULL || n < 2u) return 0.0f;
    double aa = 0.0, bb = 0.0, ab = 0.0;
    for (size_t i = 0; i < n; ++i) {
        aa += (double)a[i] * (double)a[i];
        bb += (double)b[i] * (double)b[i];
        ab += (double)a[i] * (double)b[i];
    }
    double den = sqrt(aa * bb);
    return den > 1e-12 ? (float)(ab / den) : 0.0f;
}

static float residual_noise_rms(const float *x, size_t n) {
    if (n < 5u) return 0.0f;
    memset(s_ir_smooth, 0, n * sizeof(s_ir_smooth[0]));
    double residual_sum = 0.0;
    size_t used = 0u;
    for (size_t i = 2u; i + 2u < n; ++i) {
        float smooth = (x[i - 2u] + x[i - 1u] + x[i] + x[i + 1u] + x[i + 2u]) / 5.0f;
        s_ir_smooth[i] = smooth;
        float residual = x[i] - smooth;
        residual_sum += (double)residual * (double)residual;
        used++;
    }
    return used > 0u ? (float)sqrt(residual_sum / (double)used) : 0.0f;
}

bool signal_quality_evaluate_window(
    const sample_buffer_t *buffer,
    size_t window_samples,
    float expected_sample_rate_hz,
    signal_quality_t *out_quality
) {
    if (out_quality == NULL) return false;
    memset(out_quality, 0, sizeof(*out_quality));

    if (buffer == NULL || window_samples < 100u || window_samples > SAMPLE_BUFFER_SIZE || expected_sample_rate_hz <= 0.0f) {
        out_quality->invalid_reasons = PPG_INVALID_WINDOW_SHORT;
        return false;
    }

    size_t n = 0u;
    if (!sample_buffer_copy_latest(buffer, s_samples, window_samples, &n) || n < window_samples) {
        out_quality->window_samples = n;
        out_quality->invalid_reasons = PPG_INVALID_WINDOW_SHORT;
        return false;
    }

    float dc_ir = 0.0f, dc_red = 0.0f;
    detrend_linear(s_samples, n, true, s_ir, &dc_ir);
    detrend_linear(s_samples, n, false, s_red, &dc_red);

    float ac_ir = rms(s_ir, n);
    float ac_red = rms(s_red, n);
    float noise = residual_noise_rms(s_ir, n);
    float snr = noise > 1e-6f ? ac_ir / noise : 100.0f;
    float corr = correlation(s_ir, s_red, n);
    float pi = dc_ir > 1e-6f ? ac_ir / dc_ir : 0.0f;

    size_t clipping_count = 0u;
    size_t continuous_count = 0u;
    const float expected_dt_ms = 1000.0f / expected_sample_rate_hz;
    for (size_t i = 0u; i < n; ++i) {
        if (s_samples[i].ir <= 1u || s_samples[i].red <= 1u ||
            (float)s_samples[i].ir >= PPG_ADC_MAX_VALUE - 1.0f ||
            (float)s_samples[i].red >= PPG_ADC_MAX_VALUE - 1.0f) {
            clipping_count++;
        }
        if (i > 0u) {
            uint32_t dt = s_samples[i].timestamp_ms - s_samples[i - 1u].timestamp_ms;
            if ((float)dt >= expected_dt_ms * 0.60f && (float)dt <= expected_dt_ms * 1.40f) continuous_count++;
        }
    }

    float clipping_fraction = (float)clipping_count / (float)n;
    float continuity = n > 1u ? (float)continuous_count / (float)(n - 1u) : 0.0f;

    uint32_t reasons = PPG_INVALID_NONE;
    bool signal_present = dc_ir >= MIN_DC_IR && ac_ir >= MIN_AC_RMS;
    if (!signal_present) reasons |= PPG_INVALID_NO_SIGNAL;
    if (pi < MIN_PERFUSION_INDEX) reasons |= PPG_INVALID_LOW_PERFUSION;
    if (fabsf(corr) < MIN_RED_IR_CORRELATION) reasons |= PPG_INVALID_LOW_CORRELATION;
    if (clipping_fraction > MAX_CLIPPING_FRACTION) reasons |= PPG_INVALID_CLIPPING;
    if (continuity < 0.95f) reasons |= PPG_INVALID_DISCONTINUITY;

    float score_pi = clamp01((pi - MIN_PERFUSION_INDEX) / (0.015f - MIN_PERFUSION_INDEX));
    float score_corr = clamp01((fabsf(corr) - MIN_RED_IR_CORRELATION) / (0.98f - MIN_RED_IR_CORRELATION));
    float score_snr = clamp01((snr - 2.0f) / 10.0f);
    float score_clip = clamp01(1.0f - clipping_fraction / MAX_CLIPPING_FRACTION);
    float quality_score = 0.25f * score_pi + 0.25f * score_corr + 0.20f * score_snr +
                          0.20f * continuity + 0.10f * score_clip;
    if (!signal_present) quality_score = 0.0f;

    out_quality->signal_present = signal_present;
    out_quality->invalid_reasons = reasons;
    out_quality->window_samples = n;
    out_quality->sample_rate_hz = expected_sample_rate_hz;
    out_quality->window_duration_s = (float)(s_samples[n - 1u].timestamp_ms - s_samples[0].timestamp_ms) / 1000.0f;
    out_quality->dc_ir = dc_ir;
    out_quality->dc_red = dc_red;
    out_quality->ac_ir = ac_ir;
    out_quality->ac_red = ac_red;
    out_quality->noise = noise;
    out_quality->snr = snr;
    out_quality->perfusion_index = pi;
    out_quality->red_ir_correlation = corr;
    out_quality->continuity_score = continuity;
    out_quality->clipping_fraction = clipping_fraction;
    out_quality->quality_score = clamp01(quality_score);
    return true;
}

bool signal_quality_evaluate(const sample_buffer_t *buffer, signal_quality_t *out_quality) {
    const system_config_t *cfg = config_repo_get();
    return signal_quality_evaluate_window(buffer, config_repo_sqi_window_samples(cfg),
                                          (float)cfg->sensor_sample_rate_hz, out_quality);
}
