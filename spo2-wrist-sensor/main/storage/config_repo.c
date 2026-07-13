#include "storage/config_repo.h"

// Configuração global e constante usada pelo sistema.
static const system_config_t g_config = {
    .sensor_i2c_addr = 0x57,
    .i2c_sda_gpio = 4,
    .i2c_scl_gpio = 5,
    .i2c_freq_hz = 400000,
    .sample_interval_ms = 50
};

// Retorna um ponteiro somente para leitura para a configuração do sistema.
const system_config_t *config_repo_get(void) {
    return &g_config;
}