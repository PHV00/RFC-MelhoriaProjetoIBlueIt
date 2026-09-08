#include "processing/sqi/features/threshold_crossing.h"

static int classify_side(float value, float threshold) {
    if (value > threshold) return 1;
    if (value < threshold) return -1;
    return 0;
}

bool threshold_crossing_count(
    const float *samples,
    size_t count,
    float threshold,
    uint32_t *out_crossings
) {
    if (samples == NULL || out_crossings == NULL || count < 2u) {
        return false;
    }

    uint32_t crossings = 0u;
    int previous_side = 0;

    for (size_t i = 0u; i < count; ++i) {
        const int side = classify_side(samples[i], threshold);
        if (side == 0) {
            continue;
        }

        if (previous_side != 0 && side != previous_side) {
            crossings++;
        }
        previous_side = side;
    }

    *out_crossings = crossings;
    return true;
}
