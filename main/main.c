#include "main.h"

void app_main(void)
{

    ESP_LOGI("TAG", "Start LOG");
    gpio_reset_pin(LED); 
    gpio_set_direction(LED, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(LED, 0);

    UARTAppInit();

    playerInit();

    bluetoothInit();

    for (;;)
    {
        gpio_set_level(LED, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(LED, 0);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
