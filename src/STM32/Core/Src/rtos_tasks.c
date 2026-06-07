#include "rtos_tasks.h"
#include "uart_comms.h"
#include "uart_debug.h"
#include "system_state.h"
#include "relay_pump.h"
#include "moisture_model.h"
#include <stdio.h>
#include "moisture_sensor.h"

void Task_UART(void)
{
    UART_ProcessReceived();
}


void Task_Moisture(void)
{
    static uint8_t counter = 0;
    counter++;

    MoistureReading_t r = Moisture_Read();

    MoistureModel_AddState(r.state == MOISTURE_WET);

    float percent = MoistureModel_GetPercent();

    if (counter >= 10)
    {
        Debug_Print("MOIST: raw=");
        Debug_PrintInt(r.raw_adc);

        Debug_Print(" state=");
        Debug_Print(r.state == MOISTURE_WET ? "WET" : "DRY");

        Debug_Print(" P=");
        Debug_PrintFloat("", percent);
        Debug_Print("%\r\n");

        UART_SendMoisturePercent(percent);

        counter = 0;
    }

    State_UpdateMoisture();
}



void Task_INA219(void)
{
    State_UpdatePower();
}


void Task_Pump(void)
{
    if (State_GetMode() != MODE_MANUAL)
    {
        State_StateMachineTick();
    }

}


void Task_LED(void)
{
    // NOP
}


void Task_Heartbeat(void)
{
    Debug_Print("RTOS: Heartbeat OK\r\n");
}
