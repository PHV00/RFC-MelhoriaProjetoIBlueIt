#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SQI_EVAL_WAITING = 0,
    SQI_EVAL_COMPLETE,
    SQI_EVAL_ERROR
} sqi_eval_status_t;

typedef enum {
    PPG_QUALITY_UNKNOWN = 0,
    PPG_QUALITY_VALID,
    PPG_QUALITY_INVALID
} ppg_quality_state_t;

typedef enum {
    SQI_GATE_NONE = 0,
    SQI_GATE_G1_INTEGRITY,
    SQI_GATE_G2_PULSATILITY,
    SQI_GATE_G3_MORPHOLOGY,
    SQI_GATE_G4_CHANNELS
} sqi_gate_id_t;

typedef enum {
    SQI_FAIL_NONE = 0,
    SQI_FAIL_INVALID_ARGUMENT,
    SQI_FAIL_WINDOW_SHORT,
    SQI_FAIL_DISCONTINUITY,
    SQI_FAIL_NO_SIGNAL_RED,
    SQI_FAIL_NO_SIGNAL_IR,
    SQI_FAIL_FLATLINE_RED,
    SQI_FAIL_FLATLINE_IR,
    SQI_FAIL_CLIPPING_RED,
    SQI_FAIL_CLIPPING_IR
} sqi_fail_reason_t;

typedef enum {
    G1_FAILURE_NONE          = 0,
    G1_FAILURE_DISCONTINUITY = 1u << 0,
    G1_FAILURE_NO_SIGNAL_RED = 1u << 1,
    G1_FAILURE_NO_SIGNAL_IR  = 1u << 2,
    G1_FAILURE_FLATLINE_RED  = 1u << 3,
    G1_FAILURE_FLATLINE_IR   = 1u << 4,
    G1_FAILURE_CLIPPING_RED  = 1u << 5,
    G1_FAILURE_CLIPPING_IR   = 1u << 6
} g1_integrity_failure_t;

typedef struct {
    uint32_t adc_min_value;
    uint32_t adc_max_value;
    uint32_t rail_margin_counts;
    uint32_t minimum_mean_level;
    uint32_t minimum_raw_range;
    float maximum_clipping_fraction;
    float minimum_continuity_fraction;
    float maximum_interval_deviation_fraction;
} g1_integrity_config_t;

typedef struct {
    bool passed;
    uint32_t failure_mask;
    sqi_fail_reason_t primary_reason;

    uint32_t red_min;
    uint32_t red_max;
    uint32_t red_range;
    float red_mean;
    float red_clipping_fraction;

    uint32_t ir_min;
    uint32_t ir_max;
    uint32_t ir_range;
    float ir_mean;
    float ir_clipping_fraction;

    float continuity_fraction;
    uint32_t discontinuity_count;
    uint32_t duplicate_timestamp_count;
} g1_integrity_result_t;

typedef struct {
    uint32_t window_ms;
    uint32_t step_ms;
    float minimum_quality_score;
    g1_integrity_config_t g1_integrity;
} sqi_config_t;
