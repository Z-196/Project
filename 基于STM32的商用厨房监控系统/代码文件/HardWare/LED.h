#ifndef __LED_H
#define __LED_H
#include "stm32f10x.h"                  // Device header

typedef enum
{
	LED1 = 0
}LED_enum;

#define GPIO_CLK_LED	RCC_APB2Periph_GPIOA
#define GPIO_LED		GPIOA
#define LED_1			GPIO_Pin_8
//#define LED_2           GPIO_Pin_14

void LED_Init(void);
void LED_ON(void);
void LED_OFF(void);
void LED_Turn(void);

#endif
