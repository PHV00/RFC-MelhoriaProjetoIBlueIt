#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "processing/sqi/gates/g1_integrity/gate_integrity.h"

#define TEST_SAMPLES 500u
#define ADC_MAX 0x03FFFFu

static ppg_sample_t samples[TEST_SAMPLES];

static g1_integrity_config_t default_config(void) {
    const g1_integrity_config_t cfg = {
        .adc_min_value = 0u,
        .adc_max_value = ADC_MAX,
        .rail_margin_counts = 1u,
        .minimum_mean_level = 5000u,
        .minimum_raw_range = 20u,
        .maximum_clipping_fraction = 0.01f,
        .minimum_continuity_fraction = 0.95f,
        .maximum_interval_deviation_fraction = 0.40f
    };
    return cfg;
}

static void fill_clean_window(void) {
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) {
        uint32_t phase = (uint32_t)(i % 20u);
        uint32_t pulse = phase < 10u ? phase * 20u : (20u - phase) * 20u;
        samples[i].timestamp_ms = (uint32_t)i * 10u;
        samples[i].seq = (uint32_t)i + 1u;
        samples[i].red = 60000u + pulse;
        samples[i].ir = 70000u + pulse * 2u;
    }
}

static bool evaluate_with_config(const g1_integrity_config_t *cfg, g1_integrity_result_t *result) {
    const sqi_window_t window = {
        .samples = samples,
        .count = TEST_SAMPLES,
        .expected_sample_rate_hz = 100.0f
    };
    return gate_integrity_evaluate(&window, cfg, result);
}

static g1_integrity_result_t evaluate(void) {
    g1_integrity_result_t result = {0};
    g1_integrity_config_t cfg = default_config();
    assert(evaluate_with_config(&cfg, &result));
    return result;
}

static void set_first_bad_intervals(size_t bad_count, uint32_t bad_dt_ms) {
    uint32_t timestamp = 0u;
    samples[0].timestamp_ms = timestamp;
    for (size_t i = 1u; i < TEST_SAMPLES; ++i) {
        timestamp += i <= bad_count ? bad_dt_ms : 10u;
        samples[i].timestamp_ms = timestamp;
    }
}

static void test_clean_window_passes(void) {
    fill_clean_window();
    const g1_integrity_result_t result = evaluate();
    assert(result.passed);
    assert(result.failure_mask == G1_FAILURE_NONE);
    assert(result.primary_reason == SQI_FAIL_NONE);
    assert(result.continuity_fraction == 1.0f);
}

static void test_no_signal_both_channels(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) {
        samples[i].red = 1000u + (uint32_t)(i % 50u);
        samples[i].ir = 1200u + (uint32_t)(i % 50u);
    }
    const g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert((result.failure_mask & G1_FAILURE_NO_SIGNAL_RED) != 0u);
    assert((result.failure_mask & G1_FAILURE_NO_SIGNAL_IR) != 0u);
    assert((result.failure_mask & G1_FAILURE_FLATLINE_RED) == 0u);
    assert((result.failure_mask & G1_FAILURE_FLATLINE_IR) == 0u);
    assert(result.primary_reason == SQI_FAIL_NO_SIGNAL_RED);
}

static void test_ir_no_signal_isolated(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) {
        samples[i].ir = 1000u + (uint32_t)(i % 50u);
    }
    const g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert(result.failure_mask == G1_FAILURE_NO_SIGNAL_IR);
    assert(result.primary_reason == SQI_FAIL_NO_SIGNAL_IR);
}

static void test_flatline_red_isolated(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) samples[i].red = 60000u;
    const g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert(result.failure_mask == G1_FAILURE_FLATLINE_RED);
    assert(result.primary_reason == SQI_FAIL_FLATLINE_RED);
}

static void test_flatline_both_channels(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) {
        samples[i].red = 120000u;
        samples[i].ir = 140000u;
    }
    const g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert(result.failure_mask == (G1_FAILURE_FLATLINE_RED | G1_FAILURE_FLATLINE_IR));
    assert(result.primary_reason == SQI_FAIL_FLATLINE_RED);
    assert(result.red_range == 0u);
    assert(result.ir_range == 0u);
}

static void test_clipping_red_isolated(void) {
    fill_clean_window();
    for (size_t i = 0u; i < 25u; ++i) samples[i * 20u].red = ADC_MAX;
    const g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert(result.failure_mask == G1_FAILURE_CLIPPING_RED);
    assert(result.primary_reason == SQI_FAIL_CLIPPING_RED);
    assert(result.red_clipping_fraction > 0.049f);
    assert(result.ir_clipping_fraction == 0.0f);
}

static void test_clipping_ir_isolated(void) {
    fill_clean_window();
    for (size_t i = 0u; i < 25u; ++i) samples[i * 20u].ir = ADC_MAX;
    const g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert(result.failure_mask == G1_FAILURE_CLIPPING_IR);
    assert(result.primary_reason == SQI_FAIL_CLIPPING_IR);
    assert(result.ir_clipping_fraction > 0.049f);
    assert(result.red_clipping_fraction == 0.0f);
}

static void test_clipping_exact_threshold_passes(void) {
    fill_clean_window();
    for (size_t i = 0u; i < 5u; ++i) samples[i * 100u].red = ADC_MAX;
    const g1_integrity_result_t result = evaluate();
    assert(result.red_clipping_fraction == 0.01f);
    assert(result.passed);
}

static void test_clipping_above_threshold_fails(void) {
    fill_clean_window();
    for (size_t i = 0u; i < 6u; ++i) samples[i * 80u].red = ADC_MAX;
    const g1_integrity_result_t result = evaluate();
    assert(result.red_clipping_fraction > 0.01f);
    assert(!result.passed);
    assert((result.failure_mask & G1_FAILURE_CLIPPING_RED) != 0u);
}

static void test_mean_exact_threshold_passes(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) {
        samples[i].red = (i % 2u == 0u) ? 4990u : 5010u;
    }
    const g1_integrity_result_t result = evaluate();
    assert(result.red_mean == 5000.0f);
    assert(result.red_range == 20u);
    assert((result.failure_mask & G1_FAILURE_NO_SIGNAL_RED) == 0u);
    assert((result.failure_mask & G1_FAILURE_FLATLINE_RED) == 0u);
    assert(result.passed);
}

static void test_mean_below_threshold_fails(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) {
        samples[i].red = (i % 2u == 0u) ? 4989u : 5009u;
    }
    const g1_integrity_result_t result = evaluate();
    assert(result.red_mean == 4999.0f);
    assert(!result.passed);
    assert((result.failure_mask & G1_FAILURE_NO_SIGNAL_RED) != 0u);
}

static void test_range_exact_threshold_passes(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) {
        samples[i].red = (i % 2u == 0u) ? 60000u : 60020u;
    }
    const g1_integrity_result_t result = evaluate();
    assert(result.red_range == 20u);
    assert((result.failure_mask & G1_FAILURE_FLATLINE_RED) == 0u);
    assert(result.passed);
}

static void test_range_below_threshold_fails(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) {
        samples[i].red = (i % 2u == 0u) ? 60000u : 60019u;
    }
    const g1_integrity_result_t result = evaluate();
    assert(result.red_range == 19u);
    assert(!result.passed);
    assert((result.failure_mask & G1_FAILURE_FLATLINE_RED) != 0u);
}

static void test_continuity_just_above_threshold_passes(void) {
    fill_clean_window();
    /* 475/499 valid intervals = 0.9519... */
    set_first_bad_intervals(24u, 20u);
    const g1_integrity_result_t result = evaluate();
    assert(result.continuity_fraction > 0.95f);
    assert(result.passed);
}

static void test_continuity_just_below_threshold_fails(void) {
    fill_clean_window();
    /* 474/499 valid intervals = 0.9498... */
    set_first_bad_intervals(25u, 20u);
    const g1_integrity_result_t result = evaluate();
    assert(result.continuity_fraction < 0.95f);
    assert(!result.passed);
    assert(result.failure_mask == G1_FAILURE_DISCONTINUITY);
    assert(result.primary_reason == SQI_FAIL_DISCONTINUITY);
}

static void test_duplicate_timestamps_are_counted(void) {
    fill_clean_window();
    set_first_bad_intervals(25u, 0u);
    const g1_integrity_result_t result = evaluate();
    assert(result.duplicate_timestamp_count == 25u);
    assert(result.discontinuity_count == 25u);
    assert(!result.passed);
    assert((result.failure_mask & G1_FAILURE_DISCONTINUITY) != 0u);
}

static void test_input_window_is_immutable(void) {
    ppg_sample_t before[TEST_SAMPLES];
    fill_clean_window();
    memcpy(before, samples, sizeof(before));
    (void)evaluate();
    assert(memcmp(before, samples, sizeof(before)) == 0);
}

static void test_invalid_config_returns_error(void) {
    fill_clean_window();
    g1_integrity_config_t cfg = default_config();
    cfg.maximum_clipping_fraction = 1.1f;
    g1_integrity_result_t result = {0};
    assert(!evaluate_with_config(&cfg, &result));
    assert(result.primary_reason == SQI_FAIL_INVALID_ARGUMENT);
}

int main(void) {
    test_clean_window_passes();
    test_no_signal_both_channels();
    test_ir_no_signal_isolated();
    test_flatline_red_isolated();
    test_flatline_both_channels();
    test_clipping_red_isolated();
    test_clipping_ir_isolated();
    test_clipping_exact_threshold_passes();
    test_clipping_above_threshold_fails();
    test_mean_exact_threshold_passes();
    test_mean_below_threshold_fails();
    test_range_exact_threshold_passes();
    test_range_below_threshold_fails();
    test_continuity_just_above_threshold_passes();
    test_continuity_just_below_threshold_fails();
    test_duplicate_timestamps_are_counted();
    test_input_window_is_immutable();
    test_invalid_config_returns_error();
    puts("G1 integrity tests: PASS (18 cases)");
    return 0;
}
