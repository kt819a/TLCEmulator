#ifndef DISPLAYAPP_H
#define	DISPLAYAPP_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"   
#include "esp_err.h"
#include "esp_check.h"
#include "boarddef.h"
#include <rom/ets_sys.h>
#include "emulator_app.h"

typedef struct {
        uint8_t pin;
        uint8_t SDAstate;
} DISPLAYINTERFACEPINSTATE_t;

void displayAppInit(void);

#endif	/* DISPLAYAPP_H */ 