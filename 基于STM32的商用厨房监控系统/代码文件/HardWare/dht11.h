#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f10x.h"                  // Device header

extern uint8_t dat[5];
extern uint8_t sum;

/*
dat [0] = 湿度整数（% RH）、dat [1] = 湿度小数（恒为 0）；
dat [2] = 温度整数（℃）	  、dat [3] = 温度小数（恒为 0）；
dat [4] = 前 4 个字节的和（校验位），用于验证数据有效性
*/

void DHT11_Input(void);
void DHT11_Out(void);
uint8_t DHT11_Read_Byte(void);
uint8_t DHT11_Read(void);

#endif
