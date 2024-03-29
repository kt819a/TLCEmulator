// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __BT_APP_AV_H__
#define __BT_APP_AV_H__

#include <stdint.h>
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "spdif.h"
#include "player_app.h"

#define BT_AV_TAG               "BT_AV"
#define MAX_AUTCONNECT_COUNT 6000

/**
 * @brief     callback function for A2DP sink
 */
void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);

/**
 * @brief     callback function for A2DP sink audio data stream
 */
void bt_app_a2d_data_cb(const uint8_t *data, uint32_t len);

/**
 * @brief     callback function for AVRCP controller
 */
void bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);

void bt_av_new_track();

bool has_last_connection();
void get_last_connection();
void set_last_connection(esp_bd_addr_t bda);

void bt_av_init(void);
void bt_avrc_play();
void bt_avrc_stop();
void bt_avrc_pause();
void bt_avrc_next();
void bt_avrc_prev();

#endif /* __BT_APP_AV_H__*/
