#include "processing/sqi/features/autocorrelation.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool autocorrelation_best_lag(
    const float *samples,
    size_t count,
    size_t min_lag,
    size_t max_lag,
    size_t *out_best_lag,
    float *out_best_correlation
) {
    if (samples == NULL || out_best_lag == NULL || out_best_correlation == NULL ||
        count < 3u || min_lag == 0u || min_lag > max_lag || max_lag >= count) {
        return false;
    }

    float energy_a = 0.0f;
    float energy_b = 0.0f;

    const size_t initial_pairs = count - min_lag;
    for (size_t i = 0u; i < initial_pairs; ++i) {
        const float x = samples[i];
        energy_a += x * x;
    }
    for (size_t i = min_lag; i < count; ++i) {
        const float x = samples[i];
        energy_b += x * x;
    }

    size_t best_lag = min_lag;
    float best_correlation = -1.0f;

    for (size_t lag = min_lag; lag <= max_lag; ++lag) {
        if (lag > min_lag) {
            const size_t removed_from_a = count - lag;
            const size_t removed_from_b = lag - 1u;
            const float xa = samples[removed_from_a];
            const float xb = samples[removed_from_b];
            energy_a -= xa * xa;
            energy_b -= xb * xb;
            if (energy_a < 0.0f) energy_a = 0.0f;
            if (energy_b < 0.0f) energy_b = 0.0f;
        }

        const size_t paired_count = count - lag;
        float numerator = 0.0f;
        for (size_t i = 0u; i < paired_count; ++i) {
            numerator += samples[i] * samples[i + lag];
        }

        const float denominator = sqrtf(energy_a * energy_b);
        const float correlation = denominator > 1e-12f
            ? numerator / denominator
            : 0.0f;

        if (correlation > best_correlation) {
            best_correlation = correlation;
            best_lag = lag;
        }
    }

    *out_best_lag = best_lag;
    *out_best_correlation = best_correlation;
    return true;
}

bool autocorrelation_ppg_shape_extract(
    const float *samples,
    size_t count,
    float sample_rate_hz,
    float minimum_peak_period_s,
    float maximum_peak_period_s,
    float *scratch_windowed,
    autocorrelation_ppg_shape_t *out_shape
) {
    if (samples == NULL || scratch_windowed == NULL || out_shape == NULL ||
        count < 3u || sample_rate_hz <= 0.0f ||
        minimum_peak_period_s <= 0.0f ||
        maximum_peak_period_s <= minimum_peak_period_s) {
        return false;
    }

    memset(out_shape, 0, sizeof(*out_shape));

    float energy = 0.0f;
    const float denom = (float)(count - 1u);
    for (size_t i = 0u; i < count; ++i) {
        const float phase = 2.0f * (float)M_PI * (float)i / denom;
        const float w = 0.54f - 0.46f * cosf(phase);
        const float s = samples[i] * w;
        scratch_windowed[i] = s;
        energy += s * s;
    }
    if (energy <= 1e-12f) return true;

    size_t min_lag = (size_t)ceilf(minimum_peak_period_s * sample_rate_hz);
    size_t max_lag = (size_t)floorf(maximum_peak_period_s * sample_rate_hz);
    if (min_lag < 1u) min_lag = 1u;
    if (max_lag >= count) max_lag = count - 1u;
    if (min_lag > max_lag) return false;

    bool found_fzcp = false;
    float previous_r = 1.0f; /* R[0] = 1 por definição. */
    float best_r = -1.0f;
    size_t best_lag = min_lag;

    for (size_t lag = 1u; lag <= max_lag; ++lag) {
        float numerator = 0.0f;
        const size_t paired_count = count - lag;
        for (size_t i = 0u; i < paired_count; ++i) {
            numerator += scratch_windowed[i] * scratch_windowed[i + lag];
        }
        const float r = numerator / energy;

        if (!found_fzcp && previous_r > 0.0f && r <= 0.0f) {
            found_fzcp = true;
            out_shape->has_first_zero_crossing = true;
            out_shape->first_zero_crossing_lag = lag;
            out_shape->first_zero_crossing_s = (float)lag / sample_rate_hz;
        }

        if (lag >= min_lag && r > best_r) {
            best_r = r;
            best_lag = lag;
        }

        previous_r = r;
    }

    out_shape->peak_lag = best_lag;
    out_shape->peak_period_s = (float)best_lag / sample_rate_hz;
    out_shape->peak_correlation = best_r;
    return true;
}
