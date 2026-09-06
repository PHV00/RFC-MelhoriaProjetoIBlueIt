#pragma once

#include "common/measurement_types.h"

#define TELEMETRY_PROTOCOL_VERSION 1

void serial_telemetry_print_sample(const ppg_sample_t *sample);
void serial_telemetry_print_frame(const char *state_str, const health_frame_t *frame);
void serial_telemetry_print_sensor_info(uint8_t part_id, uint8_t revision_id, uint16_t sample_rate_hz);
void serial_telemetry_print_message(const char *tag, const char *msg);
