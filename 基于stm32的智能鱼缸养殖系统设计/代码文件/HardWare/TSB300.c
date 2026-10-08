#include "TSB300.h"
#include "adc_all.h"
#include "Delay.h"

// 多次采样取平均（内部函数）
static uint16_t Get_HZD_AD_Average(uint8_t times)
{
    u32 temp_val = 0;
    u8 t;
    for(t=0; t<times; t++)
    {
        temp_val += ADC_Read_hzd();  // 调用你提供的接口
        Delay_ms(5);
    }
    return temp_val / times;
}

// 获取浑浊度传感器电压
float Get_HZD_V(void)
{
    uint16_t adx;
    float HZD_Value;
    
    adx = Get_HZD_AD_Average(10);  // 10次平均
    HZD_Value = adx * 3.3 / 4096;
    
    return HZD_Value;
}

// 获取最终浑浊度值 TU
float Get_HZD(void)
{
    float hzd_Value;
    float TU;
    hzd_Value = Get_HZD_V();
    TU = -865.68 * hzd_Value + K;
    return TU;
}
