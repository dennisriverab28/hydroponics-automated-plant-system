#include "rtos.h"
#include "stm32l4xx_hal.h"



static volatile uint32_t g_rtosTickMs = 0;


static RTOS_Task_t g_tasks[RTOS_MAX_TASKS];


void RTOS_Init(void)
{
    for (int i = 0; i < RTOS_MAX_TASKS; i++)
    {
        g_tasks[i].func       = NULL;
        g_tasks[i].period_ms  = 0;
        g_tasks[i].lastRun_ms = 0;
    }
    g_rtosTickMs = 0;
}


void RTOS_AddTask(RTOS_TaskFunc_t func, uint32_t period_ms)
{
    if (func == NULL || period_ms == 0)
        return;

    for (int i = 0; i < RTOS_MAX_TASKS; i++)
    {
        if (g_tasks[i].func == NULL)
        {
            g_tasks[i].func       = func; // Reference to task implementation in system_state.c
            g_tasks[i].active     = 1; // Always active
            g_tasks[i].period_ms  = period_ms;
            g_tasks[i].lastRun_ms = 0;
            break;
        }
    }
}


void RTOS_Tick(void)
{
    g_rtosTickMs++;
}


// Main RTOS implementation, loops through all tasks, continuously checking against elapsed time
void RTOS_Run(void)
{
    uint32_t now = g_rtosTickMs;

    for (int i = 0; i < RTOS_MAX_TASKS; i++)
    {
        if (g_tasks[i].func != NULL && g_tasks[i].active)
        {
            uint32_t elapsed = now - g_tasks[i].lastRun_ms;

            if (elapsed >= g_tasks[i].period_ms)
            {
                g_tasks[i].lastRun_ms = now;
                g_tasks[i].func();
            }
        }
    }


    __NOP();
}
