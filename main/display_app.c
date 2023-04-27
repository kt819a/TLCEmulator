#include "display_app.h"

SemaphoreHandle_t xDisplayTextUpdateMutex;
SemaphoreHandle_t xDisplayUpdateSemaphore;
SemaphoreHandle_t xDisplayDisableUpdateMutex;
QueueHandle_t xDisplayGpioInterputQueue;
int i2c_master_port = 0;
char  defaultText[] = "BLUETOOTH";
char displayText[400];
uint8_t currentPosition = 0;

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    DISPLAYINTERFACEPINSTATE_t pinState;
    pinState.pin = (int)args;
    pinState.SDAstate = gpio_get_level(DISPLAY_SDA_IN);
    xQueueSendFromISR(xDisplayGpioInterputQueue, &pinState, NULL);
}

void transliteration(char *dst, char* src)
{
    uint8_t pos = 0;
    uint16_t symb = 0;
    for (int i = 0; i < strlen(src); i++) {
        symb = 0;
        if ((src[i] & 0b11000000) == 0b11000000)
        {
            symb = ((src[i] & 0b00111111) << 6) | (src[i + 1] & 0b00111111);
            i++;
        } else
            symb = src[i];
        switch (symb) {
            case 1040:
            {
                dst[pos] = 'A';
                pos++;
            } break;
            case 1072:
            {
                dst[pos] = 'A';
                pos++;
            } break;
            case 1041:
            {
                dst[pos] = 'B';
                pos++;
            } break;
            case 1073:
            {
                dst[pos] = 'B';
                pos++;
            } break;
            case 1042:
            {
                dst[pos] = 'V';
                pos++;
            } break;
            case 1074:
            {
                dst[pos] = 'V';
                pos++;
            } break;
            case 1043:
            {
                dst[pos] = 'G';
                pos++;
            } break;
            case 1075:
            {
                dst[pos] = 'G';
                pos++;
            } break;
            case 1044:
            {
                dst[pos] = 'D';
                pos++;
            } break;
            case 1076:
            {
                dst[pos] = 'D';
                pos++;
            } break;
            case 1045:
            {
                dst[pos] = 'E';
                pos++;
            } break;
            case 1077:
            {
                dst[pos] = 'E';
                pos++;
            } break;
            case 1025:
            {
                dst[pos] = 'Y';
                pos++;
                dst[pos] = 'O';
                pos++;
            } break;
            case 1105:
            {
                dst[pos] = 'Y';
                pos++;
                dst[pos] = 'O';
                pos++;
            } break;
            case 1046:
            {
                dst[pos] = 'Z';
                pos++;
                dst[pos] = 'H';
                pos++;
            } break;
            case 1078:
            {
                dst[pos] = 'Z';
                pos++;
                dst[pos] = 'H';
                pos++;
            } break;
            case 1047:
            {
                dst[pos] = 'Z';
                pos++;
            } break;
            case 1079:
            {
                dst[pos] = 'Z';
                pos++;
            } break;
            case 1048:
            {
                dst[pos] = 'I';
                pos++;
            } break;
            case 1080:
            {
                dst[pos] = 'I';
                pos++;
            } break;
            case 1049:
            {
                dst[pos] = 'J';
                pos++;
            } break;
            case 1081:
            {
                dst[pos] = 'J';
                pos++;
            } break;
            case 1050:
            {
                dst[pos] = 'K';
                pos++;
            } break;
            case 1082:
            {
                dst[pos] = 'K';
                pos++;
            } break;
            case 1051:
            {
                dst[pos] = 'L';
                pos++;
            } break;
            case 1083:
            {
                dst[pos] = 'L';
                pos++;
            } break;
            case 1052:
            {
                dst[pos] = 'M';
                pos++;
            } break;
            case 1084:
            {
                dst[pos] = 'M';
                pos++;
            } break;
            case 1053:
            {
                dst[pos] = 'N';
                pos++;
            } break;
            case 1085:
            {
                dst[pos] = 'N';
                pos++;
            } break;
            case 1054:
            {
                dst[pos] = 'O';
                pos++;
            } break;
            case 1086:
            {
                dst[pos] = 'O';
                pos++;
            } break;
            case 1055:
            {
                dst[pos] = 'P';
                pos++;
            } break;
            case 1087:
            {
                dst[pos] = 'P';
                pos++;
            } break;
            case 1056:
            {
                dst[pos] = 'R';
                pos++;
            } break;
            case 1088:
            {
                dst[pos] = 'R';
                pos++;
            } break;
            case 1057:
            {
                dst[pos] = 'S';
                pos++;
            } break;
            case 1089:
            {
                dst[pos] = 'S';
                pos++;
            } break;
            case 1058:
            {
                dst[pos] = 'T';
                pos++;
            } break;
            case 1090:
            {
                dst[pos] = 'T';
                pos++;
            } break;
            case 1059:
            {
                dst[pos] = 'U';
                pos++;
            } break;
            case 1091:
            {
                dst[pos] = 'U';
                pos++;
            } break;
            case 1060:
            {
                dst[pos] = 'F';
                pos++;
            } break;
            case 1092:
            {
                dst[pos] = 'F';
                pos++;
            } break;
            case 1061:
            {
                dst[pos] = 'H';
                pos++;
            } break;
            case 1093:
            {
                dst[pos] = 'H';
                pos++;
            } break;
            case 1062:
            {
                dst[pos] = 'T';
                pos++;
                dst[pos] = 'S';
                pos++;
            } break;
            case 1094:
            {
                dst[pos] = 'T';
                pos++;
                dst[pos] = 'S';
                pos++;
            } break;
            case 1063:
            {
                dst[pos] = 'C';
                pos++;
                dst[pos] = 'H';
                pos++;
            } break;
            case 1095:
            {
                dst[pos] = 'C';
                pos++;
                dst[pos] = 'H';
                pos++;
            } break;
            case 1064:
            {
                dst[pos] = 'S';
                pos++;
                dst[pos] = 'H';
                pos++;
            } break;
            case 1096:
            {
                dst[pos] = 'S';
                pos++;
                dst[pos] = 'H';
                pos++;
            } break;
            case 1065:
            {
                dst[pos] = 'S';
                pos++;
                dst[pos] = 'C';
                pos++;
                dst[pos] = 'H';
                pos++;
            } break;
            case 1097:
            {
                dst[pos] = 'S';
                pos++;
                dst[pos] = 'C';
                pos++;
                dst[pos] = 'H';
                pos++;
            } break;
            case 1066:
            {
            } break;
            case 1098:
            {
            } break;
            case 1067:
            {
                dst[pos] = 'Y';
                pos++;
            } break;
            case 1099:
            {
                dst[pos] = 'Y';
                pos++;
            } break;
            case 1068:
            {
            } break;
            case 1100:
            {
            } break;
            case 1069:
            {
                dst[pos] = 'J';
                pos++;
                dst[pos] = 'E';
                pos++;
            } break;
            case 1101:
            {
                dst[pos] = 'J';
                pos++;
                dst[pos] = 'E';
                pos++;
            } break;
            case 1070:
            {
                dst[pos] = 'J';
                pos++;
                dst[pos] = 'U';
                pos++;
            } break;
            case 1102:
            {
                dst[pos] = 'J';
                pos++;
                dst[pos] = 'U';
                pos++;
            } break;
            case 1071:
            {
                dst[pos] = 'J';
                pos++;
                dst[pos] = 'A';
                pos++;
            } break;
            case 1103:
            {
                dst[pos] = 'J';
                pos++;
                dst[pos] = 'A';
                pos++;
            } break;
            default: 
            {
                dst[pos] = src[i];
                pos++;
            }
        }
    }
    dst[pos] = 0;
}

void updateDisplayText(char *msg)
{
    xSemaphoreTake(xDisplayTextUpdateMutex, 50 / portTICK_PERIOD_MS);
    ESP_LOGI("DISPLAY:", "Change display text: %s", msg);
    transliteration(displayText,msg);
    strcat(displayText, "  ");
    ESP_LOGI("DISPLAY:", "Change display text: %s", displayText);
    currentPosition = 0;
    xSemaphoreGive(xDisplayTextUpdateMutex);
}

void enableDisplayGPIOInterrupt()
{
    gpio_set_intr_type(DISPLAY_MRQ_IN, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(DISPLAY_SCK_IN, GPIO_INTR_POSEDGE);
}

void disableDisplayGPIOInterrupt()
{
    gpio_set_intr_type(DISPLAY_MRQ_IN, GPIO_INTR_DISABLE);
    gpio_set_intr_type(DISPLAY_SCK_IN, GPIO_INTR_DISABLE);
}

void initDisplayGPIO()
{
    gpio_reset_pin(DISPLAY_MRQ_IN);
    gpio_set_direction(DISPLAY_MRQ_IN, GPIO_MODE_INPUT);
    gpio_reset_pin(DISPLAY_SCK_IN);
    gpio_set_direction(DISPLAY_SCK_IN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(DISPLAY_SCK_IN,GPIO_PULLDOWN_ONLY);
    gpio_reset_pin(DISPLAY_SDA_IN);
    gpio_set_direction(DISPLAY_SDA_IN, GPIO_MODE_INPUT);

    dac_output_disable(DAC_CHANNEL_1);
    dac_output_disable(DAC_CHANNEL_2);

    gpio_reset_pin(DISPLAY_MRQ); 
    gpio_set_direction(DISPLAY_MRQ, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(DISPLAY_MRQ, 1);
    gpio_set_pull_mode(DISPLAY_MRQ,GPIO_PULLUP_ONLY);
    gpio_reset_pin(DISPLAY_SCK); 
    gpio_set_direction(DISPLAY_SCK, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(DISPLAY_SCK, 1);
    gpio_set_pull_mode(DISPLAY_SCK,GPIO_PULLUP_ONLY);
    gpio_reset_pin(DISPLAY_SDA); 
    gpio_set_direction(DISPLAY_SDA, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(DISPLAY_SDA, 1);
    gpio_set_pull_mode(DISPLAY_SDA,GPIO_PULLUP_ONLY);

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1  );

    gpio_isr_handler_add(DISPLAY_MRQ_IN, gpio_interrupt_handler, (void *)DISPLAY_MRQ_IN);
    gpio_isr_handler_add(DISPLAY_SCK_IN, gpio_interrupt_handler, (void *)DISPLAY_SCK_IN);
}

void waitPinUP(gpio_num_t pin, uint8_t timeout)
{
    while(timeout && (!gpio_get_level(pin)))
    {
        ets_delay_us(1);
        timeout --;
    }
}

void i2c_soft_start()
{
    gpio_set_level(DISPLAY_SDA, 0);
    ets_delay_us(40);
    gpio_set_level(DISPLAY_SCK, 0);
}

void i2c_soft_stop()
{
    gpio_set_level(DISPLAY_SDA, 0);
    ets_delay_us(50);
    gpio_set_level(DISPLAY_SCK, 1);
    ets_delay_us(50);
    gpio_set_level(DISPLAY_SDA, 1);
}

void i2c_soft_sendByte(uint8_t byte)
{
    for (int i = 0; i < 8; i++)         //data
    {
        if (byte & 0x80)
        {
            ets_delay_us(10);
            gpio_set_level(DISPLAY_SDA, 1);
            waitPinUP(DISPLAY_SDA_IN,2);
        }
        else
        {
            gpio_set_level(DISPLAY_SDA, 0);
        }
        byte = byte << 1;
        ets_delay_us(50);
        gpio_set_level(DISPLAY_SCK, 1);
        waitPinUP(DISPLAY_SCK_IN,200);
        ets_delay_us(50);
        gpio_set_level(DISPLAY_SCK, 0);
    }
    gpio_set_level(DISPLAY_SDA, 1);     //ack
    ets_delay_us(50);
    gpio_set_level(DISPLAY_SCK, 1);
    waitPinUP(DISPLAY_SCK_IN,200);
    ets_delay_us(50);
    gpio_set_level(DISPLAY_SCK, 0);
    ets_delay_us(5);
}

void i2c_sendTextToDiasplay(uint8_t* message)
{   
//    while(!gpio_get_level(DISPLAY_MRQ));
//    gpio_set_level(DISPLAY_MRQ,0);
//    ets_delay_us(100);
//    i2c_soft_start();
//    i2c_soft_sendByte((0x23 << 1));
//    i2c_soft_sendByte((0x01));
//    i2c_soft_sendByte((0x11));
//    i2c_soft_stop();
//    gpio_set_level(DISPLAY_MRQ,1);
//    ets_delay_us(500);
    while(!gpio_get_level(DISPLAY_MRQ));
    gpio_set_level(DISPLAY_MRQ,0);
    ets_delay_us(50);
    i2c_soft_start();
    ets_delay_us(10);
    i2c_soft_sendByte((0x23 << 1));
    i2c_soft_sendByte((0x0F));
    i2c_soft_sendByte((0x90));
    i2c_soft_sendByte((0x7F));
    i2c_soft_sendByte((0x55));
    i2c_soft_sendByte((0xFF));
    i2c_soft_sendByte((0xFF));
    i2c_soft_sendByte((0x40));
    i2c_soft_sendByte((0x01));

    for (int i = 0; i < (8); i++)
    {
        i2c_soft_sendByte(message[i]);
    }

    i2c_soft_stop();
    gpio_set_level(DISPLAY_MRQ,1);
}

static void i2cSenderTask(void *arg)
{
    ESP_LOGI("DISPLAY:", "Init display sender task");
    char msgToSend[8];
    strcpy(displayText,defaultText);
    strcat(displayText,"  ");

    for(;;)
    {
        xSemaphoreTake(xDisplayUpdateSemaphore, 777 / portTICK_PERIOD_MS);
        xSemaphoreTake(xDisplayDisableUpdateMutex, (TickType_t) portMAX_DELAY);
        xSemaphoreTake(xDisplayTextUpdateMutex, 50 / portTICK_PERIOD_MS);
        disableDisplayGPIOInterrupt();

        if (strlen(displayText) > 8)
        {
            if ((currentPosition) > (strlen(displayText) - 1))
                currentPosition = 0;
            for(int i = 0; i < 8; i ++)
            {
                msgToSend[i] = displayText[(i + currentPosition) % (strlen(displayText))];
            }
            currentPosition++;
        } else
        {
            strncpy(msgToSend,displayText,strlen(displayText));
            for (int i = strlen(displayText); i < 8; i++)
                msgToSend[i] = 0x20;
        }

        ESP_LOGI("DISPLAY:", "Sender tick : %s", msgToSend);
        i2c_sendTextToDiasplay((uint8_t *)msgToSend);

        enableDisplayGPIOInterrupt();
        xSemaphoreGive(xDisplayDisableUpdateMutex);
        xSemaphoreGive(xDisplayTextUpdateMutex);
    }
}

static void i2cSnifferTask(void *arg)
{
    uint8_t bitCount = 0;
    uint8_t byteCount = 0;
    uint8_t currentDataByte = 0;
    uint8_t currentPack[30];

    initDisplayGPIO();
    enableDisplayGPIOInterrupt();

    DISPLAYINTERFACEPINSTATE_t pinState;

    xSemaphoreTake(xDisplayDisableUpdateMutex,  (TickType_t) portMAX_DELAY);

    ESP_LOGI("DISPLAY:", "Init display sniffer task");

    for(;;)
    {
        if (xQueueReceive(xDisplayGpioInterputQueue, &pinState,  (TickType_t) portMAX_DELAY))
        {
            switch (pinState.pin) 
            {
                case (DISPLAY_MRQ_IN) :
                {//end data transfer
                    {
                        ESP_LOGI("Data:", "Byte count = %d(%d), data %02x %c %c ", byteCount, currentPack[1], currentPack[0],currentPack[6],currentPack[7]);
                        if (byteCount >= 14) 
                            {
                                ESP_LOGI("Data:", "Byte count = %d(%d), data %02x %c %c ", byteCount, currentPack[1], currentPack[0],currentPack[6],currentPack[7]);
                                for (int i = 0; i < currentPack[1]; i++)
                                    ESP_LOGI("Data:", "Byte[%d] : %c ", i, currentPack[i + 2]);
                                if (((currentPack[9] == 'b') && (currentPack[10] == 'a')) || ((currentPack[9] == 't') && (currentPack[10] == 'r')) || ((currentPack[4] == 'q') && (currentPack[6] == 'T') && (currentPack[7] == 'R'))) 
                                    {
                                        ESP_LOGI("Data:", "Search replaciable message!");
                                        xSemaphoreGive(xDisplayDisableUpdateMutex);
                                        xSemaphoreGive(xDisplayUpdateSemaphore);
                                        emulatorDetectCDCMode();

                                    } else 
                                    {
                                        xSemaphoreTake(xDisplayDisableUpdateMutex,  25 / portTICK_PERIOD_MS);
                                    }
                                
                            } else 
                            { 
                                //ESP_LOGI("Data:", "Byte count = %d, data %02x ", byteCount, currentPack[0]);
                            }

                        bitCount = 0;
                        byteCount = 0;
                        currentDataByte = 0;
                        for (int i = 0; i < 20 ; i++)
                            currentPack[i] = 0;
                    }    
                } break;
                case (DISPLAY_SCK_IN):
                {
                    if (bitCount < 8) //get data bit
                    {
                        currentDataByte = (currentDataByte << 1) | pinState.SDAstate;
                        bitCount++;
                    } else              //get ack bit / end byte transfer
                    {
                        bitCount = 0;
                        currentPack[byteCount] = currentDataByte;
                        byteCount ++;
                        //ESP_LOGI("display app", "get byte %d, byte count = %d", currentDataByte, byteCount);
                    }
                }
            }
        }
    }
    vTaskDelete(NULL);
}

void displayAppInit(void)
{
    xDisplayGpioInterputQueue = xQueueCreate(250,sizeof (DISPLAYINTERFACEPINSTATE_t));

    xDisplayUpdateSemaphore = xSemaphoreCreateBinary();
    xDisplayDisableUpdateMutex = xSemaphoreCreateBinary();
    xDisplayTextUpdateMutex = xSemaphoreCreateBinary();

    xSemaphoreGive(xDisplayDisableUpdateMutex);

    xTaskCreatePinnedToCore(i2cSnifferTask, "snifferTask", 2048, NULL, configMAX_PRIORITIES, NULL, 1);
    xTaskCreatePinnedToCore(i2cSenderTask, "senderTask", 2048, NULL, configMAX_PRIORITIES - 6, NULL, 1);
}