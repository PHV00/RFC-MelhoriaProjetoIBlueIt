#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "processing/sqi/features/autocorrelation.h"

#define FS 100.0f
#define N 500u
#define PI_F 3.14159265358979323846f

static void make_sine(float *out, float hz) {
    for (size_t i = 0u; i < N; ++i) {
        const float t = (float)i / FS;
        out[i] = sinf(2.0f * PI_F * hz * t);
    }
}

int main(void) {
    float signal[N];
    float scratch[N];
    autocorrelation_ppg_shape_t shape = {0};

    make_sine(signal, 1.25f); /* 75 bpm */
    assert(autocorrelation_ppg_shape_extract(signal, N, FS, 0.2f, 2.0f, scratch, &shape));
    assert(shape.has_first_zero_crossing);
    assert(shape.first_zero_crossing_s > 0.18f && shape.first_zero_crossing_s < 0.24f);
    assert(shape.peak_period_s > 0.77f && shape.peak_period_s < 0.82f);
    assert(shape.peak_correlation > 0.80f);

    make_sine(signal, 10.0f);
    assert(autocorrelation_ppg_shape_extract(signal, N, FS, 0.2f, 2.0f, scratch, &shape));
    assert(shape.has_first_zero_crossing);
    assert(shape.first_zero_crossing_s < 0.05f);

    make_sine(signal, 0.2f);
    assert(autocorrelation_ppg_shape_extract(signal, N, FS, 0.2f, 2.0f, scratch, &shape));
    assert(shape.has_first_zero_crossing);
    assert(shape.first_zero_crossing_s > 1.0f);

    puts("G2 Vadrevu ACF shape tests: PASS");
    return 0;
}
