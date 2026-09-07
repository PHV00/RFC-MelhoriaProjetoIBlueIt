#include "sensing/ppg_sampler.h"

#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"

#define MAX_FIFO_BATCH 32u
#define MAX30102_ADC_MAX 0x03FFFFu

#ifndef SQI_G1_FAULT_MODE
#define SQI_G1_FAULT_MODE 0
#endif

#if SQI_G1_FAULT_MODE < 0 || SQI_G1_FAULT_MODE > 4
#error "SQI_G1_FAULT_MODE must be between 0 and 4"
#endif

static const char *TAG = "PPG_SAMPLER";

static const char *fault_mode_name(void) {
    switch (SQI_G1_FAULT_MODE) {
        case 1: return "FLATLINE_BOTH";
        case 2: return "CLIPPING_RED";
        case 3: return "CLIPPING_IR";
        case 4: return "DISCONTINUITY";
        case 0:
        default: return "OFF";
    }
}

/*
 * Fault injection exists only on the dedicated validation branch.
 * It is applied before the sample enters SampleBuffer so the complete path
 * acquisition -> G1 -> fail-fast -> telemetry is exercised.
 *
 * Modes 1-4 synthesize deterministic values to isolate one G1 failure at a
 * time. Mode 0 leaves the real MAX30102 samples untouched.
 */
static void apply_fault_injection(ppg_sampler_t *sampler, ppg_sample_t *sample) {
    if (sampler == NULL || sample == NULL || SQI_G1_FAULT_MODE == 0) return;

    const uint32_t phase = sample->seq % 100u;

    if (SQI_G1_FAULT_MODE == 1) {
        sample->red = 120000u;
        sample->ir = 140000u;
        return;
    }

    /* Clean synthetic optical levels keep mean/range above G1 minima. */
    sample->red = 120000u + phase;
    sample->ir = 140000u + phase;

    if (SQI_G1_FAULT_MODE == 2) {
        /* 1 in 20 samples = about 5% clipping on RED (> 1% threshold). */
        if ((sample->seq % 20u) == 0u) sample->red = MAX30102_ADC_MAX;
        return;
    }

    if (SQI_G1_FAULT_MODE == 3) {
        /* 1 in 20 samples = about 5% clipping on IR (> 1% threshold). */
        if ((sample->seq % 20u) == 0u) sample->ir = MAX30102_ADC_MAX;
        return;
    }

    if (SQI_G1_FAULT_MODE == 4) {
        /*
         * Every tenth sample duplicates the previous timestamp. The following
         * interval also becomes too long, so continuity falls well below 95%.
         * Optical values remain clean to isolate DISCONTINUITY.
         */
        if ((sample->seq % 10u) == 0u && sampler->last_timestamp_ms != 0u) {
            sample->timestamp_ms = sampler->last_timestamp_ms;
        }
    }
}

void ppg_sampler_init(ppg_sampler_t *sampler, max3010x_t *sensor, sample_buffer_t *buffer, uint16_t sample_rate_hz) {
    if (sampler == NULL) return;
    memset(sampler, 0, sizeof(*sampler));
    sampler->sensor = sensor;
    sampler->buffer = buffer;
    sampler->sample_rate_hz = sample_rate_hz;

    if (SQI_G1_FAULT_MODE != 0) {
        ESP_LOGW(TAG, "G1 VALIDATION: fault injection active: %s", fault_mode_name());
    }
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
        apply_fault_injection(sampler, &batch[i]);
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
