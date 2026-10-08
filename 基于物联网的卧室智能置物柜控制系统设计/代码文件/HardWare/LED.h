#ifndef __LED_H
#define __LED_H
#include "stm32f10x.h"                  // Device header

void LED_Init(void);
void LED_Set(uint8_t Compare);	//0~100
void LED_ON(void);
void LED_OFF(void);

#endif
