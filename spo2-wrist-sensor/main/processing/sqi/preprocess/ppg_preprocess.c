#include "processing/sqi/preprocess/ppg_preprocess.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static bool args_valid(
    const ppg_sample_t *samples,
    size_t count,
    float sample_rate_hz,
    float cutoff_hz,
    const float *out_red,
    const float *out_ir
) {
    return samples != NULL && count >= 3u && sample_rate_hz > 0.0f &&
           cutoff_hz > 0.0f && cutoff_hz < (0.5f * sample_rate_hz) &&
           out_red != NULL && out_ir != NULL;
}

static float channel_mean(const ppg_sample_t *samples, size_t count, bool red_channel) {
    double sum = 0.0;
    for (size_t i = 0u; i < count; ++i) {
        sum += red_channel ? (double)samples[i].red : (double)samples[i].ir;
    }
    return (float)(sum / (double)count);
}

static void highpass_first_order(
    const ppg_sample_t *samples,
    size_t count,
    bool red_channel,
    float sample_rate_hz,
    float cutoff_hz,
    float mean,
    float *out
) {
    const float k = tanf((float)M_PI * cutoff_hz / sample_rate_hz);
    const float norm = 1.0f / (1.0f + k);
    const float b0 = norm;
    const float b1 = -norm;
    const float a1 = (k - 1.0f) * norm;

    float previous_x = (red_channel ? (float)samples[0].red : (float)samples[0].ir) - mean;
    float previous_y = 0.0f;

    for (size_t i = 0u; i < count; ++i) {
        const float x = (red_channel ? (float)samples[i].red : (float)samples[i].ir) - mean;
        const float y = b0 * x + b1 * previous_x - a1 * previous_y;
        out[i] = y;
        previous_x = x;
        previous_y = y;
    }
}

static void highpass_second_order_q1_in_place(
    float *samples,
    size_t count,
    float sample_rate_hz,
    float cutoff_hz
) {
    const float omega = 2.0f * (float)M_PI * cutoff_hz / sample_rate_hz;
    const float cos_omega = cosf(omega);
    const float sin_omega = sinf(omega);
    const float alpha = 0.5f * sin_omega; /* Q = 1 */
    const float a0 = 1.0f + alpha;

    const float b0 = ((1.0f + cos_omega) * 0.5f) / a0;
    const float b1 = (-(1.0f + cos_omega)) / a0;
    const float b2 = ((1.0f + cos_omega) * 0.5f) / a0;
    const float a1 = (-2.0f * cos_omega) / a0;
    const float a2 = (1.0f - alpha) / a0;

    float x1 = samples[0];
    float x2 = samples[0];
    float y1 = 0.0f;
    float y2 = 0.0f;

    for (size_t i = 0u; i < count; ++i) {
        const float x = samples[i];
        const float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        samples[i] = y;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
    }
}

bool ppg_preprocess_highpass_butterworth3(
    const ppg_sample_t *samples,
    size_t count,
    float sample_rate_hz,
    float cutoff_hz,
    float *out_red,
    float *out_ir
) {
    if (!args_valid(samples, count, sample_rate_hz, cutoff_hz, out_red, out_ir)) {
        return false;
    }

    const float red_mean = channel_mean(samples, count, true);
    const float ir_mean = channel_mean(samples, count, false);

    highpass_first_order(samples, count, true, sample_rate_hz, cutoff_hz, red_mean, out_red);
    highpass_first_order(samples, count, false, sample_rate_hz, cutoff_hz, ir_mean, out_ir);

    highpass_second_order_q1_in_place(out_red, count, sample_rate_hz, cutoff_hz);
    highpass_second_order_q1_in_place(out_ir, count, sample_rate_hz, cutoff_hz);

    return true;
}
