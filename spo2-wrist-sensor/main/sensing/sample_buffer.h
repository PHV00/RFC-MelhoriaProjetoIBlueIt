#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/measurement_types.h"

/* 5,12 s em 100 Hz: suficiente para uma janela fixa de 4 s e margem. */
#define SAMPLE_BUFFER_SIZE 512u

typedef struct {
    ppg_sample_t data[SAMPLE_BUFFER_SIZE];
    size_t head;
    size_t count;
    uint32_t overwritten_samples;
} sample_buffer_t;

void sample_buffer_init(sample_buffer_t *buffer);
bool sample_buffer_push(sample_buffer_t *buffer, const ppg_sample_t *sample);
size_t sample_buffer_count(const sample_buffer_t *buffer);
bool sample_buffer_latest(const sample_buffer_t *buffer, ppg_sample_t *sample);
bool sample_buffer_get_oldest_first(const sample_buffer_t *buffer, size_t index, ppg_sample_t *sample);
bool sample_buffer_copy_latest(const sample_buffer_t *buffer, ppg_sample_t *out, size_t requested, size_t *out_count);
uint32_t sample_buffer_overwritten(const sample_buffer_t *buffer);
