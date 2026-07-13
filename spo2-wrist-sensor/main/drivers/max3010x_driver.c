// ajustar este driver para o modelo adequado do datasheet do modulo max30105

/*
max3010x_init()
    cria a representação do sensor no software

max3010x_reset()
    envia comando de reset ao hardware

max3010x_config_default()
    configura FIFO, modo, ADC e LEDs

max3010x_read_sample()
    lê seis bytes da FIFO
    ├── três bytes RED
    └── três bytes IR
    junta os bytes
    remove bits inválidos
    grava em ppg_sample_t
*/

#include "drivers/max3010x_driver.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "drivers/i2c_bus.h"

/*
Esses são os endereços dos registradores internos do sensor.

Um registrador é uma pequena posição de memória dentro do componente eletrônico.

registrador 0x04 → ponteiro de escrita da FIFO
registrador 0x05 → contador de overflow
registrador 0x06 → ponteiro de leitura da FIFO
registrador 0x07 → dados da FIFO
registrador 0x09 → modo de funcionamento

*/

//write reg registra nas variaveis do sensor os valores correspondentes, abaixo está a localização de cada um destes locais no Max3010x
#define REG_INTR_STATUS_1     0x00
#define REG_INTR_STATUS_2     0x01
#define REG_INTR_ENABLE_1     0x02
#define REG_INTR_ENABLE_2     0x03
#define REG_FIFO_WR_PTR       0x04
#define REG_OVF_COUNTER       0x05
// OVF_COUNTER → quantas amostras foram perdidas por excesso
#define REG_FIFO_RD_PTR       0x06
#define REG_FIFO_DATA         0x07
#define REG_FIFO_CONFIG       0x08
#define REG_MODE_CONFIG       0x09
#define REG_SPO2_CONFIG       0x0A
#define REG_LED1_PA           0x0C
#define REG_LED2_PA           0x0D
#define REG_PART_ID           0xFF

/*
registrador REG_MODE_CONFIG
    recebe 0x40 → reiniciar sensor
    recebe 0x03 → operar no modo RED + IR

    modos descritos abaixo
*/
#define MODE_RESET_BIT        0x40
#define MODE_SPO2_EN          0x03

//O static significa que essa função só pode ser usada dentro deste arquivo .c
// função responsavel por registrar os valores nas variaveis nativas do hardware do sensor max3010x
static esp_err_t write_reg(max3010x_t *dev, uint8_t reg, uint8_t value) {
    return i2c_bus_write(dev->i2c_addr, reg, &value, 1);
}

/*
dev->i2c_addr → endereço I²C do sensor
reg           → registrador
&value        → endereço do byte
1             → quantidade de bytes
*/

static esp_err_t read_reg(max3010x_t *dev, uint8_t reg, uint8_t *value) {
    return i2c_bus_read(dev->i2c_addr, reg, value, 1);
}

esp_err_t max3010x_init(max3010x_t *dev, uint8_t i2c_addr) {
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    //menset preenche uma região da memoria
    memset(dev, 0, sizeof(*dev));
    dev->i2c_addr = i2c_addr;
    dev->initialized = true;

    return ESP_OK;
}

esp_err_t max3010x_reset(max3010x_t *dev) {
    if (dev == NULL || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = write_reg(dev, REG_MODE_CONFIG, MODE_RESET_BIT);
    if (err != ESP_OK) {
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(20));
    return ESP_OK;
}

esp_err_t max3010x_config_default(max3010x_t *dev) {
    if (dev == NULL || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err;

    //write reg registra nas variaveis do sensor os valores correspondentes
    err = write_reg(dev, REG_INTR_ENABLE_1, 0x00);
    if (err != ESP_OK) return err;

    err = write_reg(dev, REG_INTR_ENABLE_2, 0x00);
    if (err != ESP_OK) return err;

    err = write_reg(dev, REG_FIFO_WR_PTR, 0x00);
    if (err != ESP_OK) return err;

    err = write_reg(dev, REG_OVF_COUNTER, 0x00);
    if (err != ESP_OK) return err;

    err = write_reg(dev, REG_FIFO_RD_PTR, 0x00);
    if (err != ESP_OK) return err;

    err = write_reg(dev, REG_FIFO_CONFIG, 0x0F);
    if (err != ESP_OK) return err;

    err = write_reg(dev, REG_MODE_CONFIG, MODE_SPO2_EN);
    if (err != ESP_OK) return err;

    err = write_reg(dev, REG_SPO2_CONFIG, 0x27);
    if (err != ESP_OK) return err;

    /*
    Essa é uma parte importante que precisaremos revisar academicamente, porque a frequência de amostragem e a largura de pulso afetam:

    qualidade do PPG;
    consumo;
    resolução;
    saturação do ADC;
    cálculo de frequência cardíaca e SpO₂.
    */

    err = write_reg(dev, REG_LED1_PA, 0x24);
    if (err != ESP_OK) return err;

    err = write_reg(dev, REG_LED2_PA, 0x24);
    if (err != ESP_OK) return err;

    // ajuste da luminosidade dos leds
    /*
    Um valor maior tende a produzir mais luz, mas também:

    aumenta o consumo;
    pode saturar o sinal;
    pode gerar aquecimento;
    não garante melhor qualidade.

    Um valor muito baixo pode gerar sinal fraco.

    Um valor muito alto pode fazer a leitura chegar ao limite do ADC.

    Por isso essa configuração geralmente precisa ser calibrada ou ajustada.
    */

    return ESP_OK;
}

esp_err_t max3010x_read_sample(max3010x_t *dev, ppg_sample_t *sample) {
    if (dev == NULL || sample == NULL || !dev->initialized) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t raw[6] = {0};
    esp_err_t err = i2c_bus_read(dev->i2c_addr, REG_FIFO_DATA, raw, sizeof(raw));
    if (err != ESP_OK) {
        return err;
    }

    uint32_t red = ((uint32_t)raw[0] << 16) | ((uint32_t)raw[1] << 8) | raw[2];
    uint32_t ir  = ((uint32_t)raw[3] << 16) | ((uint32_t)raw[4] << 8) | raw[5];

    sample->red = red & 0x3FFFF;
    sample->ir = ir & 0x3FFFF;

    return ESP_OK;
}
