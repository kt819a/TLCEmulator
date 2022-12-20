#ifndef UARTAPP_H
#define	UARTAPP_H

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

void UARTAppInit(void);
void UARTAppSendTestData(void);
void UARTAppSendData(uint8_t *data, uint8_t length);

#endif	/* UARTAPP_H */ 
