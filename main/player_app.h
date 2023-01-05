#ifndef PLAYERAPP_H
#define	PLAYERAPP_H

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
#include "spdif.h"

void playerInit(void);
void write_ringbuf(const uint8_t *data, size_t size);

#endif	/* PLAYERAPP_H */ 