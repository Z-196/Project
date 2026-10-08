#include "relay.h"

void relay_Init(void)
{
	RCC_APB2PeriphClockCmd(GPIO_CLK_RELAY,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	 
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = RELAY_1;	
	GPIO_Init(GPIO_RELAY,&GPIO_InitStructure);
}

void relay_OFF(uint16_t GPIO_Pin)
{
	GPIO_SetBits(GPIO_RELAY, GPIO_Pin);
}

void relay_ON(uint16_t GPIO_Pin)
{
	GPIO_ResetBits(GPIO_RELAY,GPIO_Pin);
}

void relay_Ture(uint16_t GPIO_Pin)
{
	GPIO_WriteBit(GPIO_RELAY, GPIO_Pin, (BitAction)!GPIO_ReadInputDataBit(GPIO_RELAY, GPIO_Pin));
}
