#ifndef EMUALTORAPP_H
#define	EMUALTORAPP_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"   
#include "esp_err.h"
#include "esp_check.h"
#include "uart_app.h"
#include "player_app.h"

#define ACK_BYTE 0xC5
#define FRAME_START_BYTE 0x3D

typedef enum {
    DECODER_GIVE_ACK = 0x01,
    DECODER_NEED_SEND_ACK
} DecoderEvent_t;

/* HU CMD */
typedef enum
{
    EMULATOR_HU_ACK      = 0xC5,
    EMULATOR_HU_STATUS    = 0x93,
    EMULATOR_HU_INFO     = 0x86,
    EMULATOR_HU_PLAY     = 0x13,
    EMULATOR_HU_NEXT     = 0x17,
    EMULATOR_HU_PREV     = 0x22,
    EMULATOR_HU_FF       = 0x20,
    EMULATOR_HU_FREW     = 0x21,
    EMULATOR_HU_CDnn     = 0x26,
    EMULATOR_HU_RANDOM   = 0x27,
    EMULATOR_HU_STOP     = 0x19,
    EMULATOR_HU_PAUSE    = 0x1C,
    EMULATOR_HU_CHECK    = 0x94,
    EMULATOR_HU_START_BTN= 0x24 
} HUPayloadType_t;

/*CDC CMD*/
typedef enum 
{
    EMULATOR_CDC_ACK            = 0xC5,
    EMULATOR_CDC_BOOTING        = 0x11,
    EMULATOR_CDC_CD_CHECK       = 0x13,
    EMULATOR_CDC_BOOT_OK        = 0x15,
    EMULATOR_CDC_STATUS         = 0x20,
    EMULATOR_CDC_CD_OPERATION   = 0x21,
    EMULATOR_CDC_TRAY_OPERATION = 0x22,
    EMULATOR_CDC_TRAY_EJECTION  = 0x23,
    EMULATOR_CDC_RANDOM_STATUS  = 0x25,
    EMULATOR_CDC_TRAY_STATUS    = 0x26,
    EMULATOR_CDC_TRACK_CHANGE   = 0x27,
    EMULATOR_CDC_CD_SUMMARY     = 0x46,
    EMULATOR_CDC_PLAYING_STATUS = 0x47
} CDCPayloadType_t;

typedef enum {
    DECODER_DO_NOTHING = 0x00,
    DECODER_WAIT_HEADER,
    DECODER_WAIT_FRAME_ID,
    DECODER_WAIT_DATA_LENGTH,
    DECODER_WAIT_PAYLOAD_TYPE,
    DECODER_WAIT_PAYLOAD_DATA,
    DECODER_WAIT_CHECKSUM
} HUMessageDecodingStage_t;

typedef struct {
        uint8_t frameDataLength;
        HUPayloadType_t framePayloadType;
        uint8_t framePayloadData[2];
} HUMessage_t;

typedef enum {
    CD_STATE_NO_CD_LOADED       = 0x01,
    CD_STATE_PAUSED             = 0x03,
    CD_STATE_LOADING_TRACK      = 0x04,
    CD_STATE_PLAYING            = 0x05,
    CD_STATE_CUEING_FWD         = 0x07,
    CD_STATE_REWINDING          = 0x08,
    CD_STATE_CD_READY           = 0x09,
    CD_STATE_SEARCHING_TRACK    = 0x0A
} CDState_t;

typedef enum {
    TRAY_STATE_NO_TRAY = 0x02,
    TRAY_STATE_CD_READY = 0x03,
    TRAY_STATE_LOADING_CD = 0x04,
    TRAY_STATE_UNLOADING_CD = 0x05
} TrayState_t;

typedef union 
{
    struct 
    {
        unsigned NU1:1;
        unsigned NU2:1;
        unsigned cd6:1;
        unsigned cd5:1;
        unsigned cd4:1;
        unsigned cd3:1;
        unsigned cd2:1;
        unsigned cd1:1;
    };
    uint8_t data;
    /* data */
} CDBitmap_t;

typedef struct {
        uint8_t frameDataLength;
        CDCPayloadType_t framePayloadType;
        uint8_t framePayloadData[10];        
} CDCMessage_t;

void emulatorAppInit(void);
void emulatorDecodeHUMsg(uint8_t data);

#endif	/* EMUALTORAPP_H */ 