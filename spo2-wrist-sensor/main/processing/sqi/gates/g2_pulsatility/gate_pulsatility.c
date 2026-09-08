#include "processing/sqi/gates/g2_pulsatility/gate_pulsatility.h"

#include <math.h>
#include <string.h>

#include "processing/sqi/features/autocorrelation.h"
#include "processing/sqi/features/threshold_crossing.h"

static float rms(const float *samples, size_t count) {
    if (samples == NULL || count == 0u) return 0.0f;

    double sum_sq = 0.0;
    for (size_t i = 0u; i < count; ++i) {
        sum_sq += (double)samples[i] * (double)samples[i];
    }
    return (float)sqrt(sum_sq / (double)count);
}

static bool config_is_valid(const g2_pulsatility_config_t *config) {
    return config != NULL &&
           config->minimum_ac_rms >= 0.0f &&
           config->minimum_crossings <= config->maximum_crossings &&
           config->minimum_acf_peak >= -1.0f &&
           config->minimum_acf_peak <= 1.0f &&
           config->minimum_pulse_bpm > 0.0f &&
           config->maximum_pulse_bpm > config->minimum_pulse_bpm;
}

static bool evaluate_channel(
    const float *samples,
    size_t count,
    float sample_rate_hz,
    const g2_pulsatility_config_t *config,
    float *out_rms,
    uint32_t *out_crossings,
    float *out_acf_peak,
    size_t *out_best_lag,
    float *out_period_bpm,
    uint32_t rms_bit,
    uint32_t crossings_bit,
    uint32_t acf_bit,
    uint32_t period_bit,
    uint32_t *io_failure_mask
) {
    *out_rms = rms(samples, count);
    if (*out_rms < config->minimum_ac_rms) {
        *io_failure_mask |= rms_bit;
    }

    if (!threshold_crossing_count(samples, count, 0.0f, out_crossings)) {
        return false;
    }
    if (*out_crossings < config->minimum_crossings ||
        *out_crossings > config->maximum_crossings) {
        *io_failure_mask |= crossings_bit;
    }

    const float minimum_period_s = 60.0f / config->maximum_pulse_bpm;
    const float maximum_period_s = 60.0f / config->minimum_pulse_bpm;

    size_t min_lag = (size_t)ceilf(minimum_period_s * sample_rate_hz);
    size_t max_lag = (size_t)floorf(maximum_period_s * sample_rate_hz);
    if (min_lag < 1u) min_lag = 1u;
    if (max_lag >= count) max_lag = count - 1u;
    if (min_lag > max_lag) {
        return false;
    }

    if (!autocorrelation_best_lag(samples, count, min_lag, max_lag,
                                  out_best_lag, out_acf_peak)) {
        return false;
    }

    if (*out_acf_peak < config->minimum_acf_peak) {
        *io_failure_mask |= acf_bit;
    }

    *out_period_bpm = *out_best_lag > 0u
        ? (60.0f * sample_rate_hz / (float)*out_best_lag)
        : 0.0f;

    if (*out_period_bpm < config->minimum_pulse_bpm ||
        *out_period_bpm > config->maximum_pulse_bpm) {
        *io_failure_mask |= period_bit;
    }

    return true;
}

bool gate_pulsatility_evaluate(
    const float *red_processed,
    const float *ir_processed,
    size_t count,
    float sample_rate_hz,
    const g2_pulsatility_config_t *config,
    g2_pulsatility_result_t *out_result
) {
    if (red_processed == NULL || ir_processed == NULL || out_result == NULL ||
        count < 3u || sample_rate_hz <= 0.0f || !config_is_valid(config)) {
        return false;
    }

    memset(out_result, 0, sizeof(*out_result));

    if (!evaluate_channel(
            red_processed, count, sample_rate_hz, config,
            &out_result->red_ac_rms,
            &out_result->red_crossings,
            &out_result->red_acf_peak,
            &out_result->red_best_lag,
            &out_result->red_period_bpm,
            G2_FAILURE_LOW_RMS_RED,
            G2_FAILURE_CROSSINGS_RED,
            G2_FAILURE_LOW_ACF_RED,
            G2_FAILURE_PERIOD_RED,
            &out_result->failure_mask)) {
        return false;
    }

    if (!evaluate_channel(
            ir_processed, count, sample_rate_hz, config,
            &out_result->ir_ac_rms,
            &out_result->ir_crossings,
            &out_result->ir_acf_peak,
            &out_result->ir_best_lag,
            &out_result->ir_period_bpm,
            G2_FAILURE_LOW_RMS_IR,
            G2_FAILURE_CROSSINGS_IR,
            G2_FAILURE_LOW_ACF_IR,
            G2_FAILURE_PERIOD_IR,
            &out_result->failure_mask)) {
        return false;
    }

    out_result->passed = out_result->failure_mask == G2_FAILURE_NONE;
    return true;
}
