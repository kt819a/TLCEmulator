#ifndef PLAYERAPP_H
#define	PLAYERAPP_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"   
#include "esp_err.h"
#include "esp_check.h"
#include "uart_app.h"
#include "emulator_app.h"
#include "spdif.h"
#include "bt_app.h"

typedef enum {
    PLAYER_APP_CMD_PLAY = 0,
    PLAYER_APP_CMD_PAUSE,
    PLAYER_APP_CMD_STOP,
    PLAYER_APP_CMD_NEXT,
    PLAYER_APP_CMD_PREV
} player_app_cmd_t;

void playerInit(void);
void write_ringbuf(const uint8_t *data, size_t size);
void player_send_cmd(uint8_t cmd);

#endif	/* PLAYERAPP_H */ 