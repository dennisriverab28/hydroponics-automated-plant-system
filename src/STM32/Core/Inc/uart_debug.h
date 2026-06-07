#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stddef.h>


void Debug_Print(const char *msg);


void Debug_Println(const char *msg);



void Debug_PrintFloat(const char *label, float value);

void Debug_PrintInt(int32_t v);

#endif
