#include "uart_comms.h"
#include "relay_pump.h"
#include "grow_led.h"
#include "moisture_sensor.h"
#include "ina219.h"
#include "uart_debug.h"

#include <string.h>
#include <stdio.h>
#include "system_state.h"


extern UART_HandleTypeDef huart1;

static uint8_t rxByte;
static char uart_rx_buffer[128];
static volatile uint8_t uart_rx_index = 0;


static void UART_ClearBuffer(void)
{
    memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
    uart_rx_index = 0;
}


void UART_Init(void)
{
    UART_ClearBuffer();
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);
}


void UART_Send(const char *msg)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}


// Interrupt callback for any received UART message
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (rxByte == '\n')
        {
            uart_rx_buffer[uart_rx_index] = '\0';
        }
        else if (uart_rx_index < sizeof(uart_rx_buffer) - 1)
        {
            uart_rx_buffer[uart_rx_index++] = rxByte;
        }

        HAL_UART_Receive_IT(&huart1, &rxByte, 1);
    }
}

// Classify/parse and process any recieved UART message
void UART_ProcessReceived(void)
{
    if (uart_rx_buffer[0] == '\0')
        return;


    if (strcmp(uart_rx_buffer, "PUMP_ON") == 0)       CMD_PumpOn();
    else if (strcmp(uart_rx_buffer, "PUMP_OFF") == 0) CMD_PumpOff();

    else if (strcmp(uart_rx_buffer, "PUMP_2S") == 0)  {
    	CMD_PumpRun2s();

        char msg[128];
        snprintf(msg, sizeof(msg),
    		"{\"ManualWateringLog\": true}\n");
        UART_Send(msg);
    }

    else if (strcmp(uart_rx_buffer, "LED_PURPLE") == 0)   CMD_LED_Purple();
    else if (strcmp(uart_rx_buffer, "LED_OFF") == 0)      CMD_LED_Off();
    else if (strcmp(uart_rx_buffer, "LED_FLASHRED") == 0) CMD_LED_FlashRed();
    else if (strcmp(uart_rx_buffer, "LED_FULL") == 0)     CMD_LED_Full();
    else if (strncmp(uart_rx_buffer, "LED_DIM", 7) == 0)   CMD_LED_Dim();


    else if (strcmp(uart_rx_buffer, "REQ_MOIST") == 0)    CMD_Request_Moisture();
    else if (strcmp(uart_rx_buffer, "REQ_POWER") == 0)    CMD_Request_Power();

    else if (strcmp(uart_rx_buffer, "MODE_AUTO") == 0) {
        State_SetMode(MODE_AUTO);
    }
    else if (strcmp(uart_rx_buffer, "MODE_MANUAL") == 0) {
        State_SetMode(MODE_MANUAL);
    }

    UART_ClearBuffer();
}


void CMD_PumpOn(void)
{
    Pump_On();
}

void CMD_PumpOff(void)
{
    Pump_Off();
}

void CMD_PumpRun2s(void)
{
    Pump_Run2s();
}


void CMD_LED_Purple(void)
{
    set_grow_purple(255);
}

void CMD_LED_Off(void)
{
    for (uint16_t i = 0; i < WS_LED_COUNT; i++)
        SetPixel_BGR(i, 0, 0, 0);

    WS28XX_Update(&ws_led);
}

void CMD_LED_FlashRed(void)
{
    flash_error_led();
}

void CMD_LED_Full(void)
{
    grow_full_brightness();
}

void CMD_LED_Dim(void)
{
    int value = 0;
    sscanf(uart_rx_buffer, "LED_DIM %d", &value);

    if (value < 0) value = 0;
    if (value > 255) value = 255;

    grow_set_brightness(value);
}


void CMD_Request_Moisture(void)
{
    MoistureReading_t m = Moisture_Read();

    char msg[128];
    snprintf(msg, sizeof(msg), "{\"MoistureLevel\": %.2f}\n", m);
    UART_Send(msg);
}


void CMD_Request_Power(void)
{
    INA219_Data_t d;
    INA219_Read(&d);

    char msg[128];
    snprintf(msg, sizeof(msg), "{\"GrowLEDPower\": %.2f}\n", d);
    UART_Send(msg);
}

void UART_SendMoisturePercent(float percent)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "{\"MoisturePercent\": %.2f}\n", percent);
    UART_Send(msg);
}

