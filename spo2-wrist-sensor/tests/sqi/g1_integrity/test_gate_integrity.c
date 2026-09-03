#include <assert.h>
#include <stdio.h>

#include "processing/sqi/gates/g1_integrity/gate_integrity.h"

#define TEST_SAMPLES 100u

static ppg_sample_t samples[TEST_SAMPLES];

static g1_integrity_config_t default_config(void) {
    const g1_integrity_config_t cfg = {
        .adc_min_value = 0u,
        .adc_max_value = 0x03FFFFu,
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
        samples[i].seq = (uint32_t)i;
        samples[i].red = 60000u + pulse;
        samples[i].ir = 70000u + pulse * 2u;
    }
}

static g1_integrity_result_t evaluate(void) {
    g1_integrity_result_t result = {0};
    g1_integrity_config_t cfg = default_config();
    sqi_window_t window = {
        .samples = samples,
        .count = TEST_SAMPLES,
        .expected_sample_rate_hz = 100.0f
    };
    assert(gate_integrity_evaluate(&window, &cfg, &result));
    return result;
}

static void test_clean_window_passes(void) {
    fill_clean_window();
    g1_integrity_result_t result = evaluate();
    assert(result.passed);
    assert(result.failure_mask == G1_FAILURE_NONE);
    assert(result.primary_reason == SQI_FAIL_NONE);
    assert(result.continuity_fraction == 1.0f);
}

static void test_discontinuity_fails(void) {
    fill_clean_window();
    for (size_t i = 50u; i < TEST_SAMPLES; ++i) {
        samples[i].timestamp_ms += (uint32_t)(i - 49u) * 100u;
    }
    g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert((result.failure_mask & G1_FAILURE_DISCONTINUITY) != 0u);
    assert(result.primary_reason == SQI_FAIL_DISCONTINUITY);
}

static void test_red_flatline_fails(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) samples[i].red = 60000u;
    g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert((result.failure_mask & G1_FAILURE_FLATLINE_RED) != 0u);
}

static void test_ir_no_signal_fails(void) {
    fill_clean_window();
    for (size_t i = 0u; i < TEST_SAMPLES; ++i) samples[i].ir = 1000u + (uint32_t)(i % 5u) * 10u;
    g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert((result.failure_mask & G1_FAILURE_NO_SIGNAL_IR) != 0u);
}

static void test_red_clipping_fails(void) {
    fill_clean_window();
    for (size_t i = 0u; i < 5u; ++i) samples[i].red = 0x03FFFFu;
    g1_integrity_result_t result = evaluate();
    assert(!result.passed);
    assert((result.failure_mask & G1_FAILURE_CLIPPING_RED) != 0u);
}

int main(void) {
    test_clean_window_passes();
    test_discontinuity_fails();
    test_red_flatline_fails();
    test_ir_no_signal_fails();
    test_red_clipping_fails();
    puts("G1 integrity tests: PASS");
    return 0;
}
