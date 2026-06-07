#ifndef MOISTURE_SENSOR_H
#define MOISTURE_SENSOR_H

#include <stdint.h>
#include <stdbool.h>


typedef enum {
    MOISTURE_DRY = 0,
    MOISTURE_WET = 1
} MoistureState_t;


typedef struct {
    uint32_t raw_adc;
    uint32_t avg_adc;
    float    percent;
    MoistureState_t state;
    uint32_t timestamp_ms;
} MoistureReading_t;


void MoistureSensor_Init(void);
MoistureReading_t Moisture_Read(void);
void Moisture_Print(void);


bool Moisture_IsWet(void);
bool Moisture_IsDry(void);
float Moisture_GetPercent(void);
uint32_t Moisture_GetAvgADC(void);
uint32_t Moisture_GetRawADC(void);
const char* Moisture_GetStateString(void);




#endif
