#ifndef INA219_H
#define INA219_H

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>


#define INA219_ADDRESS        (0x40 << 1)
#define INA219_CONFIG_REG     0x00
#define INA219_SHUNT_REG      0x01
#define INA219_BUS_REG        0x02
#define INA219_POWER_REG      0x03
#define INA219_CURRENT_REG    0x04
#define INA219_CALIB_REG      0x05


#define INA219_SHUNT_R_OHMS   0.1f


#define INA219_MAX_CURRENT_MA 800


typedef struct {
    float shunt_mV;
    float bus_V;
    float current_mA;
    float power_mW;
} INA219_Data_t;


void INA219_Init(void);
void INA219_Read(INA219_Data_t *out);
float INA219_GetCurrent_mA(void);
float INA219_GetBusVoltage_V(void);


void GrowLight_CurrentLimit_Callback(float current_mA);

#endif
