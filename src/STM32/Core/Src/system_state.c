#include "system_state.h"
#include "moisture_sensor.h"
#include "relay_pump.h"
#include "ina219.h"
#include "grow_led.h"
#include "uart_comms.h"
#include "uart_debug.h"
#include <stdio.h>


SystemMode_t     g_systemMode     = MODE_AUTO;
FailState_t      g_failState      = FAIL_NONE;
MoistureState_t  g_moistState     = MOISTURE_DRY;
uint32_t         g_pumpCooldownMs = 0;


void State_Init(void)
{
    Debug_Print("STATE: Init...\r\n");


    set_grow_purple(255);


    MoistureReading_t m = Moisture_Read();
    g_moistState = m.state;

    char buf[64];
    snprintf(buf, sizeof(buf), "Init Moisture = %s\r\n",
             (g_moistState == MOISTURE_WET ? "WET" : "DRY"));
    Debug_Print(buf);

    Debug_Print("STATE: Init -> AUTO\r\n");
    g_systemMode = MODE_AUTO;
}


void State_Auto(void)
{
    Debug_Print("AUTO: Running logic...\r\n");

    if (g_failState == FAIL_NONE)
    {
        if (g_moistState == MOISTURE_DRY)
        {
            Debug_Print("AUTO: Soil dry → pump for 2s\r\n");
            Pump_Run2s();


            char msg[128];
            snprintf(msg, sizeof(msg),
            		"{\"AutoWateringLog\": true}\n");
            UART_Send(msg);
        }
        else
        {
            Debug_Print("AUTO: Soil wet → no watering\r\n");
        }
    }
    else
    {
        Debug_Print("AUTO: Fail mode active — watering disabled\r\n");
    }
}


void State_Manual(void)
{
    Debug_Print("MODE: Manual active (ESP32 controls)\r\n");
}


void State_UpdateMoisture(void)
{
    MoistureReading_t m = Moisture_Read();
    g_moistState = m.state;

    char dbg[80];
    snprintf(dbg, sizeof(dbg), "MOIST: raw=%lu avg=%lu state=%s\r\n",
             m.raw_adc, m.avg_adc,
             (g_moistState == MOISTURE_WET ? "WET" : "DRY"));
    Debug_Print(dbg);

    char msg[128];
    snprintf(msg, sizeof(msg), "{\"MoistureLevel\": %.2f}\n", m);
    UART_Send(msg);
}


void State_UpdatePower(void)
{
    INA219_Data_t d;
    INA219_Read(&d);

    FailState_t oldFail = g_failState;

    if (d.current_mA > 1500.0f)
        g_failState = FAIL_OVERCURRENT;
    else
        g_failState = FAIL_NONE;

    char dbg[160];
    snprintf(dbg, sizeof(dbg),
             "POWER: V=%.2f I=%.2f P=%.2f fail=%s\r\n",
             d.bus_V, d.current_mA, d.power_mW,
             (g_failState == FAIL_NONE ? "NONE" : "OVERCURRENT"));
    Debug_Print(dbg);

    if (g_failState != oldFail)
    {
        if (g_failState == FAIL_OVERCURRENT)
        {
            Debug_Print("FAIL: Overcurrent detected! (>1.5A)\r\n");
            flash_error_led();
        }
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "{\"GrowLEDPower\": %.2f}\n", d.power_mW);
    UART_Send(msg);
}


SystemMode_t State_GetMode(void)
{
    return g_systemMode;
}

void State_SetMode(SystemMode_t mode)
{
    g_systemMode = mode;

    if (mode == MODE_MANUAL)
        Debug_Print("MODE SWITCH: MANUAL\r\n");
    else
        Debug_Print("MODE SWITCH: AUTO\r\n");

    if (g_failState == FAIL_NONE)
        set_grow_purple(255);
}

SoilState_t State_GetSoilState(void)
{
    return (g_moistState == MOISTURE_WET) ? SOIL_WET : SOIL_DRY;
}

FailState_t State_GetFailState(void)
{
    return g_failState;
}



void State_StateMachineTick(void)
{
    switch (g_systemMode)
    {
        case MODE_AUTO:
            State_Auto();
            break;

        case MODE_MANUAL:
            State_Manual();
            break;

        default:
            break;
    }
}
