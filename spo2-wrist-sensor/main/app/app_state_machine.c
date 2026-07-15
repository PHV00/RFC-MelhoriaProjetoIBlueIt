#include "app/app_state_machine.h"

static app_state_t g_state = APP_STATE_BOOT;

static bool transition_allowed(app_state_t from, app_state_t to) {
    if (from == to) return true;
    if (to == APP_STATE_ERROR) return true;

    switch (from) {
        case APP_STATE_BOOT:
            return to == APP_STATE_SELF_TEST;
        case APP_STATE_SELF_TEST:
            return to == APP_STATE_IDLE;
        case APP_STATE_IDLE:
            return to == APP_STATE_SAMPLING || to == APP_STATE_SELF_TEST;
        case APP_STATE_SAMPLING:
            return to == APP_STATE_TRACKING || to == APP_STATE_LOW_CONFIDENCE || to == APP_STATE_IDLE;
        case APP_STATE_TRACKING:
            return to == APP_STATE_LOW_CONFIDENCE || to == APP_STATE_SAMPLING || to == APP_STATE_IDLE;
        case APP_STATE_LOW_CONFIDENCE:
            return to == APP_STATE_TRACKING || to == APP_STATE_SAMPLING || to == APP_STATE_IDLE;
        case APP_STATE_ERROR:
            return to == APP_STATE_BOOT || to == APP_STATE_SELF_TEST;
        default:
            return false;
    }
}

void app_state_machine_reset(void) {
    g_state = APP_STATE_BOOT;
}

bool app_state_machine_transition(app_state_t next_state) {
    if (!transition_allowed(g_state, next_state)) return false;
    g_state = next_state;
    return true;
}

void app_state_machine_set(app_state_t state) {
    (void)app_state_machine_transition(state);
}

app_state_t app_state_machine_get(void) {
    return g_state;
}

const char *app_state_machine_to_string(app_state_t state) {
    switch (state) {
        case APP_STATE_BOOT: return "BOOT";
        case APP_STATE_SELF_TEST: return "SELF_TEST";
        case APP_STATE_IDLE: return "IDLE";
        case APP_STATE_SAMPLING: return "SAMPLING";
        case APP_STATE_TRACKING: return "TRACKING";
        case APP_STATE_LOW_CONFIDENCE: return "LOW_CONFIDENCE";
        case APP_STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
