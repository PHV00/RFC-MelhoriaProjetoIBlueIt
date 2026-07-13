// Responsavel por analisar a janela de sinal PPG que está no buffer e avaliar de se a janela parece boa o suficiente para ser usada
#include "processing/signal_quality.h"


/*

lista de problemas :
    AC por máximo menos mínimo: max - min é sensível a outliers.
    Noise mal definido: ac_red / dc_ir não é uma estimativa de ruido
    Limiares arbitrários: precisam de fundamentação e calibração.
        dc_ir > 1000
        ac_ir > 50
        PI × 40
        noise × 10
    Usa todas as amostras disponíveis: Se o buffer estiver parcialmente cheio ou cheio, o tamanho da janela pode variar.

*/


//garanti que as notas não fiquem negativas nem maiores que 1
static float clamp01(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

bool signal_quality_evaluate(const sample_buffer_t *buffer, signal_quality_t *out_quality) {
    if (buffer == NULL || out_quality == NULL) {
        return false;
    }

    //definição do numero minimo de amostras
    size_t count = sample_buffer_count(buffer);
    if (count < 20) {
        return false;
    }

    ppg_sample_t sample;
    uint32_t min_ir = 0xFFFFFFFF;
    uint32_t max_ir = 0;
    uint32_t min_red = 0xFFFFFFFF;
    uint32_t max_red = 0;
    double sum_ir = 0.0;
    double sum_red = 0.0;

    for (size_t i = 0; i < count; i++) {
        if (!sample_buffer_get_oldest_first(buffer, i, &sample)) {
            return false;
        }

        if (sample.ir < min_ir) min_ir = sample.ir;
        if (sample.ir > max_ir) max_ir = sample.ir;
        if (sample.red < min_red) min_red = sample.red;
        if (sample.red > max_red) max_red = sample.red;

        sum_ir += sample.ir;
        sum_red += sample.red;
    }

    float dc_ir = (float)(sum_ir / (double)count);
    float dc_red = (float)(sum_red / (double)count);
    float ac_ir = (float)(max_ir - min_ir);
    float ac_red = (float)(max_red - min_red);

    //Índice de perfusão : Quanto maior essa relação, geralmente mais forte está a pulsação em relação ao nível óptico total.
    float perfusion_index = 0.0f;
    if (dc_ir > 0.0f) {
        perfusion_index = ac_ir / dc_ir;
    }

    //Ajustar para utilizar ruido real
    float noise = 0.0f;
    if (dc_ir > 0.0f) {
        noise = ac_red / dc_ir;
    }

    //ajustar presença do sinal: Os valores 1000 e 50 são limiares fixos. Precisam ser calibrados ou justificados.
    bool signal_present = (dc_ir > 1000.0f) && (ac_ir > 50.0f);

    //Esse fator 40.0f também é uma heurística escolhida pelo desenvolvedor.
    float score_from_pi = clamp01(perfusion_index * 40.0f);
    
    //Mas como a definição de noise não é boa, essa nota também fica comprometida.
    float score_from_noise = clamp01(1.0f - (noise * 10.0f));
    
    /*
    A nota final é uma média ponderada:
        60% índice de perfusão
        40% suposto ruído
    */
    float quality_score = 0.6f * score_from_pi + 0.4f * score_from_noise;

    out_quality->signal_present = signal_present;
    out_quality->dc_ir = dc_ir;
    out_quality->dc_red = dc_red;
    out_quality->ac_ir = ac_ir;
    out_quality->ac_red = ac_red;
    out_quality->noise = noise;
    out_quality->perfusion_index = perfusion_index;
    out_quality->quality_score = signal_present ? clamp01(quality_score) : 0.0f;

    return true;
}