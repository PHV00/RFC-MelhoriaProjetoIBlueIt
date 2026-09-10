#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "common/sqi_types.h"

/*
 * Avalia pulsatilidade em sinais RED/IR JÁ PRÉ-PROCESSADOS.
 *
 * O G2 não recebe RAW diretamente e não altera os vetores de entrada.
 * `scratch` deve possuir pelo menos `count` floats e é reutilizado
 * sequencialmente pelos dois canais para a janela de Hamming/ACF.
 *
 * Os thresholds são fornecidos pelo chamador. A estrutura de decisão de ACF
 * segue FZCP + Rmax + Kmax; a calibração final continua dependente do perfil
 * MAX30102/NO_GRIP.
 */
bool gate_pulsatility_evaluate(
    const float *red_processed,
    const float *ir_processed,
    size_t count,
    float sample_rate_hz,
    const g2_pulsatility_config_t *config,
    float *scratch,
    g2_pulsatility_result_t *out_result
);
