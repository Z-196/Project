#include "Buzzer.h"

void Buzzer_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(GPIO_CLK_BUZZER,ENABLE);																
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;					
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		
	GPIO_InitStructure.GPIO_Pin = BUZZER_X;				
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIO_BUZZER, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIO_BUZZER,BUZZER_X);	
}

void Buzzer_ON(void)
{
	GPIO_ResetBits(GPIO_BUZZER,BUZZER_X);
}

void Buzzer_OFF(void)
{
	GPIO_SetBits(GPIO_BUZZER,BUZZER_X);
}

void Buzzer_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIO_BUZZER,BUZZER_X) == 0)
	{
		GPIO_SetBits(GPIO_BUZZER,BUZZER_X);
	}
	else
	{
		GPIO_ResetBits(GPIO_BUZZER,BUZZER_X);
	}
}
