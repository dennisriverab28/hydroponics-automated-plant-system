#include "moisture_model.h"

static uint8_t buffer[50];
static int idx = 0;
static int filled = 0;

void MoistureModel_AddState(uint8_t isWet)
{
    buffer[idx++] = isWet ? 1 : 0;

    if (idx >= 50)
        idx = 0;

    if (filled < 50)
        filled++;
}

float MoistureModel_GetPercent(void)
{
    if (filled == 0)
        return 0.0f;

    int sum = 0;
    for (int i = 0; i < filled; i++)
        sum += buffer[i];

    return (sum * 100.0f) / filled;
}
