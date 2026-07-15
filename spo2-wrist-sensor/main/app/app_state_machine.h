#pragma once

#include <stdbool.h>

#include "app/app_types.h"

void app_state_machine_reset(void);
bool app_state_machine_transition(app_state_t next_state);
void app_state_machine_set(app_state_t state); /* Compatibilidade; valida a transição. */
app_state_t app_state_machine_get(void);
const char *app_state_machine_to_string(app_state_t state);
