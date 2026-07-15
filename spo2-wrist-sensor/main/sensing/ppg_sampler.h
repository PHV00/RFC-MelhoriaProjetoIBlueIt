#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "drivers/max3010x_driver.h"
#include "sensing/sample_buffer.h"

typedef struct {
    max3010x_t *sensor;
    sample_buffer_t *buffer;
    uint32_t seq;
    uint32_t last_timestamp_ms;
    uint16_t sample_rate_hz;
    uint32_t fifo_overflow_events;
    uint32_t read_errors;
} ppg_sampler_t;

void ppg_sampler_init(ppg_sampler_t *sampler, max3010x_t *sensor, sample_buffer_t *buffer, uint16_t sample_rate_hz);
esp_err_t ppg_sampler_poll(ppg_sampler_t *sampler, size_t *out_added);
bool ppg_sampler_step(ppg_sampler_t *sampler);
