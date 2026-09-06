#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    float a;
    float b;
    float c;
    float min_ratio_r;
    float max_ratio_r;
    bool calibrated;
    bool allow_uncalibrated_estimate;
    uint32_t version;
} spo2_calibration_t;

typedef struct {
    uint32_t window_ms;
    uint32_t step_ms;
    float minimum_quality_score;
} sqi_config_t;

typedef struct {
    uint8_t sensor_i2c_addr;
    int i2c_sda_gpio;
    int i2c_scl_gpio;
    uint32_t i2c_freq_hz;
    uint16_t sensor_sample_rate_hz;
    uint16_t acquisition_poll_ms;
    uint8_t led_red_pa;
    uint8_t led_ir_pa;
    sqi_config_t sqi;
    spo2_calibration_t spo2_calibration;
} system_config_t;

const system_config_t *config_repo_get(void);
size_t config_repo_sqi_window_samples(const system_config_t *config);
