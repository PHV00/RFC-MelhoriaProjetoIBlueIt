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

bool confidence_engine_build_frame(
    const signal_quality_t *quality,
    const hr_result_t *hr,
    const spo2_result_t *spo2,
    health_frame_t *out_frame
) {
    if (quality == NULL || hr == NULL || spo2 == NULL || out_frame == NULL) {
        return false;
    }

    out_frame->finger_present = quality->signal_present;
    out_frame->confidence = quality->quality_score;
    out_frame->quality = *quality;
    out_frame->hr = *hr;
    out_frame->spo2 = *spo2;

    out_frame->valid =
        quality->signal_present &&
        (quality->quality_score >= 0.35f);

    return true;
}