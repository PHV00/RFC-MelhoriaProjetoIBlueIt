#pragma once

#include <stddef.h>

#include "common/sqi_types.h"
#include "processing/sqi/signal_quality_types.h"
#include "sensing/sample_buffer.h"

sqi_eval_status_t signal_quality_evaluate(
    const sample_buffer_t *buffer,
    signal_quality_t *out_quality
);

sqi_eval_status_t signal_quality_evaluate_window(
    const sample_buffer_t *buffer,
    size_t window_samples,
    float expected_sample_rate_hz,
    const sqi_config_t *config,
    signal_quality_t *out_quality
);
