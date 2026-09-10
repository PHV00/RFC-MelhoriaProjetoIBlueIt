#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include "processing/sqi/features/autocorrelation.h"

#define N 500u
#define PI_F 3.14159265358979323846f

static float reference_corr_at_lag(const float *samples, size_t count, size_t lag) {
    double numerator = 0.0;
    double energy_a = 0.0;
    double energy_b = 0.0;
    const size_t paired_count = count - lag;

    for (size_t i = 0u; i < paired_count; ++i) {
        const double a = (double)samples[i];
        const double b = (double)samples[i + lag];
        numerator += a * b;
        energy_a += a * a;
        energy_b += b * b;
    }

    const double denominator = sqrt(energy_a * energy_b);
    return denominator > 1e-12 ? (float)(numerator / denominator) : 0.0f;
}

static void reference_best_lag(
    const float *samples,
    size_t count,
    size_t min_lag,
    size_t max_lag,
    size_t *out_lag,
    float *out_corr
) {
    size_t best_lag = min_lag;
    float best_corr = -1.0f;
    for (size_t lag = min_lag; lag <= max_lag; ++lag) {
        const float corr = reference_corr_at_lag(samples, count, lag);
        if (corr > best_corr) {
            best_corr = corr;
            best_lag = lag;
        }
    }
    *out_lag = best_lag;
    *out_corr = best_corr;
}

static void make_mixed_signal(float *out) {
    for (size_t i = 0u; i < N; ++i) {
        const float t = (float)i / 100.0f;
        const float pulse = 900.0f * sinf(2.0f * PI_F * 1.25f * t);
        const float harmonic = 180.0f * sinf(2.0f * PI_F * 2.50f * t + 0.3f);
        const float ripple = 55.0f * sinf(2.0f * PI_F * 7.0f * t + 0.7f);
        out[i] = pulse + harmonic + ripple;
    }
}

static void make_deterministic_noise(float *out) {
    unsigned state = 0x12345678u;
    for (size_t i = 0u; i < N; ++i) {
        state = 1664525u * state + 1013904223u;
        const int centered = (int)((state >> 16) & 0x7fffu) - 16384;
        out[i] = (float)centered / 16.0f;
    }
}

static void compare_with_reference(const float *signal, size_t min_lag, size_t max_lag) {
    size_t ref_lag = 0u;
    float ref_corr = 0.0f;
    reference_best_lag(signal, N, min_lag, max_lag, &ref_lag, &ref_corr);

    size_t opt_lag = 0u;
    float opt_corr = 0.0f;
    assert(autocorrelation_best_lag(signal, N, min_lag, max_lag, &opt_lag, &opt_corr));

    const size_t lag_delta = ref_lag > opt_lag ? ref_lag - opt_lag : opt_lag - ref_lag;
    assert(lag_delta <= 1u);
    assert(fabsf(ref_corr - opt_corr) < 0.0025f);
}

int main(void) {
    float signal[N];

    make_mixed_signal(signal);
    compare_with_reference(signal, 33u, 150u);

    make_deterministic_noise(signal);
    compare_with_reference(signal, 33u, 150u);

    puts("G2 autocorrelation optimized/reference equivalence: PASS");
    return 0;
}
