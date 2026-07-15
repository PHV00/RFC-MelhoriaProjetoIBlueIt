#include "drivers/max3010x_driver.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "drivers/i2c_bus.h"

#define REG_INTR_STATUS_1  0x00
#define REG_INTR_STATUS_2  0x01
#define REG_INTR_ENABLE_1  0x02
#define REG_INTR_ENABLE_2  0x03
#define REG_FIFO_WR_PTR    0x04
#define REG_OVF_COUNTER    0x05
#define REG_FIFO_RD_PTR    0x06
#define REG_FIFO_DATA      0x07
#define REG_FIFO_CONFIG    0x08
#define REG_MODE_CONFIG    0x09
#define REG_SPO2_CONFIG    0x0A
#define REG_LED1_PA        0x0C
#define REG_LED2_PA        0x0D
#define REG_REV_ID         0xFE
#define REG_PART_ID        0xFF

#define MODE_RESET_BIT     0x40
#define MODE_SPO2_EN       0x03
#define FIFO_DEPTH         32u
#define FIFO_POINTER_MASK  0x1Fu
#define PPG_SAMPLE_BYTES   6u
#define ADC_18BIT_MASK     0x03FFFFu

static esp_err_t write_reg(max3010x_t *dev, uint8_t reg, uint8_t value) {
    if (dev == NULL) return ESP_ERR_INVALID_ARG;
    return i2c_bus_write(dev->i2c_addr, reg, &value, 1u);
}

static esp_err_t read_reg(max3010x_t *dev, uint8_t reg, uint8_t *value) {
    if (dev == NULL || value == NULL) return ESP_ERR_INVALID_ARG;
    return i2c_bus_read(dev->i2c_addr, reg, value, 1u);
}

static esp_err_t write_verify(max3010x_t *dev, uint8_t reg, uint8_t value, uint8_t mask) {
    esp_err_t err = write_reg(dev, reg, value);
    if (err != ESP_OK) return err;
    uint8_t readback = 0u;
    err = read_reg(dev, reg, &readback);
    if (err != ESP_OK) return err;
    return ((readback & mask) == (value & mask)) ? ESP_OK : ESP_ERR_INVALID_RESPONSE;
}

static esp_err_t encode_sample_rate(uint16_t sample_rate_hz, uint8_t *bits) {
    if (bits == NULL) return ESP_ERR_INVALID_ARG;
    switch (sample_rate_hz) {
        case 50:   *bits = 0u << 2; break;
        case 100:  *bits = 1u << 2; break;
        case 200:  *bits = 2u << 2; break;
        case 400:  *bits = 3u << 2; break;
        case 800:  *bits = 4u << 2; break;
        case 1000: *bits = 5u << 2; break;
        case 1600: *bits = 6u << 2; break;
        case 3200: *bits = 7u << 2; break;
        default: return ESP_ERR_NOT_SUPPORTED;
    }
    return ESP_OK;
}

esp_err_t max3010x_init(max3010x_t *dev, uint8_t i2c_addr) {
    if (dev == NULL || i2c_addr == 0u || i2c_addr >= 0x80u) return ESP_ERR_INVALID_ARG;
    memset(dev, 0, sizeof(*dev));
    dev->i2c_addr = i2c_addr;
    dev->initialized = true;
    return ESP_OK;
}

esp_err_t max3010x_probe(max3010x_t *dev) {
    if (dev == NULL || !dev->initialized) return ESP_ERR_INVALID_STATE;
    esp_err_t err = read_reg(dev, REG_REV_ID, &dev->revision_id);
    if (err != ESP_OK) return err;
    err = read_reg(dev, REG_PART_ID, &dev->part_id);
    if (err != ESP_OK) return err;

    /* O Part ID 0x15 aparece em módulos compatíveis da família; ele não é
     * suficiente, sozinho, para distinguir todas as placas comerciais.
     */
    dev->identified = dev->part_id != 0x00u && dev->part_id != 0xFFu;
    return dev->identified ? ESP_OK : ESP_ERR_NOT_FOUND;
}

esp_err_t max3010x_reset(max3010x_t *dev) {
    if (dev == NULL || !dev->initialized) return ESP_ERR_INVALID_STATE;
    esp_err_t err = write_reg(dev, REG_MODE_CONFIG, MODE_RESET_BIT);
    if (err != ESP_OK) return err;

    for (unsigned attempt = 0u; attempt < 50u; ++attempt) {
        uint8_t mode = 0u;
        vTaskDelay(pdMS_TO_TICKS(2));
        err = read_reg(dev, REG_MODE_CONFIG, &mode);
        if (err != ESP_OK) return err;
        if ((mode & MODE_RESET_BIT) == 0u) return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

esp_err_t max3010x_flush_fifo(max3010x_t *dev) {
    if (dev == NULL || !dev->initialized) return ESP_ERR_INVALID_STATE;
    esp_err_t err = write_reg(dev, REG_FIFO_WR_PTR, 0u);
    if (err != ESP_OK) return err;
    err = write_reg(dev, REG_OVF_COUNTER, 0u);
    if (err != ESP_OK) return err;
    return write_reg(dev, REG_FIFO_RD_PTR, 0u);
}

esp_err_t max3010x_configure(max3010x_t *dev, const max3010x_config_t *config) {
    if (dev == NULL || config == NULL || !dev->initialized || !dev->identified) return ESP_ERR_INVALID_STATE;

    uint8_t rate_bits = 0u;
    esp_err_t err = encode_sample_rate(config->sample_rate_hz, &rate_bits);
    if (err != ESP_OK) return err;

    /* ADC 4096 nA (01), sample rate configurável, pulso 411 us / 18 bits (11). */
    uint8_t spo2_config = (1u << 5) | rate_bits | 0x03u;
    /* Sem média de FIFO; rollover só se explicitamente solicitado; A_FULL=15. */
    uint8_t fifo_config = (config->fifo_rollover ? 0x10u : 0x00u) | 0x0Fu;

    err = write_reg(dev, REG_INTR_ENABLE_1, 0u);
    if (err != ESP_OK) return err;
    err = write_reg(dev, REG_INTR_ENABLE_2, 0u);
    if (err != ESP_OK) return err;
    err = max3010x_flush_fifo(dev);
    if (err != ESP_OK) return err;
    err = write_verify(dev, REG_FIFO_CONFIG, fifo_config, 0x7Fu);
    if (err != ESP_OK) return err;
    err = write_verify(dev, REG_SPO2_CONFIG, spo2_config, 0x7Fu);
    if (err != ESP_OK) return err;
    err = write_verify(dev, REG_LED1_PA, config->led_red_pa, 0xFFu);
    if (err != ESP_OK) return err;
    err = write_verify(dev, REG_LED2_PA, config->led_ir_pa, 0xFFu);
    if (err != ESP_OK) return err;
    err = write_verify(dev, REG_MODE_CONFIG, MODE_SPO2_EN, 0x07u);
    if (err != ESP_OK) return err;

    uint8_t ignored[2] = {0};
    (void)i2c_bus_read(dev->i2c_addr, REG_INTR_STATUS_1, ignored, sizeof(ignored));
    dev->sample_rate_hz = config->sample_rate_hz;
    return ESP_OK;
}

esp_err_t max3010x_config_default(max3010x_t *dev) {
    const max3010x_config_t config = {
        .sample_rate_hz = 100u,
        .led_red_pa = 0x24u,
        .led_ir_pa = 0x24u,
        .fifo_rollover = false
    };
    return max3010x_configure(dev, &config);
}

esp_err_t max3010x_get_fifo_status(max3010x_t *dev, max3010x_fifo_status_t *status) {
    if (dev == NULL || status == NULL || !dev->initialized) return ESP_ERR_INVALID_STATE;
    uint8_t raw[3] = {0};
    esp_err_t err = i2c_bus_read(dev->i2c_addr, REG_FIFO_WR_PTR, raw, sizeof(raw));
    if (err != ESP_OK) return err;

    status->write_pointer = raw[0] & FIFO_POINTER_MASK;
    status->overflow_counter = raw[1] & FIFO_POINTER_MASK;
    status->read_pointer = raw[2] & FIFO_POINTER_MASK;
    status->available_samples = (uint8_t)((status->write_pointer - status->read_pointer) & FIFO_POINTER_MASK);
    return ESP_OK;
}

esp_err_t max3010x_read_fifo(max3010x_t *dev, ppg_sample_t *samples, size_t capacity, size_t *out_count) {
    if (out_count != NULL) *out_count = 0u;
    if (dev == NULL || samples == NULL || capacity == 0u || out_count == NULL || !dev->initialized) {
        return ESP_ERR_INVALID_ARG;
    }

    max3010x_fifo_status_t status = {0};
    esp_err_t err = max3010x_get_fifo_status(dev, &status);
    if (err != ESP_OK) return err;

    if (status.overflow_counter > 0u) {
        dev->fifo_overflow_events += status.overflow_counter;
        (void)max3010x_flush_fifo(dev);
        return ESP_ERR_INVALID_STATE;
    }
    if (status.available_samples == 0u) return ESP_OK;

    size_t count = status.available_samples;
    if (count > capacity) count = capacity;
    if (count > FIFO_DEPTH) count = FIFO_DEPTH;

    uint8_t raw[FIFO_DEPTH * PPG_SAMPLE_BYTES];
    err = i2c_bus_read(dev->i2c_addr, REG_FIFO_DATA, raw, count * PPG_SAMPLE_BYTES);
    if (err != ESP_OK) return err;

    for (size_t i = 0u; i < count; ++i) {
        size_t offset = i * PPG_SAMPLE_BYTES;
        samples[i].red = ((((uint32_t)raw[offset]) << 16) |
                          (((uint32_t)raw[offset + 1u]) << 8) |
                          (uint32_t)raw[offset + 2u]) & ADC_18BIT_MASK;
        samples[i].ir = ((((uint32_t)raw[offset + 3u]) << 16) |
                         (((uint32_t)raw[offset + 4u]) << 8) |
                         (uint32_t)raw[offset + 5u]) & ADC_18BIT_MASK;
    }
    *out_count = count;
    return ESP_OK;
}

esp_err_t max3010x_read_sample(max3010x_t *dev, ppg_sample_t *sample) {
    size_t count = 0u;
    esp_err_t err = max3010x_read_fifo(dev, sample, 1u, &count);
    if (err != ESP_OK) return err;
    return count == 1u ? ESP_OK : ESP_ERR_NOT_FOUND;
}
