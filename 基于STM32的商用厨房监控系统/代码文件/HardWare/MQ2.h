#ifndef __MQ2_H
#define __MQ2_H

//使用ADC1的通道7

#include "math.h"
#include "stm32f10x.h"

void MQ2_Init(void);
float MQ2_GetPPM(uint8_t times);//获取当前烟雾浓度，填入通道和采样次数后取平均值

#endif

