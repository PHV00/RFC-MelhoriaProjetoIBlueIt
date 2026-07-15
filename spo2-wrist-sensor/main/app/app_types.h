
/*
    arquivo responsavel por criar um dicionário de tipos de dados compartilhados pelo projeto

    responsavel por fornecer os tipos de dados que o sistema usa para representar:
    -estado, 
    -amostra PPG,
    -qualidade,
    -frequência cardíaca,
    -SpO₂, 
    -resultado completo.

    padronizando os dados transmitidos pelos modulos do software, que se comunicam pela mesma estruturas aqui setadas

*/

#pragma once
/*
#pragma once

Inclua este arquivo apenas uma vez durante a compilação de cada arquivo .c.

"cria singleton"

*/

#include <stdbool.h>
#include <stdint.h>

// Um enum = um tipo que representa um conjunto fechado de opções possíveis(lista de opções)
typedef enum {
    APP_STATE_BOOT = 0,
    APP_STATE_SELF_TEST,
    APP_STATE_IDLE,
    APP_STATE_SAMPLING,
    APP_STATE_TRACKING,
    APP_STATE_LOW_CONFIDENCE,
    APP_STATE_ERROR
} app_state_t;

typedef struct {
    uint32_t timestamp_ms;
    uint32_t seq;
    uint32_t ir;
    uint32_t red;
} ppg_sample_t;

typedef enum {
    ESTIMATOR_OK = 0,
    ESTIMATOR_OK_UNCALIBRATED,
    ESTIMATOR_NOT_READY,
    ESTIMATOR_LOW_QUALITY,
    ESTIMATOR_NO_SIGNAL,
    ESTIMATOR_OUT_OF_RANGE,
    ESTIMATOR_INVALID_ARGUMENT,
    ESTIMATOR_INTERNAL_ERROR
} estimator_status_t;

typedef enum {
    PPG_INVALID_NONE              = 0,
    PPG_INVALID_NO_SIGNAL         = 1u << 0,
    PPG_INVALID_WINDOW_SHORT      = 1u << 1,
    PPG_INVALID_DISCONTINUITY     = 1u << 2,
    PPG_INVALID_CLIPPING          = 1u << 3,
    PPG_INVALID_LOW_PERFUSION     = 1u << 4,
    PPG_INVALID_LOW_CORRELATION   = 1u << 5,
    PPG_INVALID_FIFO_OVERFLOW     = 1u << 6,
    PPG_INVALID_CALIBRATION       = 1u << 7,
    PPG_INVALID_DOMAIN            = 1u << 8
} ppg_invalid_reason_t;

typedef struct {
    bool signal_present;
    uint32_t invalid_reasons;
    size_t window_samples;
    float sample_rate_hz;
    float window_duration_s;
    float dc_ir;
    float dc_red;
    float ac_ir;                 /* RMS após remoção de tendência. */
    float ac_red;                /* RMS após remoção de tendência. */
    float noise;                 /* RMS residual após suavização. */
    float snr;
    float perfusion_index;
    float red_ir_correlation;
    float continuity_score;
    float clipping_fraction;
    float quality_score;
} signal_quality_t;

typedef struct {
    bool valid;
    estimator_status_t status;
    float bpm;
    float ibi_ms;
    float confidence;
    uint16_t peak_count;
} hr_result_t;

typedef struct {
    bool valid;                  /* Resultado numérico utilizável na POC. */
    bool calibrated;             /* Só é true com curva validada para o conjunto óptico final. */
    estimator_status_t status;
    float spo2;
    float ratio_r;
    float confidence;
    uint32_t calibration_version;
} spo2_result_t;

typedef struct {
    bool valid;                  /* HR e SpO2 numéricos válidos + sinal aceitável. */
    bool clinical_valid;         /* Exige calibração/validação do sistema final. */
    bool finger_present;
    uint32_t timestamp_ms;
    uint32_t invalid_reasons;
    uint32_t fifo_overflow_events;
    float confidence;
    hr_result_t hr;
    spo2_result_t spo2;
    signal_quality_t quality;
} health_frame_t;
