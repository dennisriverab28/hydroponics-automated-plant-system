#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <stdint.h>
#include <stdbool.h>


typedef enum {
    MODE_AUTO = 0,
    MODE_MANUAL = 1
} SystemMode_t;

typedef enum {
    SOIL_DRY = 0,
    SOIL_WET = 1
} SoilState_t;

typedef enum {
    FAIL_NONE = 0,
    FAIL_OVERCURRENT = 1
} FailState_t;


void State_Init(void);

void State_SetMode(SystemMode_t mode);
SystemMode_t State_GetMode(void);

SoilState_t State_GetSoilState(void);
FailState_t State_GetFailState(void);


void State_UpdateMoisture(void);
void State_UpdatePower(void);
void State_StateMachineTick(void);


int State_GetRawMoisture(void);
int State_GetWetCalibration(void);
int State_GetDryCalibration(void);

#endif
