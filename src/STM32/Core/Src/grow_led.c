#include "grow_led.h"
#include "uart_debug.h"
#include <stdio.h>
#include "ina219.h"

WS28XX_HandleTypeDef ws_led;
uint8_t WS_LED_BRIGHTNESS = 50;


extern TIM_HandleTypeDef htim3;


void SetPixel_BGR(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    WS28XX_SetPixel_RGB(&ws_led, index, b, g, r);
}


void init_grow_leds()
{
    WS28XX_Init(&ws_led, &htim3, TIM_CHANNEL_3, WS_LED_COUNT);

    char dbg[64];
    sprintf(dbg, "GROW: init leds=%d bright=%d\r\n",
            WS_LED_COUNT, WS_LED_BRIGHTNESS);
    Debug_Print(dbg);

    update_grow_leds(WS_LED_BRIGHTNESS);
}


void update_grow_leds(uint8_t brightness)
{
    WS_LED_BRIGHTNESS = brightness;


    for (uint16_t i = 0; i < WS_LED_COUNT; i++)
    {
        SetPixel_BGR(i, brightness, 0, brightness);
    }
    WS28XX_Update(&ws_led);
}

// Sets to purple with a defined intensity/brightness
void set_grow_purple(uint8_t intensity)
{
    for (uint16_t i = 0; i < WS_LED_COUNT; i++)
    {
        SetPixel_BGR(i, intensity, 0, intensity);
    }

    WS28XX_Update(&ws_led);

    char dbg[64];
    sprintf(dbg, "GROW: PURPLE intensity=%d\r\n", intensity);
    Debug_Print(dbg);
}

// Flashes LED's red 5 times to signify overcurrent or other error
void flash_error_led()
{
    Debug_Print("GROW: flash_error_led START\r\n");

    for (int cycle = 0; cycle < 5; cycle++)
    {
        // RED
        for (uint16_t i = 0; i < WS_LED_COUNT; i++)
            SetPixel_BGR(i, WS_LED_BRIGHTNESS, 0, 0);

        WS28XX_Update(&ws_led);
        HAL_Delay(250);

        // OFF
        for (uint16_t i = 0; i < WS_LED_COUNT; i++)
            SetPixel_BGR(i, 0, 0, 0);

        WS28XX_Update(&ws_led);
        HAL_Delay(200);
    }

    Debug_Print("GROW: flash_error_led DONE\r\n");
    set_grow_purple(255);
}


void grow_set_brightness(uint8_t brightness)
{
    update_grow_leds(brightness);
}


void grow_dim_to_safe()
{
    uint8_t safe_level = 120;
    update_grow_leds(safe_level);

    Debug_Print("GROW: dimmed to safe brightness (120)\r\n");
}


void grow_full_brightness()
{
    update_grow_leds(255);
    Debug_Print("GROW: full brightness (255)\r\n");
}


void GrowLight_CurrentLimit_Callback(float current_mA)
{
    Debug_Print("WARNING: Overcurrent detected! Limiting brightness...\r\n");

    grow_dim_to_safe();
}
