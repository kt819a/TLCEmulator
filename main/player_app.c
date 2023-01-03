#include "player_app.h"

#define TAG "Player"

#define I2S_RINGBUF_SIZE (16 * 1024)

static RingbufHandle_t s_ringbuf_i2s = NULL;

static void i2sTestGenerator_task(void *arg)
{
    uint16_t testData[384*2];

    for (int i = 0; i < 384; i++)
    {
        testData[i*2] = i * 170;
        testData[i*2 + 1] = i * 170;
    }

    for (;;)
    {
        UBaseType_t items;
        vRingbufferGetInfo(s_ringbuf_i2s, NULL, NULL, NULL, NULL, &items);
        if (I2S_RINGBUF_SIZE - items < 384)
        {
            vTaskDelay(50 / portTICK_PERIOD_MS);
        } else
        {
            xRingbufferSend(s_ringbuf_i2s, (void *)testData, 384 * 4, (TickType_t) portMAX_DELAY);
        }
    }
}

static void i2sSend_task(void *arg)
{
    uint8_t *data = NULL;
    size_t item_size = 0;

    for (;;) {
        data = (uint8_t *)xRingbufferReceive(s_ringbuf_i2s, &item_size, (TickType_t) portMAX_DELAY);
        if (item_size != 0){
            spdif_write(data, item_size);
            vRingbufferReturnItem(s_ringbuf_i2s,(void *)data);
        }
    }
}

void playerInit(void)
{
    s_ringbuf_i2s = xRingbufferCreate(I2S_RINGBUF_SIZE, RINGBUF_TYPE_BYTEBUF);
    if(s_ringbuf_i2s == NULL){
        ESP_LOGI(TAG, "ringbuf init ERROR!!!!!!!");
        return;
    }

    spdif_init(48000);

    xTaskCreate(i2sSend_task, "I2SSend", 2048, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(i2sTestGenerator_task, "I2STest", 4096, NULL, configMAX_PRIORITIES - 2, NULL);
}