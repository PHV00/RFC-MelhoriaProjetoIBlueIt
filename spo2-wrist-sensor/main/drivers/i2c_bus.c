// I²C significa Inter-Integrated Circuit. É um protocolo de comunicação usado para fazer componentes eletrônicos conversarem entre si.
// Esse protocolo permite que mais sensores sejem conectados ao esp e utilizem as entradas SCL e SDA mas com o SPo2 utilizando a 0x57 e os outros subsequente as outras "portas"

// esse codigo deve : 
/*
1. Inicializar o barramento I²C
2. Escrever dados no sensor
3. Ler dados do sensor
*/
/*
driver do MAX30105
        ↓
i2c_bus.c
        ↓
driver I²C do ESP-IDF
        ↓
hardware do ESP32
*/

#include "drivers/i2c_bus.h"

#define SPO2_I2C_PORT I2C_NUM_0
#define SPO2_I2C_TIMEOUT_MS 1000

static bool g_i2c_initialized = false;//variavel g_ = global = todo o software tem acesso  , mas n podem alterar seu valor

esp_err_t i2c_bus_init(int sda_gpio, int scl_gpio, uint32_t freq_hz) {
    if (g_i2c_initialized) {
        return ESP_OK;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = freq_hz
    };

    esp_err_t err = i2c_param_config(SPO2_I2C_PORT, &conf);
    if (err != ESP_OK) {
        return err;
    }

    err = i2c_driver_install(SPO2_I2C_PORT, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        return err;
    }

    g_i2c_initialized = true;
    return ESP_OK;
}

esp_err_t i2c_bus_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);

    if (data != NULL && len > 0) {
        i2c_master_write(cmd, (uint8_t *)data, len, true);
    }

    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(SPO2_I2C_PORT, cmd, pdMS_TO_TICKS(SPO2_I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    return err;
}

esp_err_t i2c_bus_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len) {
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, &data[len - 1], I2C_MASTER_NACK);

    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(SPO2_I2C_PORT, cmd, pdMS_TO_TICKS(SPO2_I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);

    return err;
}
