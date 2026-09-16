#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app/app_controller.h"

void app_main(void) {
    app_controller_init();

    while (1) {
        app_controller_step();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}