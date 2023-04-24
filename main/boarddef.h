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

/* I2C display*/

#define I2C_NUM 0
#define I2C_DISPLAY_SPEED 10000
#define DISPLAY_SDA GPIO_NUM_33
#define DISPLAY_SCK GPIO_NUM_25
#define DISPLAY_MRQ GPIO_NUM_32
#define DISPLAY_ADDRESS 0x23

#endif	/* BOARDDEF_H */ 