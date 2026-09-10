#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "processing/sqi/preprocess/ppg_preprocess.h"

#define FS 100.0f
#define N 5000u
#define DISCARD 1000u
#define PI_F 3.14159265358979323846f
#define DC_RED 100000.0f
#define DC_IR 110000.0f
#define AMPLITUDE 10000.0f

static ppg_sample_t raw[N];
static float red[N];
static float ir[N];

static void make_tone(float frequency_hz) {
    for (size_t i = 0u; i < N; ++i) {
        const float t = (float)i / FS;
        const float tone = AMPLITUDE * sinf(2.0f * PI_F * frequency_hz * t);

        raw[i].timestamp_ms = (uint32_t)(i * 10u);
        raw[i].seq = (uint32_t)i;
        raw[i].red = (uint32_t)(DC_RED + tone);
        raw[i].ir = (uint32_t)(DC_IR + tone);
    }
}

static float tail_rms(const float *samples) {
    double sum_sq = 0.0;
    const size_t count = N - DISCARD;

    for (size_t i = DISCARD; i < N; ++i) {
        sum_sq += (double)samples[i] * (double)samples[i];
    }

    return (float)sqrt(sum_sq / (double)count);
}

static float measured_gain(float frequency_hz) {
    make_tone(frequency_hz);
    assert(ppg_preprocess_highpass_butterworth3(raw, N, FS, 0.5f, red, ir));

    const float input_rms = AMPLITUDE / sqrtf(2.0f);
    const float red_gain = tail_rms(red) / input_rms;
    const float ir_gain = tail_rms(ir) / input_rms;

    /* Os dois canais passam pelo mesmo filtro e devem apresentar o mesmo ganho. */
    assert(fabsf(red_gain - ir_gain) < 0.002f);
    return red_gain;
}

int main(void) {
    const float gain_010 = measured_gain(0.10f);
    const float gain_050 = measured_gain(0.50f);
    const float gain_125 = measured_gain(1.25f);
    const float gain_200 = measured_gain(2.00f);

    /*
     * Butterworth HP de 3ª ordem, fc=0,5 Hz:
     * - 0,10 Hz deve ser fortemente atenuado (~0,008 / -42 dB);
     * - em fc o ganho esperado é ~1/sqrt(2) = 0,707 (-3 dB);
     * - frequências pulsáteis acima de fc devem ser praticamente preservadas.
     *
     * As faixas abaixo são tolerâncias de regressão numérica, não thresholds SQI.
     */
    assert(gain_010 > 0.0f && gain_010 < 0.020f);
    assert(gain_050 > 0.68f && gain_050 < 0.74f);
    assert(gain_125 > 0.97f && gain_125 < 1.03f);
    assert(gain_200 > 0.98f && gain_200 < 1.02f);

    printf(
        "G2 preprocess HP3 gains: 0.10Hz=%.4f 0.50Hz=%.4f 1.25Hz=%.4f 2.00Hz=%.4f\n",
        gain_010, gain_050, gain_125, gain_200
    );
    puts("G2 preprocess frequency-response tests: PASS");
    return 0;
}
