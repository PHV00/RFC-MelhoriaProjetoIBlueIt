#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "drivers/i2c_bus.h"
#include "drivers/max3010x_driver.h"

/*
 * PoC de coexistencia no mesmo ESP32-C3:
 *
 * XGZP6897A:
 *   VDD -> 5V/VBUS
 *   GND -> GND comum
 *   OUT -> R1 -> GPIO0 -> R2 -> GND
 *
 * MAX3010x/MAX30102:
 *   VCC -> 3V3 (conforme breakout usado no projeto)
 *   GND -> GND comum
 *   SDA -> GPIO6
 *   SCL -> GPIO7
 *
 * Este teste NAO converte o XGZP para Pa. A variante/faixa exata do sensor
 * ainda precisa ser confirmada. Para diagnostico, usamos a tensao reconstruida
 * e o delta em relacao ao zero.
 */

/* Pneumotacografo -------------------------------------------------------- */
#define PNEUMO_ADC_UNIT       ADC_UNIT_1
#define PNEUMO_ADC_CHANNEL    ADC_CHANNEL_0   /* GPIO0 */

/* Valores medidos na bancada. */
#define PNEUMO_R1_OHM         2170.0f
#define PNEUMO_R2_OHM         1500.0f
#define PNEUMO_DIVIDER_FACTOR ((PNEUMO_R1_OHM + PNEUMO_R2_OHM) / PNEUMO_R2_OHM)

#define PNEUMO_PERIOD_MS      50u
#define ZERO_SAMPLES          40u
#define ZERO_SAMPLE_MS        50u

/* PPG ------------------------------------------------------------------- */
#define PPG_I2C_SDA_GPIO      6
#define PPG_I2C_SCL_GPIO      7
#define PPG_I2C_FREQ_HZ       400000u
#define MAX3010X_I2C_ADDR     0x57u
#define PPG_POLL_MS           10u
#define PPG_BATCH_CAPACITY    32u

static const char *TAG = "PITACO_SPO2_TEST";

static adc_oneshot_unit_handle_t s_adc_handle;
static adc_cali_handle_t s_cali_handle;
static bool s_adc_calibrated = false;

static max3010x_t s_max3010x;
static bool s_ppg_ready = false;

static float s_zero_sensor_mv = 0.0f;

static int s_last_adc_raw = 0;
static int s_last_adc_mv = -1;
static float s_last_sensor_mv = 0.0f;
static float s_last_delta_mv = 0.0f;

static uint32_t s_last_red = 0u;
static uint32_t s_last_ir = 0u;
static uint32_t s_ppg_samples_total = 0u;
static uint32_t s_ppg_read_errors = 0u;

static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

static esp_err_t pneumo_adc_init(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = PNEUMO_ADC_UNIT,
    };

    esp_err_t err = adc_oneshot_new_unit(&unit_cfg, &s_adc_handle);
    if (err != ESP_OK) return err;

    adc_oneshot_chan_cfg_t channel_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    err = adc_oneshot_config_channel(
        s_adc_handle,
        PNEUMO_ADC_CHANNEL,
        &channel_cfg
    );
    if (err != ESP_OK) return err;

    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = PNEUMO_ADC_UNIT,
        .chan = PNEUMO_ADC_CHANNEL,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    err = adc_cali_create_scheme_curve_fitting(&cali_cfg, &s_cali_handle);
    if (err == ESP_OK) {
        s_adc_calibrated = true;
        ESP_LOGI(TAG, "ADC calibrado");
        return ESP_OK;
    }

    s_adc_calibrated = false;
    ESP_LOGW(TAG, "ADC sem esquema de calibracao; RAW continua disponivel");
    return ESP_OK;
}

static esp_err_t pneumo_read_once(
    int *raw,
    int *adc_mv,
    float *sensor_mv
)
{
    if (raw == NULL || adc_mv == NULL || sensor_mv == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int local_raw = 0;
    esp_err_t err = adc_oneshot_read(
        s_adc_handle,
        PNEUMO_ADC_CHANNEL,
        &local_raw
    );
    if (err != ESP_OK) return err;

    int local_mv = -1;
    if (s_adc_calibrated) {
        err = adc_cali_raw_to_voltage(
            s_cali_handle,
            local_raw,
            &local_mv
        );
        if (err != ESP_OK) return err;
    }

    *raw = local_raw;
    *adc_mv = local_mv;
    *sensor_mv = (local_mv >= 0)
        ? ((float)local_mv * PNEUMO_DIVIDER_FACTOR)
        : -1.0f;

    return ESP_OK;
}

static esp_err_t pneumo_calibrate_zero(void)
{
    if (!s_adc_calibrated) {
        ESP_LOGW(TAG, "Zero em mV ignorado porque ADC nao esta calibrado");
        s_zero_sensor_mv = 0.0f;
        return ESP_OK;
    }

    ESP_LOGI(
        TAG,
        "Calibrando zero do XGZP: mantenha P1/P2 sem fluxo por %u ms",
        (unsigned)(ZERO_SAMPLES * ZERO_SAMPLE_MS)
    );

    float sum_mv = 0.0f;

    for (uint32_t i = 0u; i < ZERO_SAMPLES; ++i) {
        int raw = 0;
        int adc_mv = -1;
        float sensor_mv = 0.0f;

        esp_err_t err = pneumo_read_once(&raw, &adc_mv, &sensor_mv);
        if (err != ESP_OK) return err;

        sum_mv += sensor_mv;
        vTaskDelay(pdMS_TO_TICKS(ZERO_SAMPLE_MS));
    }

    s_zero_sensor_mv = sum_mv / (float)ZERO_SAMPLES;
    ESP_LOGI(TAG, "ZERO XGZP = %.1f mV", s_zero_sensor_mv);
    return ESP_OK;
}

static esp_err_t ppg_init(void)
{
    esp_err_t err = i2c_bus_init(
        PPG_I2C_SDA_GPIO,
        PPG_I2C_SCL_GPIO,
        PPG_I2C_FREQ_HZ
    );
    if (err != ESP_OK) return err;

    err = max3010x_init(&s_max3010x, MAX3010X_I2C_ADDR);
    if (err != ESP_OK) return err;

    err = max3010x_probe(&s_max3010x);
    if (err != ESP_OK) return err;

    ESP_LOGI(
        TAG,
        "MAX3010x detectado: part_id=0x%02X rev_id=0x%02X",
        s_max3010x.part_id,
        s_max3010x.revision_id
    );

    err = max3010x_reset(&s_max3010x);
    if (err != ESP_OK) return err;

    err = max3010x_config_default(&s_max3010x);
    if (err != ESP_OK) return err;

    s_ppg_ready = true;
    return ESP_OK;
}

static void ppg_poll(void)
{
    if (!s_ppg_ready) return;

    ppg_sample_t batch[PPG_BATCH_CAPACITY] = {0};
    size_t count = 0u;

    esp_err_t err = max3010x_read_fifo(
        &s_max3010x,
        batch,
        PPG_BATCH_CAPACITY,
        &count
    );

    if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(
            TAG,
            "FIFO overflow; eventos=%lu",
            (unsigned long)s_max3010x.fifo_overflow_events
        );
        return;
    }

    if (err != ESP_OK) {
        ++s_ppg_read_errors;
        ESP_LOGW(
            TAG,
            "Falha de leitura PPG: %s (erros=%lu)",
            esp_err_to_name(err),
            (unsigned long)s_ppg_read_errors
        );
        return;
    }

    if (count == 0u) return;

    const ppg_sample_t *latest = &batch[count - 1u];
    s_last_red = latest->red;
    s_last_ir = latest->ir;
    s_ppg_samples_total += (uint32_t)count;
}

void app_main(void)
{
    ESP_LOGI(TAG, "PoC XGZP6897A + MAX3010x no mesmo ESP32-C3");
    ESP_LOGI(
        TAG,
        "Pinos: XGZP ADC=GPIO0 | MAX3010x SDA=GPIO6 SCL=GPIO7"
    );
    ESP_LOGI(
        TAG,
        "Divisor: R1=%.0f ohm R2=%.0f ohm fator=%.4f",
        PNEUMO_R1_OHM,
        PNEUMO_R2_OHM,
        PNEUMO_DIVIDER_FACTOR
    );

    ESP_ERROR_CHECK(pneumo_adc_init());
    ESP_ERROR_CHECK(pneumo_calibrate_zero());

    esp_err_t ppg_err = ppg_init();
    if (ppg_err != ESP_OK) {
        s_ppg_ready = false;
        ESP_LOGE(
            TAG,
            "MAX3010x indisponivel: %s. Teste segue com pneumotacografo.",
            esp_err_to_name(ppg_err)
        );
    }

    printf(
        "\n# CSV\n"
        "# ms,adc_raw,adc_mv,xgzp_mv,delta_mv,red,ir,ppg_samples,"
        "fifo_overflows,ppg_read_errors,ppg_ready\n"
    );

    uint32_t last_ppg_poll_ms = 0u;
    uint32_t last_pneumo_ms = 0u;

    while (1) {
        uint32_t t_ms = now_ms();

        if ((uint32_t)(t_ms - last_ppg_poll_ms) >= PPG_POLL_MS) {
            last_ppg_poll_ms = t_ms;
            ppg_poll();
        }

        if ((uint32_t)(t_ms - last_pneumo_ms) >= PNEUMO_PERIOD_MS) {
            last_pneumo_ms = t_ms;

            int raw = 0;
            int adc_mv = -1;
            float sensor_mv = -1.0f;

            esp_err_t err = pneumo_read_once(&raw, &adc_mv, &sensor_mv);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Falha ADC: %s", esp_err_to_name(err));
            } else {
                s_last_adc_raw = raw;
                s_last_adc_mv = adc_mv;
                s_last_sensor_mv = sensor_mv;
                s_last_delta_mv = (sensor_mv >= 0.0f)
                    ? (sensor_mv - s_zero_sensor_mv)
                    : 0.0f;

                printf(
                    "%lu,%d,%d,%.1f,%+.1f,%lu,%lu,%lu,%lu,%lu,%d\n",
                    (unsigned long)t_ms,
                    s_last_adc_raw,
                    s_last_adc_mv,
                    s_last_sensor_mv,
                    s_last_delta_mv,
                    (unsigned long)s_last_red,
                    (unsigned long)s_last_ir,
                    (unsigned long)s_ppg_samples_total,
                    (unsigned long)s_max3010x.fifo_overflow_events,
                    (unsigned long)s_ppg_read_errors,
                    s_ppg_ready ? 1 : 0
                );
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
