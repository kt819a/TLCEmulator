#ifndef BLUETOOTHAPP_H
#define	BLUETOOTHAPP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "bt_app_core.h"
#include "bt_app_av.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#define BTC_AV_SBC_DEFAULT_SAMP_FREQ A2D_SBC_IE_SAMP_FREQ_48

typedef enum {
    BT_APP_CMD_PLAY = 0,
    BT_APP_CMD_PAUSE,
    BT_APP_CMD_STOP,
    BT_APP_CMD_NEXT,
    BT_APP_CMD_PREV
} bt_app_avrc_cmd_t;

void bluetoothInit(void);

void bt_avrc_send_cmd(uint8_t cmd);

#endif	/* BLUETOOTHAPP_H */ 