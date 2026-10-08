#ifndef __ECG_H
#define __ECG_H
#include "stm32f10x.h"                  // Device header

#include "max30102.h"
#include "algorithm.h"
#include "oled.h"
#include "Delay.h"

void max30102_blood(void);       //max30102数据处理
void heart_SpO2(void);    //max30102心率血氧显示

extern float Temp;
extern uint8_t hr0,spo2;

#endif


