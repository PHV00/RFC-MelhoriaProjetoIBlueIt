#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"


#define ADC_UNIT       ADC_UNIT_1
#define ADC_CHANNEL    ADC_CHANNEL_0   // GPIO0

#define R1 2200.0f
#define R2 1500.0f

#define DIVIDER_FACTOR ((R1 + R2) / R2)

static const char *TAG = "PITACO_TEST";

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t cali_handle;

static bool adc_calibrated = false;


void app_main(void)
{
    ESP_LOGI(TAG, "Teste XGZP6897A + ESP32-C3");

    // 1. Inicializa ADC1
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&unit_cfg, &adc_handle)
    );


    // 2. Configura GPIO0 / ADC1_CH0
    adc_oneshot_chan_cfg_t channel_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            ADC_CHANNEL,
            &channel_cfg
        )
    );


    // 3. Tenta habilitar calibração do ADC
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT,
        .chan = ADC_CHANNEL,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    if (adc_cali_create_scheme_curve_fitting(
            &cali_cfg,
            &cali_handle) == ESP_OK)
    {
        adc_calibrated = true;
        ESP_LOGI(TAG, "ADC calibrado.");
    }
    else
    {
        ESP_LOGW(TAG, "ADC sem calibracao.");
    }


    // 4. Loop de leitura
    float zero_mv = 0.0f;

printf("\nAguardando estabilizacao do sensor...\n");
vTaskDelay(pdMS_TO_TICKS(1000));

printf("Calibrando ZERO. Nao toque nas entradas...\n");

/* 40 amostras x 50 ms = 2 segundos */
for (int i = 0; i < 40; i++)
{
    int raw = 0;
    int adc_mv = 0;

    ESP_ERROR_CHECK(
        adc_oneshot_read(
            adc_handle,
            ADC_CHANNEL,
            &raw
        )
    );

    ESP_ERROR_CHECK(
        adc_cali_raw_to_voltage(
            cali_handle,
            raw,
            &adc_mv
        )
    );

    float sensor_mv =
        adc_mv * DIVIDER_FACTOR;

    zero_mv += sensor_mv;

    vTaskDelay(pdMS_TO_TICKS(50));
}

zero_mv /= 40.0f;

printf("\nZERO = %.1f mV\n", zero_mv);
printf("Iniciando leitura...\n\n");


while (1)
{
    int raw = 0;
    int adc_mv = 0;

    ESP_ERROR_CHECK(
        adc_oneshot_read(
            adc_handle,
            ADC_CHANNEL,
            &raw
        )
    );

    ESP_ERROR_CHECK(
        adc_cali_raw_to_voltage(
            cali_handle,
            raw,
            &adc_mv
        )
    );

    float sensor_mv =
        adc_mv * DIVIDER_FACTOR;

    float delta_mv =
        sensor_mv - zero_mv;

    printf(
        "RAW=%4d | ADC=%4d mV | "
        "XGZP=%7.1f mV | "
        "DELTA=%+7.1f mV\n",
        raw,
        adc_mv,
        sensor_mv,
        delta_mv
    );

    vTaskDelay(pdMS_TO_TICKS(50));
    }   
}