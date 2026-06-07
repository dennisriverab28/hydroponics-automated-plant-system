#include "ina219.h"
#include "uart_debug.h"

extern I2C_HandleTypeDef hi2c1;

static uint16_t calibrationValue = 4096;


static INA219_Data_t last_read;

static void INA219_WriteRegister(uint8_t reg, uint16_t value)
{
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = value >> 8;
    buf[2] = value & 0xFF;
    HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDRESS, buf, 3, HAL_MAX_DELAY);
}

static uint16_t INA219_ReadRegister(uint8_t reg)
{
    uint8_t buf[2];
    HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDRESS, &reg, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c1, INA219_ADDRESS, buf, 2, HAL_MAX_DELAY);
    return (buf[0] << 8) | buf[1];
}

void INA219_Init(void)
{
    Debug_Print("INA219: Initializing...\r\n");

    INA219_WriteRegister(INA219_CALIB_REG, calibrationValue);
    INA219_WriteRegister(INA219_CONFIG_REG,0x019F);
    Debug_Print("INA219: Ready.\r\n");
}

void INA219_Read(INA219_Data_t *out)
{
    uint16_t rawShunt = INA219_ReadRegister(INA219_SHUNT_REG);
    uint16_t rawBus   = INA219_ReadRegister(INA219_BUS_REG);
    uint16_t rawCurrent = INA219_ReadRegister(INA219_CURRENT_REG);
    uint16_t rawPower = INA219_ReadRegister(INA219_POWER_REG);

    out->shunt_mV = (float)rawShunt * 0.01f;
    out->bus_V    = (float)(rawBus >> 3) * 0.004f;
    out->current_mA = (float)rawCurrent * 0.1f;
    out->power_mW = (float)rawPower * 2.0f;

    last_read = *out;


    if (out->current_mA >= INA219_MAX_CURRENT_MA)
    {
        GrowLight_CurrentLimit_Callback(out->current_mA);
    }
}

float INA219_GetCurrent_mA(void)
{
    return last_read.current_mA;
}

float INA219_GetBusVoltage_V(void)
{
    return last_read.bus_V;
}
