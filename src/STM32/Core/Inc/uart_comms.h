#ifndef UART_COMMS_H
#define UART_COMMS_H

#include "stm32l4xx_hal.h"


void UART_Init(void);


void UART_Send(const char *msg);


void UART_ProcessReceived(void);


void CMD_PumpOn(void);
void CMD_PumpOff(void);
void CMD_PumpRun2s(void);

void CMD_LED_Purple(void);
void CMD_LED_Off(void);
void CMD_LED_FlashRed(void);
void CMD_LED_Full(void);
void CMD_LED_Dim(void);

void CMD_Request_Moisture(void);
void CMD_Request_Power(void);

void UART_SendMoisturePercent(float percent);


#endif
