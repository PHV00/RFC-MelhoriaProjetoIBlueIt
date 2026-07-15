#include "sensing/ppg_sampler.h"

#include <string.h>

#include "esp_timer.h"

#define MAX_FIFO_BATCH 32u

void ppg_sampler_init(ppg_sampler_t *sampler, max3010x_t *sensor, sample_buffer_t *buffer, uint16_t sample_rate_hz) {
    if (sampler == NULL) return;
    memset(sampler, 0, sizeof(*sampler));
    sampler->sensor = sensor;
    sampler->buffer = buffer;
    sampler->sample_rate_hz = sample_rate_hz;
}

esp_err_t ppg_sampler_poll(ppg_sampler_t *sampler, size_t *out_added) {
    if (out_added != NULL) *out_added = 0u;
    if (sampler == NULL || sampler->sensor == NULL || sampler->buffer == NULL ||
        sampler->sample_rate_hz == 0u || out_added == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ppg_sample_t batch[MAX_FIFO_BATCH] = {0};
    size_t count = 0u;
    esp_err_t err = max3010x_read_fifo(sampler->sensor, batch, MAX_FIFO_BATCH, &count);
    if (err == ESP_ERR_INVALID_STATE) {
        sampler->fifo_overflow_events = sampler->sensor->fifo_overflow_events;
        return err;
    }
    if (err != ESP_OK) {
        sampler->read_errors++;
        return err;
    }
    if (count == 0u) return ESP_OK;

    uint32_t period_ms = (1000u + sampler->sample_rate_hz / 2u) / sampler->sample_rate_hz;
    uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    uint32_t first_ts = now_ms - (uint32_t)(count - 1u) * period_ms;
    if (sampler->last_timestamp_ms != 0u &&
        (int32_t)(first_ts - sampler->last_timestamp_ms) <= 0) {
        first_ts = sampler->last_timestamp_ms + period_ms;
    }

    for (size_t i = 0u; i < count; ++i) {
        batch[i].timestamp_ms = first_ts + (uint32_t)i * period_ms;
        batch[i].seq = ++sampler->seq;
        if (!sample_buffer_push(sampler->buffer, &batch[i])) return ESP_FAIL;
        sampler->last_timestamp_ms = batch[i].timestamp_ms;
    }
    *out_added = count;
    return ESP_OK;
}

bool ppg_sampler_step(ppg_sampler_t *sampler) {
    size_t added = 0u;
    return ppg_sampler_poll(sampler, &added) == ESP_OK;
}
