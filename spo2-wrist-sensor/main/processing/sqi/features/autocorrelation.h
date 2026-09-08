#pragma once

#include <stdbool.h>
#include <stddef.h>

/*
 * Busca o melhor lag de autocorrelação normalizada em uma faixa inclusiva.
 * A entrada deve ser uma cópia pré-processada do PPG (baseline removida).
 * Não altera o sinal de entrada.
 */
bool autocorrelation_best_lag(
    const float *samples,
    size_t count,
    size_t min_lag,
    size_t max_lag,
    size_t *out_best_lag,
    float *out_best_correlation
);
