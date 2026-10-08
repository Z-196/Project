#ifndef __WATER_H
#define __WATER_H

#include "stm32f10x.h"                  // Device header
#include "Delay.h"

#define ADC_CLK_WATER		RCC_APB2Periph_ADC1
#define ADC_WATER			ADC1
#define ADC_CHANNEL_WATER	ADC_Channel_6		

#define GPIO_CLK_WATER		RCC_APB2Periph_GPIOA
#define GPIO_WATER			GPIOA
#define	WATER_X				GPIO_Pin_6

#define VOLT_EMPTY	 0.5f	//无水时电压
#define VOLT_FULL	 2.8f	//满水时电压

void Water_Init(void);
uint16_t AD_GetValue(void);
float Get_Water_V(void);
float Get_Water(void);


#endif
