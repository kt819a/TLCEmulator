#include "uart_app.h"

static const char *TAG = "uart_events";

#define PATTERN_CHR_NUM    (1)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (200)
#define RD_BUF_SIZE (BUF_SIZE)
static QueueHandle_t uart0_queue;

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, RD_BUF_SIZE);
            switch(event.type) {
                case UART_DATA:
                    uart_read_bytes(CDC_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    for (int i = 0; i < event.size; i++)
                    {
                        emulatorDecodeHUMsg(dtmp[i]);
                    }
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    uart_flush_input(CDC_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    uart_flush_input(CDC_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void UARTAppInit(void)
{
        uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_EVEN,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };

    ESP_ERROR_CHECK(uart_param_config(CDC_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CDC_UART_NUM, CDC_UART_PIN_TX, CDC_UART_PIN_RX, GPIO_NUM_NC, GPIO_NUM_NC));

    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(CDC_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart0_queue, 0));

    //Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(CDC_UART_NUM, 20);

    //Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);

    emulatorAppInit();
}

void UARTAppSendTestData(void)
{
    char data[] = {ACK_BYTE, FRAME_START_BYTE, 0, 3, EMULATOR_CDC_BOOTING, 0x60, 0x06, 0x49};
    uart_write_bytes(CDC_UART_NUM, (const char*)&data, 8);
}

void UARTAppSendData(uint8_t *data, uint8_t length)
{
    uart_write_bytes(CDC_UART_NUM, data, length);
}