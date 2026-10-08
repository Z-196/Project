#ifndef __MYSPI_H
#define __MYSPI_H

#include "stm32f10x.h"                  // Device header

#define	GPIO_CLK_MYSPI	RCC_APB2Periph_GPIOA
#define	GPIO_MYSPI		GPIOA
#define MYSPI_MOSI		GPIO_Pin_4			//STM32控制写入
#define	MYSPI_MISO      GPIO_Pin_6			//STM32只能读取，由外部设备写入
#define MYSPI_CLK       GPIO_Pin_5			//STM32模拟时钟
#define MYSPI_SS        GPIO_Pin_7			//STM32控制通信的开关

void MySPI_Init(void);
void MySPI_Start(void);
void MySPI_Stop(void);
uint8_t MySPI_SwapByte(uint8_t ByteSend);

#endif
