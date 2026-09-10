#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "processing/sqi/features/autocorrelation.h"
#include "processing/sqi/features/threshold_crossing.h"
#include "processing/sqi/gates/g2_pulsatility/gate_pulsatility.h"
#include "processing/sqi/preprocess/ppg_preprocess.h"

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

static void make_raw_ppg(ppg_sample_t *out, float bpm, float pulse_amplitude, float drift_amplitude) {
    const float pulse_hz = bpm / 60.0f;
    const float drift_hz = 0.10f;
    for (size_t i = 0u; i < N; ++i) {
        const float t = (float)i / FS;
        const float drift = drift_amplitude * sinf(2.0f * PI_F * drift_hz * t);
        const float pulse = pulse_amplitude * sinf(2.0f * PI_F * pulse_hz * t);
        out[i].timestamp_ms = (uint32_t)(i * 10u);
        out[i].seq = (uint32_t)i;
        out[i].red = (uint32_t)(50000.0f + drift + pulse);
        out[i].ir = (uint32_t)(54000.0f + 0.8f * drift + 1.2f * pulse);
    }
}

static float vector_rms(const float *x, size_t n) {
    double sum = 0.0;
    for (size_t i = 0u; i < n; ++i) {
        sum += (double)x[i] * (double)x[i];
    }
    return (float)sqrt(sum / (double)n);
}

static g2_pulsatility_config_t test_config(void) {
    return (g2_pulsatility_config_t){
        .minimum_ac_rms = 10.0f,
        .minimum_crossings = 4u,
        .maximum_crossings = 100u,
        .minimum_acf_period_s = 0.20f,
        .maximum_acf_period_s = 2.00f,
        .minimum_fzcp_s = 0.05f,
        .maximum_fzcp_s = 1.00f,
        .minimum_acf_peak = 0.50f,
        .minimum_pulse_bpm = 40.0f,
        .maximum_pulse_bpm = 180.0f,
    };
}

int main(void) {
    float red[N];
    float ir[N];
    float scratch[N];

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
    assert(gate_pulsatility_evaluate(red, ir, N, FS, &cfg, scratch, &result));
    assert(result.passed);
    assert(result.failure_mask == 0u);
    assert(result.red_has_fzcp);
    assert(result.ir_has_fzcp);
    assert(result.red_fzcp_s >= 0.05f && result.red_fzcp_s <= 1.0f);
    assert(result.ir_fzcp_s >= 0.05f && result.ir_fzcp_s <= 1.0f);
    assert(result.red_acf_peak > 0.50f);
    assert(result.ir_acf_peak > 0.50f);
    assert(result.red_period_bpm > 73.0f && result.red_period_bpm < 77.0f);
    assert(result.ir_period_bpm > 73.0f && result.ir_period_bpm < 77.0f);

    for (size_t i = 0u; i < N; ++i) {
        red[i] = 0.0f;
        ir[i] = 0.0f;
    }
    assert(gate_pulsatility_evaluate(red, ir, N, FS, &cfg, scratch, &result));
    assert(!result.passed);
    assert((result.failure_mask & G2_FAILURE_LOW_RMS_RED) != 0u);
    assert((result.failure_mask & G2_FAILURE_LOW_RMS_IR) != 0u);
    assert((result.failure_mask & G2_FAILURE_CROSSINGS_RED) != 0u);
    assert((result.failure_mask & G2_FAILURE_CROSSINGS_IR) != 0u);
    assert((result.failure_mask & G2_FAILURE_FZCP_RED) != 0u);
    assert((result.failure_mask & G2_FAILURE_FZCP_IR) != 0u);
    assert((result.failure_mask & G2_FAILURE_LOW_ACF_RED) != 0u);
    assert((result.failure_mask & G2_FAILURE_LOW_ACF_IR) != 0u);

    make_sine(red, 220.0f, 100.0f);
    make_sine(ir, 220.0f, 100.0f);
    assert(gate_pulsatility_evaluate(red, ir, N, FS, &cfg, scratch, &result));
    assert(!result.passed);
    assert((result.failure_mask & G2_FAILURE_PERIOD_RED) != 0u);
    assert((result.failure_mask & G2_FAILURE_PERIOD_IR) != 0u);

    ppg_sample_t raw[N];
    ppg_sample_t raw_copy[N];
    make_raw_ppg(raw, 75.0f, 1200.0f, 1000.0f);
    memcpy(raw_copy, raw, sizeof(raw));

    assert(ppg_preprocess_highpass_butterworth3(raw, N, FS, 0.5f, red, ir));
    assert(memcmp(raw, raw_copy, sizeof(raw)) == 0);
    assert(vector_rms(red, N) > 100.0f);
    assert(vector_rms(ir, N) > vector_rms(red, N));

    crossings = 0u;
    assert(threshold_crossing_count(red, N, 0.0f, &crossings));
    assert(crossings >= 10u && crossings <= 14u);
    assert(autocorrelation_best_lag(red, N, 33u, 150u, &best_lag, &best_acf));
    assert(best_lag >= 78u && best_lag <= 82u);
    assert(best_acf > 0.90f);

    assert(gate_pulsatility_evaluate(red, ir, N, FS, &cfg, scratch, &result));
    assert(result.passed);
    assert(result.red_period_bpm > 73.0f && result.red_period_bpm < 77.0f);
    assert(result.ir_period_bpm > 73.0f && result.ir_period_bpm < 77.0f);

    for (size_t i = 0u; i < N; ++i) {
        raw[i].timestamp_ms = (uint32_t)(i * 10u);
        raw[i].seq = (uint32_t)i;
        raw[i].red = 50000u;
        raw[i].ir = 54000u;
    }
    assert(ppg_preprocess_highpass_butterworth3(raw, N, FS, 0.5f, red, ir));
    assert(vector_rms(red, N) < 1e-3f);
    assert(vector_rms(ir, N) < 1e-3f);

    assert(!ppg_preprocess_highpass_butterworth3(NULL, N, FS, 0.5f, red, ir));
    assert(!ppg_preprocess_highpass_butterworth3(raw, 2u, FS, 0.5f, red, ir));
    assert(!ppg_preprocess_highpass_butterworth3(raw, N, FS, 0.0f, red, ir));
    assert(!ppg_preprocess_highpass_butterworth3(raw, N, FS, 50.0f, red, ir));

    puts("G2 pulsatility foundation + preprocess tests: PASS");
    return 0;
}
