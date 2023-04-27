#include "player_app.h"

#define TAG "Player"

#define I2S_RINGBUF_SIZE (16 * 256)
#define AUDIO_SAMPLE_SIZE (16 * 2 / 8) // 16bit, 2ch, 8bit/byte 

static RingbufHandle_t s_ringbuf_i2s = NULL;

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

    spdif_init(44100);

    xTaskCreate(i2sSend_task, "I2SSend", 2048, NULL, configMAX_PRIORITIES - 3, NULL);
}

void write_ringbuf(const uint8_t *data, size_t size)
{
    // rate control
    UBaseType_t items;
    vRingbufferGetInfo(s_ringbuf_i2s, NULL, NULL, NULL, NULL, &items);
    if (items < I2S_RINGBUF_SIZE * 3 / 8) {
        xRingbufferSend(s_ringbuf_i2s, (void *)data, AUDIO_SAMPLE_SIZE, (TickType_t)portMAX_DELAY);
    } else if (items > I2S_RINGBUF_SIZE * 5 / 8) {
        size -= AUDIO_SAMPLE_SIZE;
    }

    xRingbufferSend(s_ringbuf_i2s, (void *)data, size, (TickType_t)portMAX_DELAY);
}

void player_send_cmd(uint8_t cmd)
{
    bt_avrc_send_cmd(cmd);
}