#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    G2_FAILURE_NONE             = 0u,
    G2_FAILURE_LOW_RMS_RED      = 1u << 0,
    G2_FAILURE_LOW_RMS_IR       = 1u << 1,
    G2_FAILURE_CROSSINGS_RED    = 1u << 2,
    G2_FAILURE_CROSSINGS_IR     = 1u << 3,
    G2_FAILURE_LOW_ACF_RED      = 1u << 4,
    G2_FAILURE_LOW_ACF_IR       = 1u << 5,
    G2_FAILURE_PERIOD_RED       = 1u << 6,
    G2_FAILURE_PERIOD_IR        = 1u << 7
} g2_pulsatility_failure_t;

typedef struct {
    float minimum_ac_rms;
    uint32_t minimum_crossings;
    uint32_t maximum_crossings;
    float minimum_acf_peak;
    float minimum_pulse_bpm;
    float maximum_pulse_bpm;
} g2_pulsatility_config_t;

typedef struct {
    bool passed;
    uint32_t failure_mask;

    float red_ac_rms;
    uint32_t red_crossings;
    float red_acf_peak;
    size_t red_best_lag;
    float red_period_bpm;

    float ir_ac_rms;
    uint32_t ir_crossings;
    float ir_acf_peak;
    size_t ir_best_lag;
    float ir_period_bpm;
} g2_pulsatility_result_t;

/*
 * Avalia pulsatilidade em sinais RED/IR JÁ PRÉ-PROCESSADOS.
 *
 * O G2 não recebe RAW diretamente e não altera os vetores de entrada.
 * Os thresholds são fornecidos pelo chamador: os valores científicos finais
 * ainda precisam ser calibrados no perfil MAX30102/NO_GRIP.
 */
bool gate_pulsatility_evaluate(
    const float *red_processed,
    const float *ir_processed,
    size_t count,
    float sample_rate_hz,
    const g2_pulsatility_config_t *config,
    g2_pulsatility_result_t *out_result
);
