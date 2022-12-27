#include "main.h"

void app_main(void)
{

    ESP_LOGI("TAG", "Start LOG");
    gpio_reset_pin(13); 
    gpio_set_direction(13, GPIO_MODE_INPUT_OUTPUT);
    gpio_reset_pin(12); 
    gpio_set_direction(12, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(12, 0);
    gpio_set_level(13, 0);

    gpio_set_direction(9, GPIO_MODE_INPUT);

    UARTAppInit();

    playerInit();

    for (;;)
    {
        gpio_set_level(13, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(13, 0);

        if (gpio_get_level(9) == 0)
        {
            UARTAppSendTestData();
            gpio_set_level(12,1);
            gpio_set_level(12,0);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
