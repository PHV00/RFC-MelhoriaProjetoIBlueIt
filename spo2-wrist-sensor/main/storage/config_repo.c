#include "storage/config_repo.h"

static const system_config_t g_config = {
    .sensor_i2c_addr = 0x57,
    .i2c_sda_gpio = 4,
    .i2c_scl_gpio = 5,
    .i2c_freq_hz = 400000,
    .sensor_sample_rate_hz = 100,
    .acquisition_poll_ms = 10,
    .processing_interval_ms = 500,
    .processing_window_samples = 400,
    .led_red_pa = 0x24,
    .led_ir_pa = 0x24,
    .minimum_quality_score = 0.55f,
    .spo2_calibration = {
        /* Curva apenas para demonstração de engenharia: SpO2 = 110 - 25R.
         * Substituir por coeficientes obtidos para o conjunto sensor + encapsulamento.
         */
        .a = 0.0f,
        .b = -25.0f,
        .c = 110.0f,
        .min_ratio_r = 0.20f,
        .max_ratio_r = 1.40f,
        .calibrated = false,
        .allow_uncalibrated_estimate = true,
        .version = 0u
    }
};

const system_config_t *config_repo_get(void) {
    return &g_config;
}
