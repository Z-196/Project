#include "TSB300.h"

void HZD_Init(void)
{
	RCC_APB2PeriphClockCmd(ADC_CLK_TSB300, ENABLE);
	RCC_APB2PeriphClockCmd(GPIO_CLK_TSB300, ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = TSB300_X;
	GPIO_Init(GPIO_TSB300, &GPIO_InitStructure);
		
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC_TSB300, &ADC_InitStructure);
	
	ADC_Cmd(ADC_TSB300, ENABLE);
	
	ADC_ResetCalibration(ADC_TSB300);
	while (ADC_GetResetCalibrationStatus(ADC_TSB300) == SET);
	ADC_StartCalibration(ADC_TSB300);
	while (ADC_GetCalibrationStatus(ADC_TSB300) == SET);
}

static uint16_t AD_GetValue(uint8_t ADC_Channel)
{
	ADC_RegularChannelConfig(ADC_TSB300, ADC_Channel, 1, ADC_SampleTime_55Cycles5);
	ADC_SoftwareStartConvCmd(ADC_TSB300, ENABLE);
	while (ADC_GetFlagStatus(ADC_TSB300, ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC_TSB300);
}
 
static uint16_t Get_AD_Average(uint8_t ADC_Channel,uint8_t times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=AD_GetValue(ADC_Channel);
		Delay_ms(5);
	}
	return temp_val/times;
}

//10次测量取平均
float Get_HZD_V(void)
{
	uint16_t adx;
	float HZD_Value;
	
	adx = Get_AD_Average(ADC_CHANNEL_TSB300,10);
	HZD_Value = adx*3.3/4096;
	
	return HZD_Value;
}

float Get_HZD(void)
{
	float hzd_Value;
	float TU;
	hzd_Value = Get_HZD_V();			//获取浑浊度电压
	TU = -865.68 * hzd_Value + K;		//计算浑浊度
	return TU;
}
