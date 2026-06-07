#ifndef RTOS_H
#define RTOS_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define RTOS_MAX_TASKS   10


typedef void (*RTOS_TaskFunc_t)(void);


typedef struct
{
    RTOS_TaskFunc_t func;
    uint8_t         active;
    uint32_t        period_ms;
    uint32_t        lastRun_ms;
} RTOS_Task_t;


void RTOS_Init(void);
void RTOS_AddTask(RTOS_TaskFunc_t func, uint32_t period_ms);
void RTOS_Run(void);
void RTOS_Tick(void);

#ifdef __cplusplus
}
#endif

#endif
