#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "app/app_types.h"

typedef struct {
    uint16_t sample_rate_hz;
    uint8_t led_red_pa;
    uint8_t led_ir_pa;
    bool fifo_rollover;
} max3010x_config_t;

typedef struct {
    uint8_t write_pointer;
    uint8_t read_pointer;
    uint8_t overflow_counter;
    uint8_t available_samples;
} max3010x_fifo_status_t;

typedef struct {
    uint8_t i2c_addr;
    bool initialized;
    bool identified;
    uint8_t part_id;
    uint8_t revision_id;
    uint16_t sample_rate_hz;
    uint32_t fifo_overflow_events;
} max3010x_t;

esp_err_t max3010x_init(max3010x_t *dev, uint8_t i2c_addr);
esp_err_t max3010x_probe(max3010x_t *dev);
esp_err_t max3010x_reset(max3010x_t *dev);
esp_err_t max3010x_configure(max3010x_t *dev, const max3010x_config_t *config);
esp_err_t max3010x_config_default(max3010x_t *dev);
esp_err_t max3010x_flush_fifo(max3010x_t *dev);
esp_err_t max3010x_get_fifo_status(max3010x_t *dev, max3010x_fifo_status_t *status);
esp_err_t max3010x_read_fifo(max3010x_t *dev, ppg_sample_t *samples, size_t capacity, size_t *out_count);
esp_err_t max3010x_read_sample(max3010x_t *dev, ppg_sample_t *sample);
