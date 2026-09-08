#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "common/measurement_types.h"
#include "common/sqi_types.h"

typedef struct {
    const ppg_sample_t *samples;
    size_t count;
    float expected_sample_rate_hz;
} sqi_window_t;

bool gate_integrity_evaluate(
    const sqi_window_t *window,
    const g1_integrity_config_t *config,
    g1_integrity_result_t *out_result
);
