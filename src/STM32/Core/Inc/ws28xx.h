#ifndef __WS28XX_H__
#define __WS28XX_H__

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>


#define WS2812_BITS_PER_LED   24

#define WS2812_RESET_SLOTS    60


#define WS28XX_MAX_LEDS       40


#ifndef WS_T0H
#define WS_T0H   28
#endif

#ifndef WS_T1H
#define WS_T1H   56
#endif

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t tim_channel;
    uint16_t num_leds;
    bool busy;


    uint8_t  color[WS28XX_MAX_LEDS * 3];


    uint16_t pwm[WS28XX_MAX_LEDS * WS2812_BITS_PER_LED + WS2812_RESET_SLOTS];
} WS28XX_HandleTypeDef;

void WS28XX_Init(WS28XX_HandleTypeDef *h,
                 TIM_HandleTypeDef *htim,
                 uint32_t tim_channel,
                 uint16_t led_count);

void WS28XX_SetPixel_RGB(WS28XX_HandleTypeDef *h,
                         uint16_t led,
                         uint8_t r,
                         uint8_t g,
                         uint8_t b);

void WS28XX_Update(WS28XX_HandleTypeDef *h);
void WS28XX_DMA_Complete_Callback(WS28XX_HandleTypeDef *h);

#endif
