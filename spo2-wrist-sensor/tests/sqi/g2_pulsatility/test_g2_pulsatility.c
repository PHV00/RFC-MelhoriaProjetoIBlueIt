#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "processing/sqi/features/autocorrelation.h"
#include "processing/sqi/features/threshold_crossing.h"
#include "processing/sqi/gates/g2_pulsatility/gate_pulsatility.h"

#define FS 100.0f
#define N 500u
#define PI_F 3.14159265358979323846f

static void make_sine(float *out, float bpm, float amplitude) {
    const float hz = bpm / 60.0f;
    for (size_t i = 0u; i < N; ++i) {
        const float t = (float)i / FS;
        out[i] = amplitude * sinf(2.0f * PI_F * hz * t);
    }
}

static g2_pulsatility_config_t test_config(void) {
    return (g2_pulsatility_config_t){
        .minimum_ac_rms = 10.0f,
        .minimum_crossings = 4u,
        .maximum_crossings = 30u,
        .minimum_acf_peak = 0.70f,
        .minimum_pulse_bpm = 40.0f,
        .maximum_pulse_bpm = 180.0f,
    };
}

int main(void) {
    float red[N];
    float ir[N];

    make_sine(red, 75.0f, 100.0f);
    make_sine(ir, 75.0f, 120.0f);

    uint32_t crossings = 0u;
    assert(threshold_crossing_count(red, N, 0.0f, &crossings));
    assert(crossings >= 10u && crossings <= 14u);

    size_t best_lag = 0u;
    float best_acf = 0.0f;
    assert(autocorrelation_best_lag(red, N, 33u, 150u, &best_lag, &best_acf));
    assert(best_lag >= 78u && best_lag <= 82u);
    assert(best_acf > 0.95f);

    g2_pulsatility_result_t result = {0};
    const g2_pulsatility_config_t cfg = test_config();
    assert(gate_pulsatility_evaluate(red, ir, N, FS, &cfg, &result));
    assert(result.passed);
    assert(result.failure_mask == 0u);
    assert(result.red_period_bpm > 73.0f && result.red_period_bpm < 77.0f);
    assert(result.ir_period_bpm > 73.0f && result.ir_period_bpm < 77.0f);

    for (size_t i = 0u; i < N; ++i) {
        red[i] = 0.0f;
        ir[i] = 0.0f;
    }
    assert(gate_pulsatility_evaluate(red, ir, N, FS, &cfg, &result));
    assert(!result.passed);
    assert((result.failure_mask & G2_FAILURE_LOW_RMS_RED) != 0u);
    assert((result.failure_mask & G2_FAILURE_LOW_RMS_IR) != 0u);
    assert((result.failure_mask & G2_FAILURE_CROSSINGS_RED) != 0u);
    assert((result.failure_mask & G2_FAILURE_CROSSINGS_IR) != 0u);
    assert((result.failure_mask & G2_FAILURE_LOW_ACF_RED) != 0u);
    assert((result.failure_mask & G2_FAILURE_LOW_ACF_IR) != 0u);

    make_sine(red, 220.0f, 100.0f);
    make_sine(ir, 220.0f, 100.0f);
    assert(gate_pulsatility_evaluate(red, ir, N, FS, &cfg, &result));
    assert(!result.passed);

    puts("G2 pulsatility foundation tests: PASS");
    return 0;
}
