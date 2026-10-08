#ifndef INT_BAT_ADC_H
#define INT_BAT_ADC_H

#include "adc.h"

/**
 * @brief 启动ADC采样
 */
void Int_bat_ADC_Init(void);

/**
 * @brief 直接读取电池电压
 * 
 * @return float 电池电压值(单位:mV)
 */
uint16_t Int_bat_ADC_Read(void);


#endif // INT_BAT_ADC_H
