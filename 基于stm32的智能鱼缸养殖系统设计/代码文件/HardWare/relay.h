#ifndef __RELAY5V_H
#define __RELAY5V_H
#include "stm32f10x.h"                  // Device header

#define GPIO_CLK_RELAY	RCC_APB2Periph_GPIOB
#define GPIO_RELAY		GPIOB
#define	RELAY_1			GPIO_Pin_13
#define	RELAY_2			GPIO_Pin_12
#define	RELAY_3			GPIO_Pin_15
#define	RELAY_4			GPIO_Pin_14

void relay_Init(void);
void relay_ON(uint16_t GPIO_Pin);
void relay_OFF(uint16_t GPIO_Pin);
void relay_Ture(uint16_t GPIO_Pin);

#endif
