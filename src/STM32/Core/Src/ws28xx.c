#include "ws28xx.h"
#include <stdio.h>
#include "uart_debug.h"


static void WS28XX_FillPWMBuffer(WS28XX_HandleTypeDef *h)
{
    uint16_t idx = 0;

    for (uint16_t led = 0; led < h->num_leds; led++) {
        uint8_t g = h->color[led * 3 + 0];
        uint8_t r = h->color[led * 3 + 1];
        uint8_t b = h->color[led * 3 + 2];

        uint8_t grb[3] = { g, r, b };

        for (int c = 0; c < 3; c++) {
            for (int bit = 7; bit >= 0; bit--) {
                if (grb[c] & (1 << bit)) {
                    h->pwm[idx++] = WS_T1H;
                } else {
                    h->pwm[idx++] = WS_T0H;
                }
            }
        }
    }


    for (uint16_t i = 0; i < WS2812_RESET_SLOTS; i++) {
        h->pwm[i + h->num_leds * WS2812_BITS_PER_LED] = 0;
    }
}


void WS28XX_Init(WS28XX_HandleTypeDef *h,
                 TIM_HandleTypeDef *htim,
                 uint32_t tim_channel,
                 uint16_t led_count)
{
    if (led_count > WS28XX_MAX_LEDS) {
        led_count = WS28XX_MAX_LEDS;
    }

    h->htim        = htim;
    h->tim_channel = tim_channel;
    h->num_leds    = led_count;
    h->busy        = false;

    for (uint16_t i = 0; i < (uint16_t)(led_count * 3); i++) {
        h->color[i] = 0;
    }

    WS28XX_FillPWMBuffer(h);
}


void WS28XX_SetPixel_RGB(WS28XX_HandleTypeDef *h,
                         uint16_t led,
                         uint8_t r,
                         uint8_t g,
                         uint8_t b)
{
    if (led >= h->num_leds) return;

    h->color[led * 3 + 0] = g;
    h->color[led * 3 + 1] = r;
    h->color[led * 3 + 2] = b;
}


void WS28XX_Update(WS28XX_HandleTypeDef *h)
{
    if (h->busy) return;

    h->busy = true;

    WS28XX_FillPWMBuffer(h);

    uint16_t total_slots =
        (h->num_leds * WS2812_BITS_PER_LED) + WS2812_RESET_SLOTS;


    HAL_TIM_PWM_Start_DMA(
        h->htim,
        h->tim_channel,
        (uint32_t *)h->pwm,
        total_slots
    );
}


void WS28XX_DMA_Complete_Callback(WS28XX_HandleTypeDef *h)
{
    HAL_TIM_PWM_Stop_DMA(h->htim, h->tim_channel);
    h->busy = false;
}
