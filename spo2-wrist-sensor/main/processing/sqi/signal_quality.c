#include "processing/sqi/signal_quality.h"

#include <math.h>
#include <string.h>

#include "processing/sqi/gates/g1_integrity/gate_integrity.h"
#include "storage/config_repo.h"

#define MIN_DC_IR 5000.0f
#define MIN_AC_RMS 20.0f
#define MIN_PERFUSION_INDEX 0.0002f
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

static float maxf(float a, float b) {
    return a > b ? a : b;
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

static uint32_t legacy_reasons_from_g1(uint32_t failure_mask) {
    uint32_t reasons = PPG_INVALID_NONE;
    const uint32_t no_signal_mask = G1_FAILURE_NO_SIGNAL_RED |
                                    G1_FAILURE_NO_SIGNAL_IR |
                                    G1_FAILURE_FLATLINE_RED |
                                    G1_FAILURE_FLATLINE_IR;
    const uint32_t clipping_mask = G1_FAILURE_CLIPPING_RED | G1_FAILURE_CLIPPING_IR;

    if ((failure_mask & no_signal_mask) != 0u) reasons |= PPG_INVALID_NO_SIGNAL;
    if ((failure_mask & clipping_mask) != 0u) reasons |= PPG_INVALID_CLIPPING;
    if ((failure_mask & G1_FAILURE_DISCONTINUITY) != 0u) reasons |= PPG_INVALID_DISCONTINUITY;
    return reasons;
}

static bool g1_signal_present(const g1_integrity_result_t *g1) {
    const uint32_t no_signal_mask = G1_FAILURE_NO_SIGNAL_RED |
                                    G1_FAILURE_NO_SIGNAL_IR |
                                    G1_FAILURE_FLATLINE_RED |
                                    G1_FAILURE_FLATLINE_IR;
    return g1 != NULL && (g1->failure_mask & no_signal_mask) == 0u;
}

static void populate_g1_failure_quality(
    signal_quality_t *quality,
    const g1_integrity_result_t *g1,
    const ppg_sample_t *samples,
    size_t n,
    float sample_rate_hz
) {
    quality->eval_status = SQI_EVAL_COMPLETE;
    quality->state = PPG_QUALITY_INVALID;
    quality->failed_gate = SQI_GATE_G1_INTEGRITY;
    quality->fail_reason = g1->primary_reason;
    quality->g1 = *g1;
    quality->signal_present = g1_signal_present(g1);
    quality->invalid_reasons = legacy_reasons_from_g1(g1->failure_mask);
    quality->window_samples = n;
    quality->sample_rate_hz = sample_rate_hz;
    quality->window_duration_s = (float)(samples[n - 1u].timestamp_ms - samples[0].timestamp_ms) / 1000.0f;
    quality->dc_ir = g1->ir_mean;
    quality->dc_red = g1->red_mean;
    quality->continuity_score = g1->continuity_fraction;
    quality->clipping_fraction = maxf(g1->red_clipping_fraction, g1->ir_clipping_fraction);
    quality->quality_score = 0.0f;
}

sqi_eval_status_t signal_quality_evaluate_window(
    const sample_buffer_t *buffer,
    size_t window_samples,
    float expected_sample_rate_hz,
    const sqi_config_t *config,
    signal_quality_t *out_quality
) {
    if (out_quality == NULL) return SQI_EVAL_ERROR;
    memset(out_quality, 0, sizeof(*out_quality));
    out_quality->eval_status = SQI_EVAL_ERROR;
    out_quality->state = PPG_QUALITY_UNKNOWN;
    out_quality->failed_gate = SQI_GATE_NONE;
    out_quality->fail_reason = SQI_FAIL_INVALID_ARGUMENT;

    if (buffer == NULL || config == NULL || window_samples < 100u || window_samples > SAMPLE_BUFFER_SIZE || expected_sample_rate_hz <= 0.0f) {
        return SQI_EVAL_ERROR;
    }

    size_t n = 0u;
    if (!sample_buffer_copy_latest(buffer, s_samples, window_samples, &n) || n < window_samples) {
        out_quality->eval_status = SQI_EVAL_WAITING;
        out_quality->fail_reason = SQI_FAIL_WINDOW_SHORT;
        out_quality->invalid_reasons = PPG_INVALID_WINDOW_SHORT;
        out_quality->window_samples = n;
        return SQI_EVAL_WAITING;
    }

    const sqi_window_t window = {
        .samples = s_samples,
        .count = n,
        .expected_sample_rate_hz = expected_sample_rate_hz
    };
    g1_integrity_result_t g1 = {0};
    if (!gate_integrity_evaluate(&window, &config->g1_integrity, &g1)) {
        return SQI_EVAL_ERROR;
    }

    if (!g1.passed) {
        populate_g1_failure_quality(out_quality, &g1, s_samples, n, expected_sample_rate_hz);
        return SQI_EVAL_COMPLETE;
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
    float clipping_fraction = maxf(g1.red_clipping_fraction, g1.ir_clipping_fraction);
    float continuity = g1.continuity_fraction;

    uint32_t reasons = PPG_INVALID_NONE;
    bool signal_present = dc_ir >= MIN_DC_IR && ac_ir >= MIN_AC_RMS;
    if (!signal_present) reasons |= PPG_INVALID_NO_SIGNAL;
    if (pi < MIN_PERFUSION_INDEX) reasons |= PPG_INVALID_LOW_PERFUSION;
    if (fabsf(corr) < MIN_RED_IR_CORRELATION) reasons |= PPG_INVALID_LOW_CORRELATION;

    float score_pi = clamp01((pi - MIN_PERFUSION_INDEX) / (0.015f - MIN_PERFUSION_INDEX));
    float score_corr = clamp01((fabsf(corr) - MIN_RED_IR_CORRELATION) / (0.98f - MIN_RED_IR_CORRELATION));
    float score_snr = clamp01((snr - 2.0f) / 10.0f);
    float score_clip = config->g1_integrity.maximum_clipping_fraction > 1e-6f
        ? clamp01(1.0f - clipping_fraction / config->g1_integrity.maximum_clipping_fraction)
        : (clipping_fraction <= 0.0f ? 1.0f : 0.0f);
    float quality_score = 0.25f * score_pi + 0.25f * score_corr + 0.20f * score_snr +
                          0.20f * continuity + 0.10f * score_clip;
    if (!signal_present) quality_score = 0.0f;

    out_quality->eval_status = SQI_EVAL_COMPLETE;
    out_quality->state = PPG_QUALITY_VALID;
    out_quality->failed_gate = SQI_GATE_NONE;
    out_quality->fail_reason = SQI_FAIL_NONE;
    out_quality->g1 = g1;
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
    return SQI_EVAL_COMPLETE;
}

sqi_eval_status_t signal_quality_evaluate(const sample_buffer_t *buffer, signal_quality_t *out_quality) {
    const system_config_t *cfg = config_repo_get();
    if (cfg == NULL) return SQI_EVAL_ERROR;
    return signal_quality_evaluate_window(buffer, config_repo_sqi_window_samples(cfg),
                                          (float)cfg->sensor_sample_rate_hz, &cfg->sqi, out_quality);
}
