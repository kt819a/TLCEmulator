#include "emulator_app.h"

SemaphoreHandle_t xHUAckSemaphore;
SemaphoreHandle_t xUARTSenderMutex;
QueueHandle_t xHUMessageQueue;
QueueHandle_t xCDCMessageQueue;
TimerHandle_t xTimerPlayStatus;
HUMessageDecodingStage_t currentDecodeStage = DECODER_WAIT_HEADER;
uint8_t indexOfDecodedData, crc;
HUMessage_t decodedMessage;

uint8_t frameID = 0;
CDState_t CDState;
TrayState_t trayState;
CDBitmap_t CDBitmap;
uint8_t CDNumber;
uint8_t randomStatus;
uint8_t totalTrackNumber = 99;
uint8_t indexOfCurrentTrack;


int emulatorInitCDCMessage(CDCPayloadType_t type, CDCMessage_t *msg)
{
    switch (type)
    {
    case EMULATOR_CDC_BOOTING:
        msg->frameDataLength = 3;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = 0x60;
        msg->framePayloadData[1] = 0x06;
        return 0;
        break;
case EMULATOR_CDC_CD_CHECK:
        msg->frameDataLength = 3;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = 0x02;
        msg->framePayloadData[1] = 0x86;
        return 0;
        break;    
case EMULATOR_CDC_BOOT_OK:
        msg->frameDataLength = 3;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = 0x00;
        msg->framePayloadData[1] = 0x25;
        return 0;
        break;    
case EMULATOR_CDC_STATUS:
        msg->frameDataLength = 6;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = CDState;
        msg->framePayloadData[1] = trayState;
        msg->framePayloadData[2] = 0x09;
        msg->framePayloadData[3] = 0x05;
        msg->framePayloadData[4] = CDNumber;
        return 0;
        break;    
case EMULATOR_CDC_CD_OPERATION:
        msg->frameDataLength = 2;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = CDState;
        return 0;
        break;    
case EMULATOR_CDC_TRAY_OPERATION:
        msg->frameDataLength = 3;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = trayState;
        msg->framePayloadData[1] = CDNumber;
        return 0;
        break;    
case EMULATOR_CDC_TRAY_EJECTION:
        msg->frameDataLength = 2;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = 0x08;
        return 0;
        break;    
case EMULATOR_CDC_RANDOM_STATUS:
        msg->frameDataLength = 2;
        msg->framePayloadType = type;
        if (randomStatus == 0)
            msg->framePayloadData[0] = 0x03;        //OFF
        else
            msg->framePayloadData[0] = 0x07;        //ON
        return 0;
        break;   
    case EMULATOR_CDC_TRAY_STATUS:
        msg->frameDataLength = 5;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = CDState;
        msg->framePayloadData[1] = CDNumber;
        msg->framePayloadData[2] = CDBitmap.data;
        msg->framePayloadData[3] = CDBitmap.data;
        return 0;
        break;  
    case EMULATOR_CDC_TRACK_CHANGE:                 //todo: update later
        msg->frameDataLength = 4;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = 0x10;
        msg->framePayloadData[1] = 0x01;
        msg->framePayloadData[2] = 0x22;
        return 0;
        break; 
    case EMULATOR_CDC_CD_SUMMARY:                   //todo: update later    
        msg->frameDataLength = 7;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = totalTrackNumber;
        msg->framePayloadData[1] = 0x01;
        msg->framePayloadData[2] = 0x00;
        msg->framePayloadData[3] = 0x43;
        msg->framePayloadData[4] = 0x49;
        msg->framePayloadData[5] = 0x55;
        return 0;
        break; 
        case EMULATOR_CDC_PLAYING_STATUS:                   //todo: update later    
        msg->frameDataLength = 11;
        msg->framePayloadType = type;
        msg->framePayloadData[0] = (indexOfCurrentTrack % 10) + ((indexOfCurrentTrack / 10) << 4);              //BCD format
        msg->framePayloadData[1] = 0x01;
        msg->framePayloadData[2] = 0x01;
        msg->framePayloadData[3] = 0x13;
        msg->framePayloadData[4] = 0x45;
        msg->framePayloadData[5] = 0x67;
        msg->framePayloadData[6] = 0x00;
        msg->framePayloadData[7] = 0x02;
        msg->framePayloadData[8] = 0x45;
        msg->framePayloadData[9] = 0x00;
        return 0;
        break; 
    default:
        return 1;
        break;
    }
}

void convertCDCMessageToDataArray(CDCMessage_t *msg, uint8_t *data, uint8_t *length)
{
    uint8_t crc = 0;

    data[0] = FRAME_START_BYTE;
    crc ^= data[0];
    data[1] = frameID;
    crc ^= data[1];
    data[2] = msg->frameDataLength;
    crc ^= data[2];
    data[3] = msg -> framePayloadType;
    crc ^= data[3];
    for (int i = 0; i < (msg->frameDataLength - 1); i++)
    {
        data[i + 4] = msg -> framePayloadData[i];
        crc ^= data[i+4];
    }
    data[msg->frameDataLength + 3] = crc;
    *length = msg->frameDataLength + 4;
}

static void emulatorProcessHUMessage_task(void *pvParameters)
{
    ESP_LOGI("emulator app", "init process HU message task");

    HUMessage_t HUmsg;
    CDCMessage_t CDCmsg;
    while (xQueueReceive(xHUMessageQueue, &HUmsg, (TickType_t) portMAX_DELAY))
    {
        switch (HUmsg.framePayloadType)
        {
        case EMULATOR_HU_STATUS:
            CDState = CD_STATE_NO_CD_LOADED;
            trayState = TRAY_STATE_CD_READY;
            if(emulatorInitCDCMessage(EMULATOR_CDC_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_RANDOM_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_TRAY_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_INFO:
            CDState = CD_STATE_NO_CD_LOADED;
            trayState = TRAY_STATE_CD_READY;
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_SUMMARY, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_PLAY:
            player_send_cmd(PLAYER_APP_CMD_PLAY);
            CDState = CD_STATE_LOADING_TRACK;
            trayState = TRAY_STATE_CD_READY;
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            CDState = CD_STATE_PLAYING;
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_PLAYING_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_NEXT:
            player_send_cmd(PLAYER_APP_CMD_NEXT);
            if (indexOfCurrentTrack < 99)
                indexOfCurrentTrack ++;
            if(emulatorInitCDCMessage(EMULATOR_CDC_TRACK_CHANGE, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            CDState = CD_STATE_PLAYING;
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_PLAYING_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_PREV:
            player_send_cmd(PLAYER_APP_CMD_PREV);
            if (indexOfCurrentTrack > 1)
                indexOfCurrentTrack --;
            if(emulatorInitCDCMessage(EMULATOR_CDC_TRACK_CHANGE, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            CDState = CD_STATE_PLAYING;
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_PLAYING_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_FF:
            CDState = CD_STATE_PLAYING;
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_PLAYING_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_FREW:
            CDState = CD_STATE_PLAYING;
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_PLAYING_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_CDnn:
            CDNumber = CDCmsg.framePayloadData[0];
            if(emulatorInitCDCMessage(EMULATOR_CDC_TRACK_CHANGE, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            CDState = CD_STATE_PLAYING;
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_SUMMARY, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_PLAYING_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_RANDOM:
            if (CDCmsg.framePayloadData[0] == 0x07)
                randomStatus = 1;
            else
                randomStatus = 0;
            if(emulatorInitCDCMessage(EMULATOR_CDC_RANDOM_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_TRACK_CHANGE, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_PLAYING_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_STOP:
            CDState = CD_STATE_CD_READY;
            player_send_cmd(PLAYER_APP_CMD_STOP);
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_START_BTN:
            CDState = CD_STATE_PLAYING;
            player_send_cmd(PLAYER_APP_CMD_PLAY);
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_PAUSE:
            CDState = CD_STATE_PAUSED;
            player_send_cmd(PLAYER_APP_CMD_PAUSE);
            if(emulatorInitCDCMessage(EMULATOR_CDC_CD_OPERATION, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        case EMULATOR_HU_CHECK:
            CDState = CD_STATE_NO_CD_LOADED;
            trayState = TRAY_STATE_CD_READY;

            if(emulatorInitCDCMessage(EMULATOR_CDC_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_RANDOM_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            if(emulatorInitCDCMessage(EMULATOR_CDC_TRAY_STATUS, &CDCmsg) == 0)
                xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
            break;
        default:
            break;
        }
        //ESP_LOGI("emulator app", "process HU message. Type: %d", HUmsg.framePayloadType);
    }
    vTaskDelete(NULL);
}

static void emulatorSendMessageToHU_task(void *pvParameters)
{
    CDCMessage_t msg;
    uint8_t data[20];
    uint8_t length;
    while (xQueueReceive(xCDCMessageQueue, &msg, (TickType_t) portMAX_DELAY))
    {
        do 
        {
            convertCDCMessageToDataArray(&msg, data, &length);
            xSemaphoreTake(xUARTSenderMutex,(TickType_t)portMAX_DELAY);
            UARTAppSendData(data, length);
            xSemaphoreGive(xUARTSenderMutex);
            //ESP_LOGI("emulator app", "send msg to HU. Type %d", msg.framePayloadType);
        } while (xSemaphoreTake(xHUAckSemaphore, (500 / portTICK_PERIOD_MS)) != pdTRUE);
    }
    vTaskDelete(NULL);
}

void vTimerPlayStatusCallback( TimerHandle_t pxTimer )
{
    CDCMessage_t CDCmsg;
    if(emulatorInitCDCMessage(EMULATOR_CDC_BOOT_OK, &CDCmsg) == 0)
            xQueueSend(xCDCMessageQueue, &CDCmsg, 0);
}

void emulatorAppInit(void)
{
    ESP_LOGI("emulator app", "init emulator app");
    xHUAckSemaphore = xSemaphoreCreateBinary();
    xHUMessageQueue = xQueueCreate(10, sizeof (HUMessage_t));
    xCDCMessageQueue = xQueueCreate(20, sizeof (CDCMessage_t));
    xUARTSenderMutex = xSemaphoreCreateMutex();
    xTaskCreate(emulatorProcessHUMessage_task, "proc HU message", 2048, NULL, 11, NULL);
    xTaskCreate(emulatorSendMessageToHU_task, "emulator send message to HU", 2048, NULL, 10, NULL);

    //init sequence
    CDCMessage_t msg;
    CDBitmap.cd1 = 1;
    CDBitmap.cd2 = 1;
    randomStatus = 0;
    if(emulatorInitCDCMessage(EMULATOR_CDC_BOOTING, &msg) == 0)
    {
        xQueueSend(xCDCMessageQueue, &msg, 0);
    }
    if(emulatorInitCDCMessage(EMULATOR_CDC_BOOT_OK, &msg) == 0)
    {
        xQueueSend(xCDCMessageQueue, &msg, 0);
    }
    CDState = CD_STATE_PLAYING;
    trayState = TRAY_STATE_CD_READY;
    CDNumber = 1;
    indexOfCurrentTrack = 1;
    if(emulatorInitCDCMessage(EMULATOR_CDC_STATUS, &msg) == 0)
    {
        xQueueSend(xCDCMessageQueue, &msg, 0);
    }

    xTimerPlayStatus = xTimerCreate("TimerPlayStatus",          // Name, not used by the kernel.
                                    (500 / portTICK_PERIOD_MS), // The timer period in ticks.
                                    pdTRUE,                     // The timers will auto-reload themselves when they expire.
                                    ( void * ) 1,               // Assign each timer a unique id.
                                    vTimerPlayStatusCallback    // Timer calls when it expires.
                                    );

    xTimerStart(xTimerPlayStatus, 0);
}

void emulatorDecodeHUMsg(uint8_t data)
{
    //ESP_LOGI("emulator app", "byte for decoder: 0x%.2Xh", data);

    if (data == ACK_BYTE) // receive ACK
    {
        frameID++;
        //ESP_LOGI("emulator app", "GET ACK.");
        xSemaphoreGive(xHUAckSemaphore);
        return;
    }

    switch (currentDecodeStage)
    {
    case DECODER_WAIT_HEADER:
        if (data == FRAME_START_BYTE)
        {
            crc = 0;
            indexOfDecodedData = 0;
            crc ^= data;
            currentDecodeStage = DECODER_WAIT_FRAME_ID;
        }
        break;
    case DECODER_WAIT_FRAME_ID: // not used
        crc ^= data;
        currentDecodeStage = DECODER_WAIT_DATA_LENGTH;
        break;
    case DECODER_WAIT_DATA_LENGTH:
        crc ^= data;
        decodedMessage.frameDataLength = data;
        currentDecodeStage = DECODER_WAIT_PAYLOAD_TYPE;
        break;
    case DECODER_WAIT_PAYLOAD_TYPE:
        crc ^= data;
        decodedMessage.framePayloadType = data;
        if (decodedMessage.frameDataLength > 1)
            currentDecodeStage = DECODER_WAIT_PAYLOAD_DATA; // no payload data
        else if (decodedMessage.frameDataLength == 1)
            currentDecodeStage = DECODER_WAIT_CHECKSUM;
        else
            currentDecodeStage = DECODER_WAIT_HEADER;
        break;
    case DECODER_WAIT_PAYLOAD_DATA:
        if (indexOfDecodedData >= 2) // payload can be no more then 2
        {
            currentDecodeStage = DECODER_WAIT_HEADER;
            return;
        }

        crc ^= data;
        decodedMessage.framePayloadData[indexOfDecodedData] = data;
        indexOfDecodedData++;

        if (indexOfDecodedData >= (decodedMessage.frameDataLength - 1))
            currentDecodeStage = DECODER_WAIT_CHECKSUM;
        break;
    case DECODER_WAIT_CHECKSUM:
        crc ^= data;
        if (crc == 0)
        {
            xSemaphoreTake(xUARTSenderMutex,(TickType_t)portMAX_DELAY);
            uint8_t ackData = EMULATOR_CDC_ACK;
            UARTAppSendData(&ackData, 1);
            //ESP_LOGI("emulator app", "Decoded new message.");
            //ESP_LOGI("emulator app", "SEND ACK.");
            xSemaphoreGive(xUARTSenderMutex);
            xQueueSend(xHUMessageQueue, &decodedMessage, ( TickType_t ) 0);
        }
        currentDecodeStage = DECODER_WAIT_HEADER;
        break;
    default:
        currentDecodeStage = DECODER_WAIT_HEADER;
        break;
    }
}

void emulatorDetectCDCMode()
{
    CDState = CD_STATE_PLAYING;
}