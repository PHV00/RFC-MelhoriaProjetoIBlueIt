#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    bool has_first_zero_crossing;
    size_t first_zero_crossing_lag;
    float first_zero_crossing_s;
    size_t peak_lag;
    float peak_period_s;
    float peak_correlation;
} autocorrelation_ppg_shape_t;

/*
 * Busca o melhor lag de autocorrelação normalizada em uma faixa inclusiva.
 * Esta API é mantida para os testes e para a implementação inicial do G2.
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

/*
 * Extrai as features de ACF descritas por Vadrevu & Manikandan (2019):
 *   - aplica janela de Hamming ao segmento;
 *   - usa R[k] = sum(s[n]s[n+k]) / sum(s[n]^2);
 *   - encontra o primeiro cruzamento por zero da ACF (FZCP);
 *   - encontra o maior valor de ACF na faixa de período candidata.
 *
 * scratch_windowed deve possuir pelo menos `count` floats. A entrada original
 * permanece imutável. Esta API é inicialmente validada no host antes de
 * substituir a ACF simplificada atualmente ligada ao gate de produção.
 */
bool autocorrelation_ppg_shape_extract(
    const float *samples,
    size_t count,
    float sample_rate_hz,
    float minimum_peak_period_s,
    float maximum_peak_period_s,
    float *scratch_windowed,
    autocorrelation_ppg_shape_t *out_shape
);
