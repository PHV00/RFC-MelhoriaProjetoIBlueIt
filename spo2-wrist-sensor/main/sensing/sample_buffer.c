#include "sensing/sample_buffer.h"

#include <string.h>

void sample_buffer_init(sample_buffer_t *buffer) {
    if (buffer == NULL) return;
    memset(buffer, 0, sizeof(*buffer));
}

bool sample_buffer_push(sample_buffer_t *buffer, const ppg_sample_t *sample) {
    if (buffer == NULL || sample == NULL) return false;

    if (buffer->count == SAMPLE_BUFFER_SIZE) {
        buffer->overwritten_samples++;
    }

    buffer->data[buffer->head] = *sample;
    buffer->head = (buffer->head + 1u) % SAMPLE_BUFFER_SIZE;
    if (buffer->count < SAMPLE_BUFFER_SIZE) buffer->count++;
    return true;
}

size_t sample_buffer_count(const sample_buffer_t *buffer) {
    return buffer != NULL ? buffer->count : 0u;
}

bool sample_buffer_latest(const sample_buffer_t *buffer, ppg_sample_t *sample) {
    if (buffer == NULL || sample == NULL || buffer->count == 0u) return false;
    size_t last = (buffer->head + SAMPLE_BUFFER_SIZE - 1u) % SAMPLE_BUFFER_SIZE;
    *sample = buffer->data[last];
    return true;
}

bool sample_buffer_get_oldest_first(const sample_buffer_t *buffer, size_t index, ppg_sample_t *sample) {
    if (buffer == NULL || sample == NULL || index >= buffer->count) return false;
    size_t oldest = (buffer->head + SAMPLE_BUFFER_SIZE - buffer->count) % SAMPLE_BUFFER_SIZE;
    size_t pos = (oldest + index) % SAMPLE_BUFFER_SIZE;
    *sample = buffer->data[pos];
    return true;
}

bool sample_buffer_copy_latest(const sample_buffer_t *buffer, ppg_sample_t *out, size_t requested, size_t *out_count) {
    if (out_count != NULL) *out_count = 0u;
    if (buffer == NULL || out == NULL || requested == 0u || buffer->count == 0u) return false;

    size_t n = requested < buffer->count ? requested : buffer->count;
    size_t first_index = buffer->count - n;
    for (size_t i = 0; i < n; ++i) {
        if (!sample_buffer_get_oldest_first(buffer, first_index + i, &out[i])) return false;
    }
    if (out_count != NULL) *out_count = n;
    return true;
}

uint32_t sample_buffer_overwritten(const sample_buffer_t *buffer) {
    return buffer != NULL ? buffer->overwritten_samples : 0u;
}
