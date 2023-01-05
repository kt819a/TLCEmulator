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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "bt_app_core.h"
#include "bt_app_av.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

TimerHandle_t xTimerAutoConnect;
uint16_t autoConnectRetryCount;
bool autoConnectIsAvaliable;

esp_bd_addr_t last_connection = {0,0,0,0,0,0};
const char* last_bda_nvs_name = "src_bda";

/* a2dp event handler */
static void bt_av_hdl_a2d_evt(uint16_t event, void *p_param);
/* avrc event handler */
static void bt_av_hdl_avrc_evt(uint16_t event, void *p_param);


static uint32_t m_pkt_cnt = 0;
static esp_a2d_audio_state_t m_audio_state = ESP_A2D_AUDIO_STATE_STOPPED;
/*auto connect timer handle*/
void vTimerAutoConnet( TimerHandle_t pxTimer )
{
    if ( autoConnectRetryCount < MAX_AUTCONNECT_COUNT) {
        esp_a2d_sink_connect(last_connection);
        ESP_LOGI(BT_AV_TAG, "%s start discovery. Wait...... count = %d", __func__, autoConnectRetryCount);
        autoConnectRetryCount++;
    }
}

/* callback for A2DP sink */
void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: 
    case ESP_A2D_AUDIO_STATE_EVT:
    case ESP_A2D_PROF_STATE_EVT: /*{
        ESP_LOGI(BT_AV_TAG, "ESP_A2D_PROF_STATE_EVT. State: %d", (uint8_t) rc->a2d_prof_stat.init_state);
        break;
    }*/
    case ESP_A2D_AUDIO_CFG_EVT: {
        bt_app_work_dispatch(bt_av_hdl_a2d_evt, event, param, sizeof(esp_a2d_cb_param_t), NULL);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "a2dp invalid cb event: %d", event);
        break;
    }
}

void bt_app_a2d_data_cb(const uint8_t *data, uint32_t len)
{
    write_ringbuf(data, len);
}

void bt_app_rc_ct_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{
    switch (event) {
    case ESP_AVRC_CT_CONNECTION_STATE_EVT:
    case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT: {
        bt_app_work_dispatch(bt_av_hdl_avrc_evt, event, param, sizeof(esp_avrc_ct_cb_param_t), NULL);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "avrc invalid cb event: %d", event);
        break;
    }
}

static void bt_av_hdl_a2d_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
    esp_a2d_cb_param_t *a2d = NULL;
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        uint8_t *bda = a2d->conn_stat.remote_bda;
        ESP_LOGI(BT_AV_TAG, "avrc conn_state evt: state %d, feature 0x%x, [%02x:%02x:%02x:%02x:%02x:%02x]",
                           a2d->conn_stat.state, a2d->conn_stat.disc_rsn, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED)
        {
            set_last_connection(a2d->conn_stat.remote_bda);
            xTimerStop(xTimerAutoConnect, 0);
        }
        if ((a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) && (a2d->conn_stat.disc_rsn == ESP_A2D_DISC_RSN_ABNORMAL))
        {
            if (autoConnectIsAvaliable)
                xTimerStart(xTimerAutoConnect, 0);
        }
        break;
    }
    case ESP_A2D_PROF_STATE_EVT: {
        if (autoConnectIsAvaliable)
            xTimerStart(xTimerAutoConnect, 0);
        break;
    }
    case ESP_A2D_AUDIO_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        ESP_LOGI(BT_AV_TAG, "a2dp audio_state_cb state %d", a2d->audio_stat.state);
        m_audio_state = a2d->audio_stat.state;
        if (ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state) {
            m_pkt_cnt = 0;
        }
        break;
    }
    case ESP_A2D_AUDIO_CFG_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        ESP_LOGI(BT_AV_TAG, "a2dp audio_cfg_cb , codec type %d", a2d->audio_cfg.mcc.type);
        // for now only SBC stream is supported
        if (a2d->audio_cfg.mcc.type == ESP_A2D_MCT_SBC) {
            int sample_rate = 16000;
            char oct0 = a2d->audio_cfg.mcc.cie.sbc[0];
            if (oct0 & (0x01 << 6)) {
                sample_rate = 32000;
            } else if (oct0 & (0x01 << 5)) {
                sample_rate = 44100;
            } else if (oct0 & (0x01 << 4)) {
                sample_rate = 48000;
            }
            spdif_set_sample_rates(sample_rate);
            //MediaControl *ctrl = (MediaControl *)local_service->Based.instance;
            //CodecEvent evt;
            
            //ctrl->decoderRequestSetup(ctrl, (void *)&evt);
            //temporarily hardcoded the PCM configuaration
            ESP_LOGI(BT_AV_TAG, "configure audio player %x-%x-%x-%x\n",
                     a2d->audio_cfg.mcc.cie.sbc[0],
                     a2d->audio_cfg.mcc.cie.sbc[1],
                     a2d->audio_cfg.mcc.cie.sbc[2],
                     a2d->audio_cfg.mcc.cie.sbc[3]);
            ESP_LOGI(BT_AV_TAG, "audio player configured, samplerate=%d", sample_rate);
        }
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled evt %d", __func__, event);
        break;
    }
}

static void bt_av_hdl_avrc_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s evt %d", __func__, event);
    esp_avrc_ct_cb_param_t *rc = (esp_avrc_ct_cb_param_t *)(p_param);
    switch (event) {
    case ESP_AVRC_CT_CONNECTION_STATE_EVT: {
        uint8_t *bda = rc->conn_stat.remote_bda;
        ESP_LOGI(BT_AV_TAG, "avrc conn_state evt: state %d, feature 0x%x, [%02x:%02x:%02x:%02x:%02x:%02x]",
                           rc->conn_stat.connected, rc->conn_stat.connected, bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        break;
    }
    case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT: {
        ESP_LOGI(BT_AV_TAG, "avrc passthrough rsp: key_code 0x%x, key_state %d", rc->psth_rsp.key_code, rc->psth_rsp.key_state);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled evt %d", __func__, event);
        break;
    }
}

bool has_last_connection() {  
    esp_bd_addr_t empty_connection = {0,0,0,0,0,0};
    int result = memcmp(last_connection, empty_connection, ESP_BD_ADDR_LEN);
    return result!=0;
}

void get_last_connection(){
    ESP_LOGD(BT_AV_TAG, "%s", __func__);
    nvs_handle my_handle;
    esp_err_t err;
    
    err = nvs_open("connected_bda", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
         ESP_LOGE(BT_AV_TAG,"NVS OPEN ERROR");
    }

    esp_bd_addr_t bda;
    size_t size = sizeof(bda);
    err = nvs_get_blob(my_handle, last_bda_nvs_name, bda, &size);
    if ( err != ESP_OK) { 
        if ( err == ESP_ERR_NVS_NOT_FOUND ) {
            ESP_LOGI(BT_AV_TAG, "nvs_blob does not exist");
        } else {
            ESP_LOGE(BT_AV_TAG, "nvs_get_blob failed");
        }
    }
    nvs_close(my_handle);
    if (err == ESP_OK) {
        memcpy(last_connection,bda,size);
    } 
    //ESP_LOGD(BT_AV_TAG, "=> %s", to_str(last_connection));
}

void set_last_connection(esp_bd_addr_t bda){
    //same value, nothing to store
    ESP_LOGE(BT_AV_TAG, "Save BDA to NVS");
    ESP_LOGD(BT_AV_TAG, "no change!");
    if ( memcmp(bda, last_connection, ESP_BD_ADDR_LEN) == 0 ) {
        ESP_LOGD(BT_AV_TAG, "no change!");
        return; 
    }
    nvs_handle my_handle;
    esp_err_t err;
    
    err = nvs_open("connected_bda", NVS_READWRITE, &my_handle);
    if (err != ESP_OK){
         ESP_LOGE(BT_AV_TAG, "NVS OPEN ERROR");
    }
    err = nvs_set_blob(my_handle, last_bda_nvs_name, bda, ESP_BD_ADDR_LEN);
    if (err == ESP_OK) {
        err = nvs_commit(my_handle);
    } else {
        ESP_LOGE(BT_AV_TAG, "NVS WRITE ERROR");
    }
    if (err != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "NVS COMMIT ERROR");
    }
    nvs_close(my_handle);
    memcpy(last_connection, bda, ESP_BD_ADDR_LEN);
}

void bt_av_init(void)
{
    get_last_connection();
    if (has_last_connection())
        {
            ESP_LOGI(BT_AV_TAG, "get last connections, [%02x:%02x:%02x:%02x:%02x:%02x]",
                    last_connection[0], last_connection[1], last_connection[2], last_connection[3], last_connection[4], last_connection[5]);
            autoConnectIsAvaliable = true;
        } else
        {
            autoConnectIsAvaliable = false;
        }
    xTimerAutoConnect = xTimerCreate("TimerPlayStatus",          // Name, not used by the kernel.
                                    (1000 / portTICK_PERIOD_MS), // The timer period in ticks.
                                    pdTRUE,                     // The timers will auto-reload themselves when they expire.
                                    ( void * ) 1,               // Assign each timer a unique id.
                                    vTimerAutoConnet            // Timer calls when it expires.
                                    );
    
}