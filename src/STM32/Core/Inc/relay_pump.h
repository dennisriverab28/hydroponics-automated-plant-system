#ifndef RELAY_PUMP_H
#define RELAY_PUMP_H

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define PUMP_RELAY_GPIO_PORT   GPIOA
#define PUMP_RELAY_PIN         GPIO_PIN_8


void Pump_Init(void);
void Pump_On(void);
void Pump_Off(void);
void Pump_Run5s(void);

#endif
