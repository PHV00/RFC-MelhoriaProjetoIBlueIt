#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/sqi_types.h"

typedef struct {
    uint32_t timestamp_ms;
    uint32_t seq;
    uint32_t ir;
    uint32_t red;
} ppg_sample_t;

typedef enum {
    ESTIMATOR_OK = 0,
    ESTIMATOR_OK_UNCALIBRATED,
    ESTIMATOR_NOT_READY,
    ESTIMATOR_LOW_QUALITY,
    ESTIMATOR_NO_SIGNAL,
    ESTIMATOR_OUT_OF_RANGE,
    ESTIMATOR_INVALID_ARGUMENT,
    ESTIMATOR_INTERNAL_ERROR
} estimator_status_t;

typedef enum {
    PPG_INVALID_NONE              = 0,
    PPG_INVALID_NO_SIGNAL         = 1u << 0,
    PPG_INVALID_WINDOW_SHORT      = 1u << 1,
    PPG_INVALID_DISCONTINUITY     = 1u << 2,
    PPG_INVALID_CLIPPING          = 1u << 3,
    PPG_INVALID_LOW_PERFUSION     = 1u << 4,
    PPG_INVALID_LOW_CORRELATION   = 1u << 5,
    PPG_INVALID_FIFO_OVERFLOW     = 1u << 6,
    PPG_INVALID_CALIBRATION       = 1u << 7,
    PPG_INVALID_DOMAIN            = 1u << 8
} ppg_invalid_reason_t;

typedef struct {
    sqi_eval_status_t eval_status;
    ppg_quality_state_t state;
    sqi_gate_id_t failed_gate;
    sqi_fail_reason_t fail_reason;
    g1_integrity_result_t g1;

    bool signal_present;
    uint32_t invalid_reasons;
    size_t window_samples;
    float sample_rate_hz;
    float window_duration_s;
    float dc_ir;
    float dc_red;
    float ac_ir;
    float ac_red;
    float noise;
    float snr;
    float perfusion_index;
    float red_ir_correlation;
    float continuity_score;
    float clipping_fraction;
    float quality_score;
} signal_quality_t;

typedef struct {
    bool valid;
    estimator_status_t status;
    float bpm;
    float ibi_ms;
    float confidence;
    uint16_t peak_count;
} hr_result_t;

typedef struct {
    bool valid;
    bool calibrated;
    estimator_status_t status;
    float spo2;
    float ratio_r;
    float confidence;
    uint32_t calibration_version;
} spo2_result_t;

typedef struct {
    bool valid;
    bool clinical_valid;
    bool finger_present;
    uint32_t timestamp_ms;
    uint32_t invalid_reasons;
    uint32_t fifo_overflow_events;
    float confidence;
    hr_result_t hr;
    spo2_result_t spo2;
    signal_quality_t quality;
} health_frame_t;
