#include "processing/spo2_estimator.h"

#include <math.h>
#include <string.h>

static float clamp01(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

bool spo2_estimator_compute_with_calibration(
    const sample_buffer_t *buffer,
    const signal_quality_t *quality,
    const spo2_calibration_t *calibration,
    spo2_result_t *out_spo2
) {
    if (out_spo2 == NULL) return false;
    memset(out_spo2, 0, sizeof(*out_spo2));
    out_spo2->status = ESTIMATOR_INVALID_ARGUMENT;

    if (buffer == NULL || quality == NULL || calibration == NULL) return false;
    if (!quality->signal_present) {
        out_spo2->status = ESTIMATOR_NO_SIGNAL;
        return false;
    }
    if (quality->quality_score < 0.50f || fabsf(quality->red_ir_correlation) < 0.60f) {
        out_spo2->status = ESTIMATOR_LOW_QUALITY;
        return false;
    }
    if (quality->dc_red <= 1e-6f || quality->dc_ir <= 1e-6f ||
        quality->ac_red <= 1e-6f || quality->ac_ir <= 1e-6f) {
        out_spo2->status = ESTIMATOR_NO_SIGNAL;
        return false;
    }

    float red_norm = quality->ac_red / quality->dc_red;
    float ir_norm = quality->ac_ir / quality->dc_ir;
    if (!isfinite(red_norm) || !isfinite(ir_norm) || ir_norm <= 1e-8f) {
        out_spo2->status = ESTIMATOR_INTERNAL_ERROR;
        return false;
    }

    float ratio = red_norm / ir_norm;
    out_spo2->ratio_r = ratio;
    out_spo2->calibrated = calibration->calibrated;
    out_spo2->calibration_version = calibration->version;

    if (!isfinite(ratio) || ratio < calibration->min_ratio_r || ratio > calibration->max_ratio_r) {
        out_spo2->status = ESTIMATOR_OUT_OF_RANGE;
        return false;
    }
    if (!calibration->calibrated && !calibration->allow_uncalibrated_estimate) {
        out_spo2->status = ESTIMATOR_NOT_READY;
        return false;
    }

    float spo2 = calibration->a * ratio * ratio + calibration->b * ratio + calibration->c;
    if (!isfinite(spo2) || spo2 < 70.0f || spo2 > 100.5f) {
        out_spo2->status = ESTIMATOR_OUT_OF_RANGE;
        return false;
    }
    if (spo2 > 100.0f) spo2 = 100.0f;

    float center = 0.5f * (calibration->min_ratio_r + calibration->max_ratio_r);
    float half_range = 0.5f * (calibration->max_ratio_r - calibration->min_ratio_r);
    float domain_margin = half_range > 1e-6f ? clamp01(1.0f - fabsf(ratio - center) / half_range) : 0.0f;

    out_spo2->valid = true;
    out_spo2->status = calibration->calibrated ? ESTIMATOR_OK : ESTIMATOR_OK_UNCALIBRATED;
    out_spo2->spo2 = spo2;
    out_spo2->confidence = quality->quality_score * (0.65f + 0.35f * domain_margin) *
                           (calibration->calibrated ? 1.0f : 0.60f);
    return true;
}

bool spo2_estimator_compute(const sample_buffer_t *buffer, const signal_quality_t *quality, spo2_result_t *out_spo2) {
    const system_config_t *cfg = config_repo_get();
    return spo2_estimator_compute_with_calibration(buffer, quality, &cfg->spo2_calibration, out_spo2);
}
