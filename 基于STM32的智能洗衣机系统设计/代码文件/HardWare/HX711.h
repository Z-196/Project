#ifndef __HX711_H
#define __HX711_H

#include "stm32f10x.h"                  // Device header
#include "delay.h"  
#include "stdint.h"

#define GPIO_CLK_HX711	RCC_APB2Periph_GPIOB
#define GPIO_HX711		GPIOB
#define HX711_SCK		GPIO_Pin_11
#define HX711_DOUT		GPIO_Pin_10

#define HX711_TIMEOUT_US  100000  // 100ms超时（避免死等）

#define GapValue 		370
/*
校准参数
因为不同的传感器特性曲线不是很一致，因此，
每一个传感器需要矫正这里这个参数才能使测量值很准确。
当发现测试出来的重量偏大时，增加该数值。
如果测试出来的重量偏小时，减小改数值。
该值可以为小数
*/

extern uint32_t Weight_Shiwu;
extern uint32_t Weight_Maopi;

void HX711_Init(void);
void Get_Weight(void);
void Get_Maopi(void);

#endif
