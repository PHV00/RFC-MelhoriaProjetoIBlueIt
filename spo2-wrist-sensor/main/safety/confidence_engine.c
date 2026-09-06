/*
O confidence_engine pega os resultados já calculados e monta um quadro final de saúde, decidindo se aquele conjunto de dados pode ser considerado confiável.

signal_quality_t
hr_result_t
spo2_result_t
        ↓
(estamos aqui)confidence_engine
        ↓
health_frame_t

retorno false
    a função falhou porque recebeu argumentos inválidos

out_frame->valid = false
    a função funcionou, mas os dados fisiológicos não são confiáveis

pontos a implementar: 
    +qualidade do sinal
    + presença do dedo
    + índice de perfusão
    + ruído
    + estabilidade do HR
    + validade da SpO₂
    + estabilidade temporal
            ↓
    confidence score

float confidence = 0.0f;

confidence += 0.35f * quality->quality_score;
confidence += 0.20f * perfusion_score;
confidence += 0.20f * noise_score;
confidence += 0.15f * hr_stability_score;
confidence += 0.10f * spo2_stability_score;

*/
#include "safety/confidence_engine.h"

#include <string.h>

#include "storage/config_repo.h"

static float clamp01(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

bool confidence_engine_build_frame_ex(
    const signal_quality_t *quality,
    const hr_result_t *hr,
    const spo2_result_t *spo2,
    uint32_t timestamp_ms,
    uint32_t fifo_overflow_events,
    float minimum_quality_score,
    health_frame_t *out_frame
) {
    if (quality == NULL || hr == NULL || spo2 == NULL || out_frame == NULL) return false;
    memset(out_frame, 0, sizeof(*out_frame));

    out_frame->timestamp_ms = timestamp_ms;
    out_frame->finger_present = quality->signal_present;
    out_frame->fifo_overflow_events = fifo_overflow_events;
    out_frame->quality = *quality;
    out_frame->hr = *hr;
    out_frame->spo2 = *spo2;
    out_frame->invalid_reasons = quality->invalid_reasons;

    if (fifo_overflow_events > 0u) out_frame->invalid_reasons |= PPG_INVALID_FIFO_OVERFLOW;
    if (!spo2->calibrated) out_frame->invalid_reasons |= PPG_INVALID_CALIBRATION;
    if (spo2->status == ESTIMATOR_OUT_OF_RANGE || hr->status == ESTIMATOR_OUT_OF_RANGE) {
        out_frame->invalid_reasons |= PPG_INVALID_DOMAIN;
    }

    float confidence = 0.50f * quality->quality_score +
                       0.25f * (hr->valid ? hr->confidence : 0.0f) +
                       0.25f * (spo2->valid ? spo2->confidence : 0.0f);
    out_frame->confidence = clamp01(confidence);

    uint32_t critical_quality_reasons = PPG_INVALID_NO_SIGNAL |
                                        PPG_INVALID_WINDOW_SHORT |
                                        PPG_INVALID_DISCONTINUITY |
                                        PPG_INVALID_CLIPPING;
    out_frame->valid = quality->signal_present &&
                       quality->quality_score >= minimum_quality_score &&
                       hr->valid && spo2->valid &&
                       (quality->invalid_reasons & critical_quality_reasons) == 0u;
    out_frame->clinical_valid = out_frame->valid && spo2->calibrated;
    return true;
}

bool confidence_engine_build_frame(
    const signal_quality_t *quality,
    const hr_result_t *hr,
    const spo2_result_t *spo2,
    health_frame_t *out_frame
) {
    const system_config_t *cfg = config_repo_get();
    return confidence_engine_build_frame_ex(quality, hr, spo2, 0u, 0u,
                                            cfg->sqi.minimum_quality_score, out_frame);
}
