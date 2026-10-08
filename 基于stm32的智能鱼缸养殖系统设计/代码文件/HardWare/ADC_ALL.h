#ifndef __ADC_ALL_H
#define __ADC_ALL_H

#include "stm32f10x.h"

void ADC_ALL_Init(void);

u16 ADC_Read_WaterLevel(void);    	// PA0 通道0
u16 ADC_Read_hzd(void);  			// PA1 通道1
u16 ADC_Read_tds(void);	 			// PA2 通道2

#endif
