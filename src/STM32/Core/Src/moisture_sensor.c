#include "moisture_sensor.h"
#include "uart_debug.h"
#include "main.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc1;

static uint32_t wet_adc  = 2861;
static uint32_t dry_adc  = 2871;

void MoistureSensor_Init(void)
{
    Debug_Print("Moisture Sensor Initialized\r\n");
}

MoistureReading_t Moisture_Read(void)
{
    MoistureReading_t r;

    uint32_t sum = 0;
    const int samples = 64;

    for (int i = 0; i < samples; i++)
    {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        sum += HAL_ADC_GetValue(&hadc1);
    }

    r.avg_adc = sum / samples;
    r.raw_adc = r.avg_adc;
    r.timestamp_ms = HAL_GetTick();

    float range = (float)(dry_adc - wet_adc);
    if (range < 1) range = 1;

    // Normalized to calculate percentage values, if out of bounds, clip
    float norm = 1.0f - ((float)(r.avg_adc - wet_adc) / range);

    if (norm < 0) norm = 0;
    if (norm > 1) norm = 1;

    r.percent = norm * 100.0f;

    // Classified into wet or dry (does not use moisture model for percentage)
    uint32_t mid = (wet_adc + dry_adc) / 2;
    r.state = (r.avg_adc <= mid) ? MOISTURE_WET : MOISTURE_DRY;

    return r;
}

// Print to debug console for moisture metrics
void Moisture_Print(void)
{
    char msg[128];
    MoistureReading_t r = Moisture_Read();

    snprintf(msg, sizeof(msg),
             "MOISTURE | RAW=%lu | AVG=%lu | %.2f%% | %s\r\n",
             r.raw_adc,
             r.avg_adc,
             r.percent,
             (r.state == MOISTURE_WET ? "WET" : "DRY"));

    Debug_Print(msg);
}

static MoistureReading_t lastReading;


// Basic helpers for moisture related functionality
uint32_t Moisture_GetRawADC(void)
{
    return lastReading.raw_adc;
}

uint32_t Moisture_GetAvgADC(void)
{
    return lastReading.avg_adc;
}

float Moisture_GetPercent(void)
{
    return lastReading.percent;
}

bool Moisture_IsWet(void)
{
    return lastReading.state == MOISTURE_WET;
}

bool Moisture_IsDry(void)
{
    return lastReading.state == MOISTURE_DRY;
}

const char* Moisture_GetStateString(void)
{
    return (lastReading.state == MOISTURE_WET ? "WET" : "DRY");
}



