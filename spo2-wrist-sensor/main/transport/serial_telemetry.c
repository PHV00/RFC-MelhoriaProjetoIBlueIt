// função enviar os dados obtidos pelo canal usb/serial para o jogo  
#include "transport/serial_telemetry.h"

#include <stdio.h>

void serial_telemetry_print_sample(const ppg_sample_t *sample) {
    if (sample == NULL) {
        return;
    }

    printf(
        "[RAW] seq=%lu ts=%lu red=%lu ir=%lu\n",
        (unsigned long)sample->seq,
        (unsigned long)sample->timestamp_ms,
        (unsigned long)sample->red,
        (unsigned long)sample->ir
    );
}

void serial_telemetry_print_frame(const char *state_str, const health_frame_t *frame) {
    if (frame == NULL) {
        return;
    }

    printf(
        "[FRAME][STATE=%s] valid=%d finger=%d conf=%.2f "
        "dc_ir=%.1f ac_ir=%.1f pi=%.4f q=%.2f "
        "hr_valid=%d hr=%.2f "
        "spo2_valid=%d spo2=%.2f\n",
        state_str ? state_str : "UNKNOWN",
        frame->valid,
        frame->finger_present,
        frame->confidence,
        frame->quality.dc_ir,
        frame->quality.ac_ir,
        frame->quality.perfusion_index,
        frame->quality.quality_score,
        frame->hr.valid,
        frame->hr.bpm,
        frame->spo2.valid,
        frame->spo2.spo2
    );
}

void serial_telemetry_print_message(const char *tag, const char *msg) {
    printf("[%s] %s\n", tag ? tag : "LOG", msg ? msg : "");
}