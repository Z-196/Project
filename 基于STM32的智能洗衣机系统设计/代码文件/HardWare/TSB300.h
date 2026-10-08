#ifndef __TSB300_H
#define __TSB300_H

#include "stm32f10x.h"                  // Device header
#include "Delay.h"

#define ADC_CLK_TSB300				RCC_APB2Periph_ADC1
#define ADC_TSB300					ADC1
#define ADC_CHANNEL_TSB300			ADC_Channel_5		

#define GPIO_CLK_TSB300	RCC_APB2Periph_GPIOA
#define GPIO_TSB300		GPIOA
#define	TSB300_X		GPIO_Pin_5


#define K 3291.3				//电压校正系数


void HZD_Init(void);

float Get_HZD_V(void);
float Get_HZD(void);


#endif
