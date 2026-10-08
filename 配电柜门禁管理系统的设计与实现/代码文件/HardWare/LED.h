#ifndef __LED_H
#define __LED_H
#include "stm32f10x.h"                  // Device header

typedef enum
{
	LED1 = 0,
	LED2 = 1,
}LED_enum;

void LED_Init(void);
void LED_ON(LED_enum LED);
void LED_OFF(LED_enum LED);
void LED_Turn(LED_enum LED);

#endif
