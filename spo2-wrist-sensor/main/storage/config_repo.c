#include "storage/config_repo.h"

static const system_config_t g_config = {
    .sensor_i2c_addr = 0x57,
    .i2c_sda_gpio = 4,
    .i2c_scl_gpio = 5,
    .i2c_freq_hz = 400000,
    .sensor_sample_rate_hz = 100,
    .acquisition_poll_ms = 10,
    .led_red_pa = 0x24,
    .led_ir_pa = 0x24,
    .sqi = {
        /* Baseline científica adotada a partir da implementação dos gates. */
        .window_ms = 5000u,
        .step_ms = 1000u,
        /* Mantido temporariamente para compatibilidade com o confidence_engine
         * enquanto o SQI hierárquico substitui gradualmente o score legado.
         */
        .minimum_quality_score = 0.55f,
        .g1_integrity = {
            /* O driver configura o MAX3010x em 411 us / 18 bits. */
            .adc_min_value = 0u,
            .adc_max_value = 0x03FFFFu,
            .rail_margin_counts = 1u,
            /* Thresholds iniciais de engenharia. Devem ser calibrados no perfil
             * NO_GRIP com dados RAW representativos antes de serem congelados.
             */
            .minimum_mean_level = 5000u,
            .minimum_raw_range = 20u,
            .maximum_clipping_fraction = 0.01f,
            .minimum_continuity_fraction = 0.95f,
            .maximum_interval_deviation_fraction = 0.40f
        }
    },
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

size_t config_repo_sqi_window_samples(const system_config_t *config) {
    if (config == NULL || config->sensor_sample_rate_hz == 0u || config->sqi.window_ms == 0u) {
        return 0u;
    }

    uint64_t scaled = (uint64_t)config->sensor_sample_rate_hz * (uint64_t)config->sqi.window_ms;
    return (size_t)(scaled / 1000u);
}
