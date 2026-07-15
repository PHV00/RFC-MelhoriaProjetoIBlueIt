#include "transport/serial_telemetry.h"

#include <stdio.h>

void serial_telemetry_print_sample(const ppg_sample_t *sample) {
    if (sample == NULL) return;
    /* printf("{\"v\":%d,\"type\":\"ppg\",\"seq\":%lu,\"ts_ms\":%lu,\"red\":%lu,\"ir\":%lu}\n",
           TELEMETRY_PROTOCOL_VERSION,
           (unsigned long)sample->seq,
           (unsigned long)sample->timestamp_ms,
           (unsigned long)sample->red,
           (unsigned long)sample->ir);
    */
}

void serial_telemetry_print_frame(const char *state_str, const health_frame_t *frame) {
    if (frame == NULL) return;
    /*
    printf(
        "{\"v\":%d,\"type\":\"health_frame\",\"state\":\"%s\","
        "\"ts_ms\":%lu,\"valid\":%s,\"clinical_valid\":%s,\"finger\":%s,"
        "\"confidence\":%.4f,\"invalid_reasons\":%lu,\"fifo_overflows\":%lu,"
        "\"quality\":{\"score\":%.4f,\"pi\":%.6f,\"snr\":%.3f,"
        "\"corr\":%.4f,\"continuity\":%.4f,\"clipping\":%.6f},"
        "\"hr\":{\"valid\":%s,\"bpm\":%.2f,\"ibi_ms\":%.2f,\"confidence\":%.4f,\"status\":%d},"
        "\"spo2\":{\"valid\":%s,\"calibrated\":%s,\"value\":%.2f,\"r\":%.5f,"
        "\"confidence\":%.4f,\"status\":%d,\"calibration_version\":%lu}}\n",
        TELEMETRY_PROTOCOL_VERSION,
        state_str != NULL ? state_str : "UNKNOWN",
        (unsigned long)frame->timestamp_ms,
        frame->valid ? "true" : "false",
        frame->clinical_valid ? "true" : "false",
        frame->finger_present ? "true" : "false",
        frame->confidence,
        (unsigned long)frame->invalid_reasons,
        (unsigned long)frame->fifo_overflow_events,
        frame->quality.quality_score,
        frame->quality.perfusion_index,
        frame->quality.snr,
        frame->quality.red_ir_correlation,
        frame->quality.continuity_score,
        frame->quality.clipping_fraction,
        frame->hr.valid ? "true" : "false",
        frame->hr.bpm,
        frame->hr.ibi_ms,
        frame->hr.confidence,
        (int)frame->hr.status,
        frame->spo2.valid ? "true" : "false",
        frame->spo2.calibrated ? "true" : "false",
        frame->spo2.spo2,
        frame->spo2.ratio_r,
        frame->spo2.confidence,
        (int)frame->spo2.status,
        (unsigned long)frame->spo2.calibration_version
    );
    */
    printf(
        "{\"v\":%d,\"type\":\"oximetry\",\"state\":\"%s\","
        "\"ts_ms\":%lu,\"valid\":%s,\"finger\":%s,"
        "\"quality\":%.2f,\"pi\":%.4f,"
        "\"hr\":{\"valid\":%s,\"bpm\":%.1f,\"confidence\":%.2f},"
        "\"spo2\":{\"valid\":%s,\"value\":%.1f,\"r\":%.3f,"
        "\"confidence\":%.2f}}\n",

        TELEMETRY_PROTOCOL_VERSION,
        state_str != NULL ? state_str : "UNKNOWN",
        (unsigned long)frame->timestamp_ms,
        frame->valid ? "true" : "false",
        frame->finger_present ? "true" : "false",

        frame->quality.quality_score,
        frame->quality.perfusion_index,

        frame->hr.valid ? "true" : "false",
        frame->hr.bpm,
        frame->hr.confidence,

        frame->spo2.valid ? "true" : "false",
        frame->spo2.spo2,
        frame->spo2.ratio_r,
        frame->spo2.confidence
    );
    /*
    printf(
    "{\"type\":\"oximetry\",\"ts_ms\":%lu,"
    "\"valid\":%s,\"finger\":%s,"
    "\"hr\":%.1f,\"spo2\":%.1f,\"confidence\":%.2f}\n",

    (unsigned long)frame->timestamp_ms,
    frame->valid ? "true" : "false",
    frame->finger_present ? "true" : "false",
    frame->hr.bpm,
    frame->spo2.spo2,
    frame->confidence
    );
    */
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
