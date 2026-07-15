#include "drivers/i2c_bus.h"

#include <stdbool.h>

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define SPO2_I2C_PORT I2C_NUM_0
#define SPO2_I2C_TIMEOUT_MS 50u

static bool s_initialized = false;
static SemaphoreHandle_t s_mutex = NULL;

static bool valid_address(uint8_t address) {
    return address > 0u && address < 0x80u;
}

esp_err_t i2c_bus_init(int sda_gpio, int scl_gpio, uint32_t freq_hz) {
    if (s_initialized) return ESP_OK;
    if (sda_gpio < 0 || scl_gpio < 0 || freq_hz == 0u) return ESP_ERR_INVALID_ARG;

    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = freq_hz,
        .clk_flags = 0
    };

    esp_err_t err = i2c_param_config(SPO2_I2C_PORT, &config);
    if (err != ESP_OK) return err;
    err = i2c_driver_install(SPO2_I2C_PORT, config.mode, 0, 0, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

    s_mutex = xSemaphoreCreateMutex();
    if (s_mutex == NULL) return ESP_ERR_NO_MEM;
    s_initialized = true;
    return ESP_OK;
}

esp_err_t i2c_bus_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len) {
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    if (!valid_address(dev_addr) || (len > 0u && data == NULL)) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(SPO2_I2C_TIMEOUT_MS)) != pdTRUE) return ESP_ERR_TIMEOUT;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = i2c_master_start(cmd);
    if (err == ESP_OK) err = i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    if (err == ESP_OK) err = i2c_master_write_byte(cmd, reg_addr, true);
    if (err == ESP_OK && len > 0u) err = i2c_master_write(cmd, data, len, true);
    if (err == ESP_OK) err = i2c_master_stop(cmd);
    if (err == ESP_OK) err = i2c_master_cmd_begin(SPO2_I2C_PORT, cmd, pdMS_TO_TICKS(SPO2_I2C_TIMEOUT_MS));

    i2c_cmd_link_delete(cmd);
    xSemaphoreGive(s_mutex);
    return err;
}

esp_err_t i2c_bus_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len) {
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    if (!valid_address(dev_addr) || data == NULL || len == 0u) return ESP_ERR_INVALID_ARG;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(SPO2_I2C_TIMEOUT_MS)) != pdTRUE) return ESP_ERR_TIMEOUT;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = i2c_master_start(cmd);
    if (err == ESP_OK) err = i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    if (err == ESP_OK) err = i2c_master_write_byte(cmd, reg_addr, true);
    if (err == ESP_OK) err = i2c_master_start(cmd);
    if (err == ESP_OK) err = i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);
    if (err == ESP_OK && len > 1u) err = i2c_master_read(cmd, data, len - 1u, I2C_MASTER_ACK);
    if (err == ESP_OK) err = i2c_master_read_byte(cmd, data + len - 1u, I2C_MASTER_NACK);
    if (err == ESP_OK) err = i2c_master_stop(cmd);
    if (err == ESP_OK) err = i2c_master_cmd_begin(SPO2_I2C_PORT, cmd, pdMS_TO_TICKS(SPO2_I2C_TIMEOUT_MS));

    i2c_cmd_link_delete(cmd);
    xSemaphoreGive(s_mutex);
    return err;
}
