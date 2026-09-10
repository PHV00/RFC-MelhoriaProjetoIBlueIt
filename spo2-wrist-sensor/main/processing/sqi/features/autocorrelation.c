#include "processing/sqi/features/autocorrelation.h"

#include <math.h>

bool autocorrelation_best_lag(
    const float *samples,
    size_t count,
    size_t min_lag,
    size_t max_lag,
    size_t *out_best_lag,
    float *out_best_correlation
) {
    if (samples == NULL || out_best_lag == NULL || out_best_correlation == NULL ||
        count < 3u || min_lag == 0u || min_lag > max_lag || max_lag >= count) {
        return false;
    }

    /*
     * A versão inicial recalculava, para cada lag, numerador + duas energias
     * usando double. No ESP32-C3 isso torna o custo do G2 incompatível com o
     * orçamento da FIFO do MAX30102.
     *
     * Aqui mantemos a mesma definição de autocorrelação normalizada, porém:
     *   - o numerador continua O(N) por lag (parte inevitável da busca direta);
     *   - as energias das duas janelas são calculadas uma vez e atualizadas em
     *     O(1) quando o lag avança;
     *   - todo o hot path usa float/sqrtf, suficiente para a decisão de SQI.
     */
    float energy_a = 0.0f;
    float energy_b = 0.0f;

    const size_t initial_pairs = count - min_lag;
    for (size_t i = 0u; i < initial_pairs; ++i) {
        const float x = samples[i];
        energy_a += x * x;
    }
    for (size_t i = min_lag; i < count; ++i) {
        const float x = samples[i];
        energy_b += x * x;
    }

    size_t best_lag = min_lag;
    float best_correlation = -1.0f;

    for (size_t lag = min_lag; lag <= max_lag; ++lag) {
        if (lag > min_lag) {
            const size_t removed_from_a = count - lag;
            const size_t removed_from_b = lag - 1u;
            const float xa = samples[removed_from_a];
            const float xb = samples[removed_from_b];
            energy_a -= xa * xa;
            energy_b -= xb * xb;

            /* Protege apenas contra pequeno erro de arredondamento acumulado. */
            if (energy_a < 0.0f) energy_a = 0.0f;
            if (energy_b < 0.0f) energy_b = 0.0f;
        }

        const size_t paired_count = count - lag;
        float numerator = 0.0f;
        for (size_t i = 0u; i < paired_count; ++i) {
            numerator += samples[i] * samples[i + lag];
        }

        const float denominator = sqrtf(energy_a * energy_b);
        const float correlation = denominator > 1e-12f
            ? numerator / denominator
            : 0.0f;

        if (correlation > best_correlation) {
            best_correlation = correlation;
            best_lag = lag;
        }
    }

    *out_best_lag = best_lag;
    *out_best_correlation = best_correlation;
    return true;
}
