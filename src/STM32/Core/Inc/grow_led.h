#ifndef GROW_LED_H
#define GROW_LED_H

#include "ws28xx.h"
#include "main.h"
#include <stdint.h>

#define WS_LED_COUNT 20

extern WS28XX_HandleTypeDef ws_led;


void init_grow_leds();
void update_grow_leds(uint8_t brightness);
void flash_error_led();


void grow_set_brightness(uint8_t brightness);
void grow_dim_to_safe();
void grow_full_brightness();



void SetPixel_BGR(uint16_t index, uint8_t r, uint8_t g, uint8_t b);

#endif
