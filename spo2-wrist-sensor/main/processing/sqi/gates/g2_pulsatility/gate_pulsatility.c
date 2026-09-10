#include "processing/sqi/gates/g2_pulsatility/gate_pulsatility.h"

#include <math.h>
#include <string.h>

#include "processing/sqi/features/autocorrelation.h"
#include "processing/sqi/features/threshold_crossing.h"

static float rms(const float *samples, size_t count) {
    if (samples == NULL || count == 0u) return 0.0f;

    float sum_sq = 0.0f;
    for (size_t i = 0u; i < count; ++i) {
        sum_sq += samples[i] * samples[i];
    }
    return sqrtf(sum_sq / (float)count);
}

static bool config_is_valid(const g2_pulsatility_config_t *config) {
    return config != NULL &&
           config->minimum_ac_rms >= 0.0f &&
           config->minimum_crossings <= config->maximum_crossings &&
           config->minimum_acf_period_s > 0.0f &&
           config->maximum_acf_period_s > config->minimum_acf_period_s &&
           config->minimum_fzcp_s >= 0.0f &&
           config->maximum_fzcp_s > config->minimum_fzcp_s &&
           config->minimum_acf_peak >= -1.0f &&
           config->minimum_acf_peak <= 1.0f &&
           config->minimum_pulse_bpm > 0.0f &&
           config->maximum_pulse_bpm > config->minimum_pulse_bpm;
}

static sqi_fail_reason_t primary_reason_from_mask(uint32_t mask) {
    if ((mask & G2_FAILURE_LOW_RMS_RED) != 0u) return SQI_FAIL_LOW_AC_RMS_RED;
    if ((mask & G2_FAILURE_LOW_RMS_IR) != 0u) return SQI_FAIL_LOW_AC_RMS_IR;
    if ((mask & G2_FAILURE_CROSSINGS_RED) != 0u) return SQI_FAIL_CROSSINGS_RED;
    if ((mask & G2_FAILURE_CROSSINGS_IR) != 0u) return SQI_FAIL_CROSSINGS_IR;
    if ((mask & G2_FAILURE_FZCP_RED) != 0u) return SQI_FAIL_FZCP_RED;
    if ((mask & G2_FAILURE_FZCP_IR) != 0u) return SQI_FAIL_FZCP_IR;
    if ((mask & G2_FAILURE_LOW_ACF_RED) != 0u) return SQI_FAIL_LOW_ACF_RED;
    if ((mask & G2_FAILURE_LOW_ACF_IR) != 0u) return SQI_FAIL_LOW_ACF_IR;
    if ((mask & G2_FAILURE_PERIOD_RED) != 0u) return SQI_FAIL_PERIOD_RED;
    if ((mask & G2_FAILURE_PERIOD_IR) != 0u) return SQI_FAIL_PERIOD_IR;
    return SQI_FAIL_NONE;
}

static bool evaluate_channel(
    const float *samples,
    size_t count,
    float sample_rate_hz,
    const g2_pulsatility_config_t *config,
    float *scratch,
    float *out_rms,
    uint32_t *out_crossings,
    bool *out_has_fzcp,
    float *out_fzcp_s,
    float *out_acf_peak,
    size_t *out_best_lag,
    float *out_period_bpm,
    uint32_t rms_bit,
    uint32_t crossings_bit,
    uint32_t fzcp_bit,
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

    autocorrelation_ppg_shape_t shape = {0};
    if (!autocorrelation_ppg_shape_extract(
            samples,
            count,
            sample_rate_hz,
            config->minimum_acf_period_s,
            config->maximum_acf_period_s,
            scratch,
            &shape)) {
        return false;
    }

    *out_has_fzcp = shape.has_first_zero_crossing;
    *out_fzcp_s = shape.first_zero_crossing_s;
    *out_acf_peak = shape.peak_correlation;
    *out_best_lag = shape.peak_lag;
    *out_period_bpm = shape.peak_lag > 0u
        ? (60.0f * sample_rate_hz / (float)shape.peak_lag)
        : 0.0f;

    if (!shape.has_first_zero_crossing ||
        shape.first_zero_crossing_s < config->minimum_fzcp_s ||
        shape.first_zero_crossing_s > config->maximum_fzcp_s) {
        *io_failure_mask |= fzcp_bit;
    }

    if (shape.peak_correlation < config->minimum_acf_peak) {
        *io_failure_mask |= acf_bit;
    }

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
    float *scratch,
    g2_pulsatility_result_t *out_result
) {
    if (out_result == NULL) return false;
    memset(out_result, 0, sizeof(*out_result));
    out_result->primary_reason = SQI_FAIL_INVALID_ARGUMENT;

    if (red_processed == NULL || ir_processed == NULL || scratch == NULL ||
        count < 3u || sample_rate_hz <= 0.0f || !config_is_valid(config)) {
        return false;
    }

    if (!evaluate_channel(
            red_processed, count, sample_rate_hz, config, scratch,
            &out_result->red_ac_rms,
            &out_result->red_crossings,
            &out_result->red_has_fzcp,
            &out_result->red_fzcp_s,
            &out_result->red_acf_peak,
            &out_result->red_best_lag,
            &out_result->red_period_bpm,
            G2_FAILURE_LOW_RMS_RED,
            G2_FAILURE_CROSSINGS_RED,
            G2_FAILURE_FZCP_RED,
            G2_FAILURE_LOW_ACF_RED,
            G2_FAILURE_PERIOD_RED,
            &out_result->failure_mask)) {
        return false;
    }

    if (!evaluate_channel(
            ir_processed, count, sample_rate_hz, config, scratch,
            &out_result->ir_ac_rms,
            &out_result->ir_crossings,
            &out_result->ir_has_fzcp,
            &out_result->ir_fzcp_s,
            &out_result->ir_acf_peak,
            &out_result->ir_best_lag,
            &out_result->ir_period_bpm,
            G2_FAILURE_LOW_RMS_IR,
            G2_FAILURE_CROSSINGS_IR,
            G2_FAILURE_FZCP_IR,
            G2_FAILURE_LOW_ACF_IR,
            G2_FAILURE_PERIOD_IR,
            &out_result->failure_mask)) {
        return false;
    }

    out_result->passed = out_result->failure_mask == G2_FAILURE_NONE;
    out_result->primary_reason = primary_reason_from_mask(out_result->failure_mask);
    return true;
}
