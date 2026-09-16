#pragma once

#include <stdbool.h>

#include "common/measurement_types.h"
#include "sensing/sample_buffer.h"
#include "storage/config_repo.h"

bool spo2_estimator_compute(
    const sample_buffer_t *buffer,
    const signal_quality_t *quality,
    spo2_result_t *out_spo2
);

bool spo2_estimator_compute_with_calibration(
    const sample_buffer_t *buffer,
    const signal_quality_t *quality,
    const spo2_calibration_t *calibration,
    spo2_result_t *out_spo2
);
