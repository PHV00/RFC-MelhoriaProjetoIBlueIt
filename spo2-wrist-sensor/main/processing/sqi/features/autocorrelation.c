#include "processing/sqi/features/autocorrelation.h"

#include <math.h>

static bool normalized_autocorrelation_at_lag(
    const float *samples,
    size_t count,
    size_t lag,
    float *out_value
) {
    if (samples == NULL || out_value == NULL || lag == 0u || lag >= count) {
        return false;
    }

    double numerator = 0.0;
    double energy_a = 0.0;
    double energy_b = 0.0;

    const size_t paired_count = count - lag;
    for (size_t i = 0u; i < paired_count; ++i) {
        const double a = (double)samples[i];
        const double b = (double)samples[i + lag];
        numerator += a * b;
        energy_a += a * a;
        energy_b += b * b;
    }

    const double denominator = sqrt(energy_a * energy_b);
    if (denominator <= 1e-12) {
        *out_value = 0.0f;
        return true;
    }

    *out_value = (float)(numerator / denominator);
    return true;
}

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

    size_t best_lag = min_lag;
    float best_correlation = -1.0f;

    for (size_t lag = min_lag; lag <= max_lag; ++lag) {
        float correlation = 0.0f;
        if (!normalized_autocorrelation_at_lag(samples, count, lag, &correlation)) {
            return false;
        }

        if (correlation > best_correlation) {
            best_correlation = correlation;
            best_lag = lag;
        }
    }

    *out_best_lag = best_lag;
    *out_best_correlation = best_correlation;
    return true;
}
