/*
É uma estrutura de buffer circular com tamanho de 128 definida pelo SAMPLE_BUFFER_SIZE definido na interface .h
Estrutura: 
    sample_buffer_t
    ├── data  → vetor onde ficam as amostras
    ├── head  → posição onde será escrita a próxima amostra
    └── count → quantas amostras válidas existem

Funções:
    sample_buffer_init()
        esvazia logicamente o buffer

    sample_buffer_push()
        insere uma nova amostra
        substitui a mais antiga se estiver cheio

    sample_buffer_count()
        informa quantas amostras válidas existem

    sample_buffer_latest()
        retorna a amostra mais recente

    sample_buffer_get_oldest_first()
        acessa as amostras em ordem cronológica
*/

#include "sensing/sample_buffer.h"

void sample_buffer_init(sample_buffer_t *buffer) {
    if (buffer == NULL) {
        return;
    }

    buffer->head = 0;
    buffer->count = 0;
}

bool sample_buffer_push(sample_buffer_t *buffer, const ppg_sample_t *sample) {
    if (buffer == NULL || sample == NULL) {
        return false;
    }

    buffer->data[buffer->head] = *sample;
    buffer->head = (buffer->head + 1) % SAMPLE_BUFFER_SIZE;

    if (buffer->count < SAMPLE_BUFFER_SIZE) {
        buffer->count++;
    }

    return true;
}

size_t sample_buffer_count(const sample_buffer_t *buffer) {
    return buffer ? buffer->count : 0;
}

bool sample_buffer_latest(const sample_buffer_t *buffer, ppg_sample_t *sample) {
    if (buffer == NULL || sample == NULL || buffer->count == 0) {
        return false;
    }

    size_t last = (buffer->head + SAMPLE_BUFFER_SIZE - 1) % SAMPLE_BUFFER_SIZE;
    *sample = buffer->data[last];
    return true;
}

bool sample_buffer_get_oldest_first(const sample_buffer_t *buffer, size_t index, ppg_sample_t *sample) {
    if (buffer == NULL || sample == NULL || index >= buffer->count) {
        return false;
    }

    size_t oldest = (buffer->head + SAMPLE_BUFFER_SIZE - buffer->count) % SAMPLE_BUFFER_SIZE;
    size_t pos = (oldest + index) % SAMPLE_BUFFER_SIZE;
    *sample = buffer->data[pos];
    return true;
}