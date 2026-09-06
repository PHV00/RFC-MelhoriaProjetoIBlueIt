#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common/measurement_types.h"

bool confidence_engine_build_frame(
    const signal_quality_t *quality,
    const hr_result_t *hr,
    const spo2_result_t *spo2,
    health_frame_t *out_frame
);

bool confidence_engine_build_frame_ex(
    const signal_quality_t *quality,
    const hr_result_t *hr,
    const spo2_result_t *spo2,
    uint32_t timestamp_ms,
    uint32_t fifo_overflow_events,
    float minimum_quality_score,
    health_frame_t *out_frame
);
