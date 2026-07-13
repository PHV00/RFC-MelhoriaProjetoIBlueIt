//O sampler é uma estrutura que reúne os elementos necessários para a coleta.
/*
O sampler é uma estrutura que reúne os elementos necessários para a coleta.

ppg_sampler_t
├── sensor → de onde ler
├── buffer → onde guardar
└── seq    → número da próxima amostra
*/

#include "sensing/ppg_sampler.h"

#include "esp_timer.h"

void ppg_sampler_init(ppg_sampler_t *sampler, max3010x_t *sensor, sample_buffer_t *buffer) {
    if (sampler == NULL) {
        return;
    }

    sampler->sensor = sensor;
    sampler->buffer = buffer;
    sampler->seq = 0;
}

bool ppg_sampler_step(ppg_sampler_t *sampler) {
    if (sampler == NULL || sampler->sensor == NULL || sampler->buffer == NULL) {
        return false;
    }

    ppg_sample_t sample = {0};

    if (max3010x_read_sample(sampler->sensor, &sample) != ESP_OK) {
        return false;
    }

    sample.timestamp_ms = (uint32_t)(   esp_timer_get_time() / 1000ULL);
    sample.seq = ++sampler->seq;

    return sample_buffer_push(sampler->buffer, &sample);
}