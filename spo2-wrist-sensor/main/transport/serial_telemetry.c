#include "transport/serial_telemetry.h"

#include <stdio.h>

static const char *quality_state_to_string(ppg_quality_state_t state) {
    switch (state) {
        case PPG_QUALITY_VALID: return "VALID";
        case PPG_QUALITY_INVALID: return "INVALID";
        case PPG_QUALITY_UNKNOWN:
        default: return "UNKNOWN";
    }
}

static const char *gate_to_string(sqi_gate_id_t gate) {
    switch (gate) {
        case SQI_GATE_G1_INTEGRITY: return "G1_INTEGRITY";
        case SQI_GATE_G2_PULSATILITY: return "G2_PULSATILITY";
        case SQI_GATE_G3_MORPHOLOGY: return "G3_MORPHOLOGY";
        case SQI_GATE_G4_CHANNELS: return "G4_CHANNELS";
        case SQI_GATE_NONE:
        default: return "NONE";
    }
}

static const char *fail_reason_to_string(sqi_fail_reason_t reason) {
    switch (reason) {
        case SQI_FAIL_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case SQI_FAIL_WINDOW_SHORT: return "WINDOW_SHORT";
        case SQI_FAIL_DISCONTINUITY: return "DISCONTINUITY";
        case SQI_FAIL_NO_SIGNAL_RED: return "NO_SIGNAL_RED";
        case SQI_FAIL_NO_SIGNAL_IR: return "NO_SIGNAL_IR";
        case SQI_FAIL_FLATLINE_RED: return "FLATLINE_RED";
        case SQI_FAIL_FLATLINE_IR: return "FLATLINE_IR";
        case SQI_FAIL_CLIPPING_RED: return "CLIPPING_RED";
        case SQI_FAIL_CLIPPING_IR: return "CLIPPING_IR";
        case SQI_FAIL_NONE:
        default: return "NONE";
    }
}

void serial_telemetry_print_sample(const ppg_sample_t *sample) {
    if (sample == NULL) return;
    /* RAW permanece disponível no buffer; a emissão contínua fica desabilitada
     * para não saturar a serial durante os testes do pipeline.
     */
}

void serial_telemetry_print_frame(const char *state_str, const health_frame_t *frame) {
    if (frame == NULL) return;

    printf(
        "{\"v\":%d,\"type\":\"oximetry\",\"state\":\"%s\","
        "\"ts_ms\":%lu,\"valid\":%s,\"finger\":%s,"
        "\"quality_state\":\"%s\",\"failed_gate\":\"%s\",\"fail_reason\":\"%s\","
        "\"quality\":%.2f,\"pi\":%.4f,"
        "\"g1\":{\"passed\":%s,\"mask\":%lu,\"continuity\":%.4f,"
        "\"red_mean\":%.1f,\"red_range\":%lu,\"red_clip\":%.4f,"
        "\"ir_mean\":%.1f,\"ir_range\":%lu,\"ir_clip\":%.4f},"
        "\"hr\":{\"valid\":%s,\"bpm\":%.1f,\"confidence\":%.2f},"
        "\"spo2\":{\"valid\":%s,\"value\":%.1f,\"r\":%.3f,"
        "\"confidence\":%.2f}}\n",
        TELEMETRY_PROTOCOL_VERSION,
        state_str != NULL ? state_str : "UNKNOWN",
        (unsigned long)frame->timestamp_ms,
        frame->valid ? "true" : "false",
        frame->finger_present ? "true" : "false",
        quality_state_to_string(frame->quality.state),
        gate_to_string(frame->quality.failed_gate),
        fail_reason_to_string(frame->quality.fail_reason),
        frame->quality.quality_score,
        frame->quality.perfusion_index,
        frame->quality.g1.passed ? "true" : "false",
        (unsigned long)frame->quality.g1.failure_mask,
        frame->quality.g1.continuity_fraction,
        frame->quality.g1.red_mean,
        (unsigned long)frame->quality.g1.red_range,
        frame->quality.g1.red_clipping_fraction,
        frame->quality.g1.ir_mean,
        (unsigned long)frame->quality.g1.ir_range,
        frame->quality.g1.ir_clipping_fraction,
        frame->hr.valid ? "true" : "false",
        frame->hr.bpm,
        frame->hr.confidence,
        frame->spo2.valid ? "true" : "false",
        frame->spo2.spo2,
        frame->spo2.ratio_r,
        frame->spo2.confidence
    );
}

void serial_telemetry_print_sensor_info(uint8_t part_id, uint8_t revision_id, uint16_t sample_rate_hz) {
    printf("{\"v\":%d,\"type\":\"sensor_info\",\"part_id\":%u,\"revision_id\":%u,\"sample_rate_hz\":%u}\n",
           TELEMETRY_PROTOCOL_VERSION, part_id, revision_id, sample_rate_hz);
}

void serial_telemetry_print_message(const char *tag, const char *msg) {
    printf("{\"v\":%d,\"type\":\"log\",\"tag\":\"%s\",\"message\":\"%s\"}\n",
           TELEMETRY_PROTOCOL_VERSION,
           tag != NULL ? tag : "LOG",
           msg != NULL ? msg : "");
}
