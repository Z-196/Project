#ifndef __KEY_H
#define __KEY_H
#include "stm32f10x.h"                  // Device header
#include "delay.h"

#define GPIO_CLK_KEY	RCC_APB2Periph_GPIOC
#define GPIO_KEY		GPIOC
#define KEY_1			GPIO_Pin_15
#define KEY_2           GPIO_Pin_14
#define KEY_3           GPIO_Pin_13

void Key_Init(void);
uint8_t Get_Key(void);


#endif
