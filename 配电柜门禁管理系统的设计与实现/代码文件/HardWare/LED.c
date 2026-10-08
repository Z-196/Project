#include "LED.h"

#define GPIO_CLK_LED	RCC_APB2Periph_GPIOA
#define GPIO_LED		GPIOA
#define LED_1			GPIO_Pin_0
#define LED_2           GPIO_Pin_1

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(GPIO_CLK_LED, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = LED_1;
	GPIO_Init(GPIO_LED, &GPIO_InitStructure);					
	GPIO_InitStructure.GPIO_Pin = LED_2;
	GPIO_Init(GPIO_LED, &GPIO_InitStructure);
	
	LED_OFF(LED2);
	LED_OFF(LED1);
}

void LED_OFF(LED_enum LED)
{			
	if(LED == LED1)		GPIO_SetBits(GPIO_LED, LED_1);
	else if(LED == LED2)GPIO_SetBits(GPIO_LED, LED_2);	
}

void LED_ON(LED_enum LED)
{
	if(LED == LED1)		GPIO_ResetBits(GPIO_LED, LED_1);
	else if(LED == LED2)GPIO_ResetBits(GPIO_LED, LED_2);		
}


void LED_Turn(LED_enum LED)
{				
	if(LED == LED1)	
	{
		if (GPIO_ReadOutputDataBit(GPIO_LED, LED_1) == 0)	
				{GPIO_SetBits(GPIO_LED, LED_1);}
		else	{GPIO_ResetBits(GPIO_LED, LED_1);}
	}	
	else if(LED == LED2)
	{
		if (GPIO_ReadOutputDataBit(GPIO_LED, LED_2) == 0)	
				{GPIO_SetBits(GPIO_LED, LED_2);}
		else	{GPIO_ResetBits(GPIO_LED, LED_2);}
	}	
		
}

