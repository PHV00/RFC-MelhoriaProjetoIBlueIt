#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "app/app_types.h"

// define o tamanho do buffer circular de amostras
#define SAMPLE_BUFFER_SIZE 128

typedef struct {
    ppg_sample_t data[SAMPLE_BUFFER_SIZE];
    size_t head;
    size_t count;
} sample_buffer_t;

void sample_buffer_init(sample_buffer_t *buffer);
bool sample_buffer_push(sample_buffer_t *buffer, const ppg_sample_t *sample);
size_t sample_buffer_count(const sample_buffer_t *buffer);
bool sample_buffer_latest(const sample_buffer_t *buffer, ppg_sample_t *sample);
bool sample_buffer_get_oldest_first(const sample_buffer_t *buffer, size_t index, ppg_sample_t *sample);