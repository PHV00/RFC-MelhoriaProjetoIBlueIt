#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "processing/sqi/signal_quality_types.h"
#include "sensing/sample_buffer.h"

bool signal_quality_evaluate(const sample_buffer_t *buffer, signal_quality_t *out_quality);
bool signal_quality_evaluate_window(
    const sample_buffer_t *buffer,
    size_t window_samples,
    float expected_sample_rate_hz,
    signal_quality_t *out_quality
);
