#include "LED.h"

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(GPIO_CLK_LED, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = LED_1;
	GPIO_Init(GPIO_LED, &GPIO_InitStructure);					
	GPIO_ResetBits(GPIO_LED, LED_1);				
}

void LED_OFF(void)
{				
	GPIO_ResetBits(GPIO_LED, LED_1);	
}

void LED_ON(void)
{
	GPIO_SetBits(GPIO_LED, LED_1);	
}


void LED_Turn(void)
{				
	if (GPIO_ReadOutputDataBit(GPIO_LED, LED_1) == 0)	
			{GPIO_SetBits(GPIO_LED, LED_1);}
	else	{GPIO_ResetBits(GPIO_LED, LED_1);}
}

