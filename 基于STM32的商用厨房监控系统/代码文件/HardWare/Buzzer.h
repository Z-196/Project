#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f10x.h"                  // Device header

#define GPIO_CLK_BUZZER		RCC_APB2Periph_GPIOA
#define GPIO_BUZZER			GPIOA
#define BUZZER_X			GPIO_Pin_12

void Buzzer_Init(void);
void Buzzer_ON(void);
void Buzzer_OFF(void);
void Buzzer_Turn(void);



#endif
