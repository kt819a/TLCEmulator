#include "display_app.h"

SemaphoreHandle_t xDisplayUpdateSemaphore;
SemaphoreHandle_t xDisplayDisableUpdateMutex;
QueueHandle_t xDisplayGpioInterputQueue;
int i2c_master_port = 0;
char  defaultText[] = "BLUETOOTH";
char displayText[200];

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    DISPLAYINTERFACEPINSTATE_t pinState;
    pinState.pin = (int)args;
    pinState.SDAstate = gpio_get_level(DISPLAY_SDA);
    xQueueSendFromISR(xDisplayGpioInterputQueue, &pinState, NULL);
}



void enableDisplayGPIOInterrupt()
{
    gpio_set_intr_type(DISPLAY_MRQ, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(DISPLAY_SCK, GPIO_INTR_POSEDGE);
}

void disableDisplayGPIOInterrupt()
{
    gpio_set_intr_type(DISPLAY_MRQ, GPIO_INTR_DISABLE);
    gpio_set_intr_type(DISPLAY_SCK, GPIO_INTR_DISABLE);
}

void initDisplayGPIO()
{
    gpio_reset_pin(GPIO_NUM_35);
    gpio_set_direction(DISPLAY_MRQ, GPIO_MODE_OUTPUT);

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

    gpio_isr_handler_add(DISPLAY_MRQ, gpio_interrupt_handler, (void *)DISPLAY_MRQ);
    gpio_isr_handler_add(DISPLAY_SCK, gpio_interrupt_handler, (void *)DISPLAY_SCK);
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
        while(gpio_get_level(DISPLAY_SCK));
        if (byte & 0x80)
        {
            ets_delay_us(10);
            gpio_set_level(DISPLAY_SDA, 1);
            while(!gpio_get_level(DISPLAY_SDA));
        }
        else
        {
            gpio_set_level(DISPLAY_SDA, 0);
        }
        byte = byte << 1;
        ets_delay_us(50);
        gpio_set_level(DISPLAY_SCK, 1);
        while(!gpio_get_level(DISPLAY_SCK));
        ets_delay_us(50);
        gpio_set_level(DISPLAY_SCK, 0);
    }
    while(gpio_get_level(DISPLAY_SCK));
    gpio_set_level(DISPLAY_SDA, 1);     //ack
    ets_delay_us(50);
    gpio_set_level(DISPLAY_SCK, 1);
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
    uint8_t currentPosition = 0;
    strcpy(displayText,defaultText);

    if (strlen(displayText) > 8)
    {
        uint8_t len = strlen(displayText);
        displayText[len] = 0x20;
        displayText[len + 1] = 0x20;
        displayText[len + 2] = 0x20;
        displayText[len + 3] = 0;
    }

    for(;;)
    {
        xSemaphoreTake(xDisplayUpdateSemaphore, 400 / portTICK_PERIOD_MS);
        xSemaphoreTake(xDisplayDisableUpdateMutex, (TickType_t) portMAX_DELAY);
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

        xSemaphoreGive(xDisplayDisableUpdateMutex);
        enableDisplayGPIOInterrupt();
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
                case (DISPLAY_MRQ) :
                {//end data transfer
                    {
                        //ESP_LOGI("Data:", "Byte count = %d(%d), data %02x %c %c ", byteCount, currentPack[1], currentPack[0],currentPack[6],currentPack[7]);
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
                case (DISPLAY_SCK):
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

    xSemaphoreGive(xDisplayDisableUpdateMutex);

    xTaskCreatePinnedToCore(i2cSnifferTask, "snifferTask", 2048, NULL, configMAX_PRIORITIES, NULL, 1);
    xTaskCreatePinnedToCore(i2cSenderTask, "senderTask", 2048, NULL, configMAX_PRIORITIES - 6, NULL, 1);
}