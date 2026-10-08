#ifndef __HCSR501_H
#define __HCSR501_H
/*
热释电红外传感器
输出0/1数字信号
*/
#include "stm32f10x.h"                  // Device header

#define GPIO_CLK_HCSR501	RCC_APB2Periph_GPIOA
#define GPIO_HCSR501		GPIOA
#define HCSR501_1			GPIO_Pin_15
//#define HCSR501_2			GPIO_Pin_5

typedef enum
{
	HCSR1 = 0,
	HCSR2 = 1
}HCSR_enum;

void HCSR501_Init(void);
uint8_t HCSR501_ReadData(HCSR_enum HCSR);


#endif
