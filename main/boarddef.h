#ifndef BOARDDEF_H
#define	BOARDDEF_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"

/* GPIO */

#define LED GPIO_NUM_2

/* UART */

#define CDC_UART_NUM UART_NUM_2
#define CDC_UART_PIN_RX GPIO_NUM_16
#define CDC_UART_PIN_TX GPIO_NUM_17

/* I2S */

#define I2S_NUM I2S_NUM_0
#define SPDIF_PIN GPIO_NUM_12

#endif	/* BOARDDEF_H */ 