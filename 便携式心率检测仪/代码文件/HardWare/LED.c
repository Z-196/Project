#include "LED.h"

void LED_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(GPIO_CLK_LED, ENABLE);

	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = LED_1;
	GPIO_Init(GPIO_LED, &GPIO_InitStructure);			
	GPIO_InitStructure.GPIO_Pin = LED_2;
	GPIO_Init(GPIO_LED, &GPIO_InitStructure);		
		
	/*设置GPIO初始化后的默认电平*/
	GPIO_SetBits(GPIO_LED, LED_1);				
	GPIO_SetBits(GPIO_LED, LED_2);
}

void LED_ON(LED_enum LED)
{				
	switch (LED)		
	{
		case LED1:
			GPIO_ResetBits(GPIO_LED, LED_1);	
			break;
		case LED2:
			GPIO_ResetBits(GPIO_LED, LED_2);	
			break;
		default:
			break;
	}
}


void LED_OFF(LED_enum LED)
{
	switch (LED)		
	{
		case LED1:
			GPIO_SetBits(GPIO_LED, LED_1);	
			break;
		case LED2:
			GPIO_SetBits(GPIO_LED, LED_2);	
			break;
		default:
			break;
	}
}


void LED_Turn(LED_enum LED)
{
	switch (LED)		
	{
		case LED1:
			if (GPIO_ReadOutputDataBit(GPIO_LED, LED_1) == 0)	
					{GPIO_SetBits(GPIO_LED, LED_1);}
			else	{GPIO_ResetBits(GPIO_LED, LED_1);}
			break;
		case LED2:
			if (GPIO_ReadOutputDataBit(GPIO_LED, LED_2) == 0)	
					{GPIO_SetBits(GPIO_LED, LED_2);}
			else	{GPIO_ResetBits(GPIO_LED, LED_2);}
			break;		
		default:
			break;
	}
}

