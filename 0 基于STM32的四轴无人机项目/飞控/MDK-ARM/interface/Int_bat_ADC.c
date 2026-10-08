#include "Int_bat_ADC.h"

/**
 * @brief 启动ADC采样
 */
void Int_bat_ADC_Init(void)
{
    // 1. 打开使能引脚
    HAL_GPIO_WritePin(BAT_ADC_EN_GPIO_Port, BAT_ADC_EN_Pin, GPIO_PIN_RESET);

    // 2. 完成ADC初始化
    HAL_ADC_Start(&hadc1);
}

/**
 * @brief 直接读取电池电压
 *
 * @return float 电池电压值(单位:mV)
 */
uint16_t Int_bat_ADC_Read(void)
{
    uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
    
    // 将ADC值转换为电压值(假设ADC满量程3.3V，12位分辨率)
    float voltage = adc_value * 3300 * 2 / 4095.0f;
    
    return voltage;
}
