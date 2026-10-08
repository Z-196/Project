#include "Int_bat_ADC.h"

extern uint16_t adc_buff[5];    //摇杆任务中会循环读取ADC所有通道的值

/**
 * @brief 打开测量电路，ADC外设已在摇杆任务中打开
 */
void Int_bat_ADC_Init(void)
{
    // 打开使能引脚
    HAL_GPIO_WritePin(BAT_EN_GPIO_Port, BAT_EN_Pin, GPIO_PIN_SET);
}

/**
 * @brief 直接读取电池电压
 *
 * @return float 电池电压值(单位:mV)
 */
uint16_t Int_bat_ADC_Read(void)
{
    // 读取ADC值
    uint32_t adc_value = adc_buff[4];
    
    // 将ADC值转换为电压值(假设ADC满量程3.3V，12位分辨率)
    float voltage = adc_value * 3300 * 2 / 4095.0f;
    
    return voltage;
}
