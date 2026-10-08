#ifndef __LED_H
#define __LED_H
#include "stm32f10x.h"                  // Device header

typedef enum
{
	LED1 = 0,
	LED2 = 1
}LED_enum;

#define GPIO_CLK_LED	RCC_APB2Periph_GPIOB
#define GPIO_LED		GPIOB
#define LED_1			GPIO_Pin_13
#define LED_2           GPIO_Pin_12


void LED_Init(void);
void LED_ON(LED_enum LED);
void LED_OFF(LED_enum LED);
void LED_Turn(LED_enum LED);

#endif
