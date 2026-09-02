#pragma once

#include "common/measurement_types.h"

typedef enum {
    APP_STATE_BOOT = 0,
    APP_STATE_SELF_TEST,
    APP_STATE_IDLE,
    APP_STATE_SAMPLING,
    APP_STATE_TRACKING,
    APP_STATE_LOW_CONFIDENCE,
    APP_STATE_ERROR
} app_state_t;
