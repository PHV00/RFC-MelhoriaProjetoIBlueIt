#include "app/app_controller.h"

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "app/app_state_machine.h"
#include "drivers/i2c_bus.h"
#include "drivers/max3010x_driver.h"
#include "processing/hr_estimator.h"
#include "processing/sqi/signal_quality.h"
#include "processing/spo2_estimator.h"
#include "safety/confidence_engine.h"
#include "sensing/ppg_sampler.h"
#include "sensing/sample_buffer.h"
#include "storage/config_repo.h"
#include "transport/serial_telemetry.h"

static const char *TAG = "APP";

static max3010x_t s_sensor;
static sample_buffer_t s_buffer;
static ppg_sampler_t s_sampler;
static uint32_t s_last_processing_ms;
static uint32_t s_last_raw_ms;
static uint32_t s_last_recovery_ms;
static uint8_t s_consecutive_errors;
static bool s_ready;

static esp_err_t configure_sensor(const system_config_t *cfg) {
    esp_err_t err = max3010x_probe(&s_sensor);
    if (err != ESP_OK) return err;
    err = max3010x_reset(&s_sensor);
    if (err != ESP_OK) return err;

    const max3010x_config_t sensor_cfg = {
        .sample_rate_hz = cfg->sensor_sample_rate_hz,
        .led_red_pa = cfg->led_red_pa,
        .led_ir_pa = cfg->led_ir_pa,
        .fifo_rollover = false
    };
    return max3010x_configure(&s_sensor, &sensor_cfg);
}

static void enter_error(const char *message) {
    (void)app_state_machine_transition(APP_STATE_ERROR);
    serial_telemetry_print_message("ERROR", message);
    s_ready = false;
}

static void attempt_recovery(uint32_t now_ms) {
    if ((uint32_t)(now_ms - s_last_recovery_ms) < 1000u) return;
    s_last_recovery_ms = now_ms;

    const system_config_t *cfg = config_repo_get();
    (void)app_state_machine_transition(APP_STATE_BOOT);
    (void)app_state_machine_transition(APP_STATE_SELF_TEST);
    if (configure_sensor(cfg) == ESP_OK) {
        sample_buffer_init(&s_buffer);
        ppg_sampler_init(&s_sampler, &s_sensor, &s_buffer, cfg->sensor_sample_rate_hz);
        s_consecutive_errors = 0u;
        s_ready = true;
        (void)app_state_machine_transition(APP_STATE_IDLE);
        serial_telemetry_print_message("RECOVERY", "Sensor reinicializado");
    } else {
        (void)app_state_machine_transition(APP_STATE_ERROR);
    }
}

void app_controller_init(void) {
    const system_config_t *cfg = config_repo_get();
    app_state_machine_reset();
    (void)app_state_machine_transition(APP_STATE_SELF_TEST);
    ESP_LOGI(TAG, "Inicializando prova de conceito de HR/SpO2");

    esp_err_t err = i2c_bus_init(cfg->i2c_sda_gpio, cfg->i2c_scl_gpio, cfg->i2c_freq_hz);
    if (err != ESP_OK) {
        enter_error("Falha ao inicializar I2C");
        return;
    }
    err = max3010x_init(&s_sensor, cfg->sensor_i2c_addr);
    if (err != ESP_OK || configure_sensor(cfg) != ESP_OK) {
        enter_error("Sensor MAX3010x nao identificado/configurado");
        return;
    }

    sample_buffer_init(&s_buffer);
    ppg_sampler_init(&s_sampler, &s_sensor, &s_buffer, cfg->sensor_sample_rate_hz);
    serial_telemetry_print_sensor_info(s_sensor.part_id, s_sensor.revision_id, s_sensor.sample_rate_hz);

    s_last_processing_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    s_last_raw_ms = s_last_processing_ms;
    s_ready = true;
    (void)app_state_machine_transition(APP_STATE_IDLE);
}

void app_controller_step(void) {
    uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    if (!s_ready) {
        attempt_recovery(now_ms);
        return;
    }

    size_t added = 0u;
    esp_err_t err = ppg_sampler_poll(&s_sampler, &added);
    if (err == ESP_ERR_INVALID_STATE) {
        if (app_state_machine_get() == APP_STATE_IDLE) {
            (void)app_state_machine_transition(APP_STATE_SAMPLING);
        }
        (void)app_state_machine_transition(APP_STATE_LOW_CONFIDENCE);
        serial_telemetry_print_message("FIFO", "Overflow detectado; FIFO limpa e janela invalidada");
        sample_buffer_init(&s_buffer);
        return;
    }
    if (err != ESP_OK) {
        s_consecutive_errors++;
        if (s_consecutive_errors >= 3u) enter_error("Falhas I2C consecutivas na aquisicao");
        return;
    }
    s_consecutive_errors = 0u;

    if (added > 0u) {
        (void)app_state_machine_transition(APP_STATE_SAMPLING);
        if ((uint32_t)(now_ms - s_last_raw_ms) >= 200u) {
            ppg_sample_t latest = {0};
            if (sample_buffer_latest(&s_buffer, &latest)) serial_telemetry_print_sample(&latest);
            s_last_raw_ms = now_ms;
        }
    }

    const system_config_t *cfg = config_repo_get();
    if ((uint32_t)(now_ms - s_last_processing_ms) < cfg->sqi.step_ms) return;
    s_last_processing_ms = now_ms;

    signal_quality_t quality = {0};
    hr_result_t hr = {0};
    spo2_result_t spo2 = {0};
    health_frame_t frame = {0};

    bool quality_ready = signal_quality_evaluate_window(
        &s_buffer,
        config_repo_sqi_window_samples(cfg),
        (float)cfg->sensor_sample_rate_hz,
        &quality
    );
    if (!quality_ready) {
        (void)app_state_machine_transition(APP_STATE_LOW_CONFIDENCE);
        return;
    }

    (void)hr_estimator_compute(&s_buffer, &quality, &hr);
    (void)spo2_estimator_compute_with_calibration(&s_buffer, &quality, &cfg->spo2_calibration, &spo2);

    ppg_sample_t latest = {0};
    (void)sample_buffer_latest(&s_buffer, &latest);
    (void)confidence_engine_build_frame_ex(
        &quality,
        &hr,
        &spo2,
        latest.timestamp_ms,
        s_sampler.fifo_overflow_events,
        cfg->sqi.minimum_quality_score,
        &frame
    );

    if (frame.valid) {
        (void)app_state_machine_transition(APP_STATE_TRACKING);
    } else {
        (void)app_state_machine_transition(APP_STATE_LOW_CONFIDENCE);
    }
    serial_telemetry_print_frame(
        app_state_machine_to_string(app_state_machine_get()),
        &frame
    );
}
