#ifndef __WATER_H
#define __WATER_H

#include "stm32f10x.h"

// 这里按你实际硬件校准填写
#define VOLT_EMPTY    0.5f    // 空水位电压
#define VOLT_FULL     2.4f    // 满水位电压

float Get_Water_V(void);
float Get_Water(void);

#endif
