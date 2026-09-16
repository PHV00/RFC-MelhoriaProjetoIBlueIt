#include "processing/hr_estimator.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define HR_MIN_BPM 35.0f
#define HR_MAX_BPM 200.0f
#define MAX_PEAKS 32u

static ppg_sample_t s_samples[SAMPLE_BUFFER_SIZE];
static float s_signal[SAMPLE_BUFFER_SIZE];
static float s_filtered[SAMPLE_BUFFER_SIZE];
static uint32_t s_peak_times[MAX_PEAKS];
static float s_intervals[MAX_PEAKS];

static int compare_float(const void *a, const void *b) {
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
}

static float clamp01(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

static void detrend_ir(const ppg_sample_t *samples, size_t n, float *out) {
    double sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
    double center = ((double)n - 1.0) * 0.5;
    for (size_t i = 0; i < n; ++i) {
        double x = (double)i - center;
        double y = (double)samples[i].ir;
        sum_y += y;
        sum_xy += x * y;
        sum_xx += x * x;
    }
    double mean = sum_y / (double)n;
    double slope = sum_xx > 1e-12 ? sum_xy / sum_xx : 0.0;
    for (size_t i = 0; i < n; ++i) {
        double x = (double)i - center;
        out[i] = (float)((double)samples[i].ir - (mean + slope * x));
    }
}

static void moving_average_5(const float *in, float *out, size_t n) {
    if (n == 0u) return;
    for (size_t i = 0; i < n; ++i) {
        size_t begin = i > 2u ? i - 2u : 0u;
        size_t end = i + 2u < n ? i + 2u : n - 1u;
        float sum = 0.0f;
        for (size_t j = begin; j <= end; ++j) sum += in[j];
        out[i] = sum / (float)(end - begin + 1u);
    }
}

static size_t detect_peaks(const ppg_sample_t *samples, const float *x, size_t n, float threshold, bool invert) {
    size_t count = 0u;
    uint32_t min_distance_ms = (uint32_t)(60000.0f / HR_MAX_BPM);

    for (size_t i = 2u; i + 2u < n; ++i) {
        float value = invert ? -x[i] : x[i];
        float left = invert ? -x[i - 1u] : x[i - 1u];
        float right = invert ? -x[i + 1u] : x[i + 1u];
        if (value < threshold || value <= left || value < right) continue;

        uint32_t ts = samples[i].timestamp_ms;
        if (count > 0u && (uint32_t)(ts - s_peak_times[count - 1u]) < min_distance_ms) {
            continue;
        }
        if (count < MAX_PEAKS) s_peak_times[count++] = ts;
    }
    return count;
}

bool hr_estimator_compute(const sample_buffer_t *buffer, const signal_quality_t *quality, hr_result_t *out_hr) {
    if (out_hr == NULL) return false;
    memset(out_hr, 0, sizeof(*out_hr));
    out_hr->status = ESTIMATOR_INVALID_ARGUMENT;

    if (buffer == NULL || quality == NULL) return false;
    if (!quality->signal_present) {
        out_hr->status = ESTIMATOR_NO_SIGNAL;
        return false;
    }
    if (quality->quality_score < 0.40f) {
        out_hr->status = ESTIMATOR_LOW_QUALITY;
        return false;
    }

    size_t requested = quality->window_samples;
    if (requested < 200u || requested > SAMPLE_BUFFER_SIZE) {
        out_hr->status = ESTIMATOR_NOT_READY;
        return false;
    }
    size_t n = 0u;
    if (!sample_buffer_copy_latest(buffer, s_samples, requested, &n) || n < requested) {
        out_hr->status = ESTIMATOR_NOT_READY;
        return false;
    }

    detrend_ir(s_samples, n, s_signal);
    moving_average_5(s_signal, s_filtered, n);

    double sum_sq = 0.0;
    for (size_t i = 0; i < n; ++i) sum_sq += (double)s_filtered[i] * (double)s_filtered[i];
    float stddev = (float)sqrt(sum_sq / (double)n);
    if (stddev < 1e-3f) {
        out_hr->status = ESTIMATOR_NO_SIGNAL;
        return false;
    }

    float threshold = 0.25f * stddev;
    size_t positive = detect_peaks(s_samples, s_filtered, n, threshold, false);
    uint32_t positive_times[MAX_PEAKS];
    memcpy(positive_times, s_peak_times, positive * sizeof(uint32_t));
    size_t negative = detect_peaks(s_samples, s_filtered, n, threshold, true);
    if (positive >= negative) {
        memcpy(s_peak_times, positive_times, positive * sizeof(uint32_t));
    }
    size_t peak_count = positive >= negative ? positive : negative;

    if (peak_count < 3u) {
        out_hr->status = ESTIMATOR_NOT_READY;
        return false;
    }

    size_t interval_count = 0u;
    float min_ibi = 60000.0f / HR_MAX_BPM;
    float max_ibi = 60000.0f / HR_MIN_BPM;
    for (size_t i = 1u; i < peak_count; ++i) {
        float dt = (float)(uint32_t)(s_peak_times[i] - s_peak_times[i - 1u]);
        if (dt >= min_ibi && dt <= max_ibi && interval_count < MAX_PEAKS) {
            s_intervals[interval_count++] = dt;
        }
    }
    if (interval_count < 2u) {
        out_hr->status = ESTIMATOR_OUT_OF_RANGE;
        return false;
    }

    qsort(s_intervals, interval_count, sizeof(float), compare_float);
    float median = interval_count % 2u
        ? s_intervals[interval_count / 2u]
        : 0.5f * (s_intervals[interval_count / 2u - 1u] + s_intervals[interval_count / 2u]);

    double mean = 0.0;
    for (size_t i = 0; i < interval_count; ++i) mean += s_intervals[i];
    mean /= (double)interval_count;
    double variance = 0.0;
    for (size_t i = 0; i < interval_count; ++i) {
        double d = (double)s_intervals[i] - mean;
        variance += d * d;
    }
    variance /= (double)interval_count;
    float cv = mean > 1e-6 ? (float)(sqrt(variance) / mean) : 1.0f;
    float bpm = 60000.0f / median;

    if (!isfinite(bpm) || bpm < HR_MIN_BPM || bpm > HR_MAX_BPM) {
        out_hr->status = ESTIMATOR_OUT_OF_RANGE;
        return false;
    }

    out_hr->valid = true;
    out_hr->status = ESTIMATOR_OK;
    out_hr->bpm = bpm;
    out_hr->ibi_ms = median;
    out_hr->peak_count = (uint16_t)peak_count;
    out_hr->confidence = quality->quality_score * clamp01(1.0f - cv / 0.20f);
    return true;
}
