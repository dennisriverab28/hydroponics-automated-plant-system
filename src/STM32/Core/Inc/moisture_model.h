#ifndef MOISTURE_MODEL_H
#define MOISTURE_MODEL_H

#include <stdint.h>
#include <stddef.h>

void MoistureModel_AddState(uint8_t isWet);
float MoistureModel_GetPercent(void);

#endif
