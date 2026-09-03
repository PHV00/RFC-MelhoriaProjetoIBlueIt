#include "processing/sqi/gates/g1_integrity/gate_integrity.h"

#include <string.h>

static bool config_is_valid(const g1_integrity_config_t *config) {
    if (config == NULL) return false;
    if (config->adc_max_value <= config->adc_min_value) return false;
    if (config->rail_margin_counts > (config->adc_max_value - config->adc_min_value) / 2u) return false;
    if (config->minimum_mean_level > config->adc_max_value) return false;
    if (config->minimum_raw_range > (config->adc_max_value - config->adc_min_value)) return false;
    if (config->maximum_clipping_fraction < 0.0f || config->maximum_clipping_fraction > 1.0f) return false;
    if (config->minimum_continuity_fraction < 0.0f || config->minimum_continuity_fraction > 1.0f) return false;
    if (config->maximum_interval_deviation_fraction < 0.0f || config->maximum_interval_deviation_fraction >= 1.0f) return false;
    return true;
}

static bool is_near_rail(uint32_t value, const g1_integrity_config_t *config) {
    uint32_t low_limit = config->adc_min_value + config->rail_margin_counts;
    uint32_t high_limit = config->adc_max_value - config->rail_margin_counts;
    return value <= low_limit || value >= high_limit;
}

static sqi_fail_reason_t primary_reason_from_mask(uint32_t mask) {
    if ((mask & G1_FAILURE_DISCONTINUITY) != 0u) return SQI_FAIL_DISCONTINUITY;
    if ((mask & G1_FAILURE_CLIPPING_RED) != 0u) return SQI_FAIL_CLIPPING_RED;
    if ((mask & G1_FAILURE_CLIPPING_IR) != 0u) return SQI_FAIL_CLIPPING_IR;
    if ((mask & G1_FAILURE_NO_SIGNAL_RED) != 0u) return SQI_FAIL_NO_SIGNAL_RED;
    if ((mask & G1_FAILURE_NO_SIGNAL_IR) != 0u) return SQI_FAIL_NO_SIGNAL_IR;
    if ((mask & G1_FAILURE_FLATLINE_RED) != 0u) return SQI_FAIL_FLATLINE_RED;
    if ((mask & G1_FAILURE_FLATLINE_IR) != 0u) return SQI_FAIL_FLATLINE_IR;
    return SQI_FAIL_NONE;
}

bool gate_integrity_evaluate(
    const sqi_window_t *window,
    const g1_integrity_config_t *config,
    g1_integrity_result_t *out_result
) {
    if (out_result == NULL) return false;
    memset(out_result, 0, sizeof(*out_result));
    out_result->primary_reason = SQI_FAIL_INVALID_ARGUMENT;

    if (window == NULL || window->samples == NULL || window->count < 2u ||
        window->expected_sample_rate_hz <= 0.0f || !config_is_valid(config)) {
        return false;
    }

    uint32_t red_min = UINT32_MAX;
    uint32_t red_max = 0u;
    uint32_t ir_min = UINT32_MAX;
    uint32_t ir_max = 0u;
    uint32_t red_clipping_count = 0u;
    uint32_t ir_clipping_count = 0u;
    double red_sum = 0.0;
    double ir_sum = 0.0;

    const float expected_dt_ms = 1000.0f / window->expected_sample_rate_hz;
    const float tolerance = config->maximum_interval_deviation_fraction;
    const float minimum_dt_ms = expected_dt_ms * (1.0f - tolerance);
    const float maximum_dt_ms = expected_dt_ms * (1.0f + tolerance);
    uint32_t continuous_intervals = 0u;
    uint32_t discontinuity_count = 0u;
    uint32_t duplicate_timestamp_count = 0u;

    for (size_t i = 0u; i < window->count; ++i) {
        const ppg_sample_t *sample = &window->samples[i];

        if (sample->red < red_min) red_min = sample->red;
        if (sample->red > red_max) red_max = sample->red;
        if (sample->ir < ir_min) ir_min = sample->ir;
        if (sample->ir > ir_max) ir_max = sample->ir;

        red_sum += (double)sample->red;
        ir_sum += (double)sample->ir;

        if (is_near_rail(sample->red, config)) red_clipping_count++;
        if (is_near_rail(sample->ir, config)) ir_clipping_count++;

        if (i > 0u) {
            uint32_t dt_ms = sample->timestamp_ms - window->samples[i - 1u].timestamp_ms;
            if (dt_ms == 0u) duplicate_timestamp_count++;

            if ((float)dt_ms >= minimum_dt_ms && (float)dt_ms <= maximum_dt_ms) {
                continuous_intervals++;
            } else {
                discontinuity_count++;
            }
        }
    }

    const float red_mean = (float)(red_sum / (double)window->count);
    const float ir_mean = (float)(ir_sum / (double)window->count);
    const uint32_t red_range = red_max - red_min;
    const uint32_t ir_range = ir_max - ir_min;
    const float red_clipping_fraction = (float)red_clipping_count / (float)window->count;
    const float ir_clipping_fraction = (float)ir_clipping_count / (float)window->count;
    const float continuity_fraction = (float)continuous_intervals / (float)(window->count - 1u);

    uint32_t failure_mask = G1_FAILURE_NONE;
    if (continuity_fraction < config->minimum_continuity_fraction) {
        failure_mask |= G1_FAILURE_DISCONTINUITY;
    }
    if (red_mean < (float)config->minimum_mean_level) {
        failure_mask |= G1_FAILURE_NO_SIGNAL_RED;
    }
    if (ir_mean < (float)config->minimum_mean_level) {
        failure_mask |= G1_FAILURE_NO_SIGNAL_IR;
    }
    if (red_range < config->minimum_raw_range) {
        failure_mask |= G1_FAILURE_FLATLINE_RED;
    }
    if (ir_range < config->minimum_raw_range) {
        failure_mask |= G1_FAILURE_FLATLINE_IR;
    }
    if (red_clipping_fraction > config->maximum_clipping_fraction) {
        failure_mask |= G1_FAILURE_CLIPPING_RED;
    }
    if (ir_clipping_fraction > config->maximum_clipping_fraction) {
        failure_mask |= G1_FAILURE_CLIPPING_IR;
    }

    out_result->passed = failure_mask == G1_FAILURE_NONE;
    out_result->failure_mask = failure_mask;
    out_result->primary_reason = primary_reason_from_mask(failure_mask);
    out_result->red_min = red_min;
    out_result->red_max = red_max;
    out_result->red_range = red_range;
    out_result->red_mean = red_mean;
    out_result->red_clipping_fraction = red_clipping_fraction;
    out_result->ir_min = ir_min;
    out_result->ir_max = ir_max;
    out_result->ir_range = ir_range;
    out_result->ir_mean = ir_mean;
    out_result->ir_clipping_fraction = ir_clipping_fraction;
    out_result->continuity_fraction = continuity_fraction;
    out_result->discontinuity_count = discontinuity_count;
    out_result->duplicate_timestamp_count = duplicate_timestamp_count;
    return true;
}
