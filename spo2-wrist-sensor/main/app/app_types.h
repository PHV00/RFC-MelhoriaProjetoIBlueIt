
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
.typedef enum {
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

typedef struct {
    bool signal_present;
    float dc_ir;
    float dc_red;
    float ac_ir;
    float ac_red;
    float noise;
    float perfusion_index;
    float quality_score;
} signal_quality_t;

typedef struct {
    bool valid;
    float bpm;
} hr_result_t;

typedef struct {
    bool valid;
    float spo2;
} spo2_result_t;

typedef struct {
    bool valid;
    bool finger_present;
    float confidence;
    hr_result_t hr;
    spo2_result_t spo2;
    signal_quality_t quality;
} health_frame_t;