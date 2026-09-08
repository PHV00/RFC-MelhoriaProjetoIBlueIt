#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Conta cruzamentos de um limiar em um sinal já pré-processado.
 *
 * Amostras exatamente iguais ao limiar são ignoradas na mudança de estado,
 * evitando dupla contagem em platôs. A função não altera o vetor de entrada.
 */
bool threshold_crossing_count(
    const float *samples,
    size_t count,
    float threshold,
    uint32_t *out_crossings
);
