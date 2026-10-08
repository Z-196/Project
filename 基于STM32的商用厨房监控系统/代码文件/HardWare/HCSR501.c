#include "HCSR501.h"

void HCSR501_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(GPIO_CLK_HCSR501,ENABLE);																
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;					
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = HCSR501_1;				
	GPIO_Init(GPIO_HCSR501, &GPIO_InitStructure);
//	GPIO_InitStructure.GPIO_Pin = HCSR501_2;				
//	GPIO_Init(GPIO_HCSR501, &GPIO_InitStructure);
}

uint8_t HCSR501_ReadData(HCSR_enum HCSR)
{
	if(HCSR	== HCSR1)
	{	
		if(GPIO_ReadInputDataBit(GPIO_HCSR501,HCSR501_1) == 1)	return 1;	//有人
		else	return 0;	//没人
	}
//	else if(HCSR == HCSR2)
//	{
//		if(GPIO_ReadInputDataBit(GPIO_HCSR501,HCSR501_2) == 1)	return 1;	//有人
//		else	return 0;	//没人
//	}
	return 0;
}

