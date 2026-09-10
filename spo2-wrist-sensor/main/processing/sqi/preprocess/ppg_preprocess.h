#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "common/measurement_types.h"

/*
 * Pré-processamento dedicado ao G2.
 *
 * Aplica um Butterworth high-pass de 3ª ordem como cascata de uma seção
 * de 1ª ordem e uma seção biquad de 2ª ordem (Q = 1), após centralização
 * da janela para reduzir o transiente causado pelo grande nível DC do PPG.
 *
 * O vetor RAW de entrada nunca é alterado. RED e IR são processados em
 * buffers de saída independentes.
 */
bool ppg_preprocess_highpass_butterworth3(
    const ppg_sample_t *samples,
    size_t count,
    float sample_rate_hz,
    float cutoff_hz,
    float *out_red,
    float *out_ir
);
