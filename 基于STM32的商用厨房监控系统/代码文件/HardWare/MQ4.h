#ifndef __MQ4_H
#define __MQ4_H

//使用ADC1的通道7

#include "math.h"
#include "stm32f10x.h"

void MQ4_Init(void);
float MQ4_GetPPM(uint8_t times);//获取当前烟雾浓度，填入通道和采样次数后取平均值

#endif

