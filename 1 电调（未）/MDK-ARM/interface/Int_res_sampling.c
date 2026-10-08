#include "Int_res_sampling.h"

uint16_t adc_buff[4] = {0};

/**
 * @brief 初始化采样电阻
 */
void Int_res_sampling_Init(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff, 4);
}

/**
 * @brief 获取电阻采样值并更新到结构体中
 */
void Int_res_sampling_GetValue(Res_Sampling_Struct *res_sampling)
{
    res_sampling->ubemf = adc_buff[0];
    res_sampling->vbemf = adc_buff[1];
    res_sampling->wbemf = adc_buff[2];
    res_sampling->mid = adc_buff[3];
}
