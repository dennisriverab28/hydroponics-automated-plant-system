#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

#include "system_state.h"



void Task_UART(void);
void Task_Moisture(void);
void Task_INA219(void);
void Task_Pump(void);
void Task_LED(void);
void Task_Heartbeat(void);

#endif
