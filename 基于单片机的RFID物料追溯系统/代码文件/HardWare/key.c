#include "key.h"

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(GPIO_CLK_KEY, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = KEY_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = KEY_2;
	GPIO_Init(GPIO_KEY, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = KEY_3;
	GPIO_Init(GPIO_KEY, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = KEY_4;
	GPIO_Init(GPIO_KEY, &GPIO_InitStructure);
	
}

uint8_t Get_Key(void)
{
	uint8_t KeyNum = 0;
	if (GPIO_ReadInputDataBit(GPIOA, KEY_1) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA, KEY_1) == 0);
		Delay_ms(20);
		KeyNum = 1;
	}
	
	if (GPIO_ReadInputDataBit(GPIO_KEY, KEY_2) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIO_KEY, KEY_2) == 0);
		Delay_ms(20);
		KeyNum = 2;
	}
	
    if (GPIO_ReadInputDataBit(GPIO_KEY, KEY_3) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIO_KEY, KEY_3) == 0);
		Delay_ms(20);
		KeyNum = 3;
	}
	
    if (GPIO_ReadInputDataBit(GPIO_KEY, KEY_4) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIO_KEY, KEY_4) == 0);
		Delay_ms(20);
		KeyNum = 4;
	}

	return KeyNum;
}
