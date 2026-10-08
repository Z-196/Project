#include "tds.h"
#include "delay.h"
#include "adc_all.h"

#define TDS_VREF            3.3f           // 参考电压（V），根据供电调整（3.3V/5V）
#define TDS_ADC_MAX         4095.0f        // 12位ADC最大值（0-4095）
#define TDS_SAMPLE_TIMES    5              // 采样次数（平衡噪声与响应速度）
#define CONDUCTIVITY_SCALE  2000.0f        // 电导率量程（μS/cm），参考传感器手册
#define TDS_CONVERT_RATIO   0.5f           // TDS=电导率×0.5（多数水质适用，可校准）


// 多次采样取平均（减少噪声）
uint16_t TDS_GetADCValue(void) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < TDS_SAMPLE_TIMES; i++) {
        sum += ADC_Read_tds();
        Delay_us(200);  // 短延时避免采样间隔过长
    }
    return (uint16_t)(sum / TDS_SAMPLE_TIMES);  // 返回平均值
}

// 计算电导率（μS/cm）
float TDS_GetConductivity(void) {
    uint16_t adcValue = TDS_GetADCValue();
    // 转换为电压（V）
    float voltage = (adcValue / TDS_ADC_MAX) * TDS_VREF;
    // 线性转换为电导率（根据传感器量程）
    return (voltage / TDS_VREF) * CONDUCTIVITY_SCALE;
}

// 计算TDS值（ppm）
float TDS_GetTDS(void) {
    float conductivity = TDS_GetConductivity();
    // TDS = 电导率 × 转换系数（0.5-0.7，根据校准调整）
    float tds = conductivity * TDS_CONVERT_RATIO;
    // 过滤异常值（仅保留非负值）
    return (tds < 0) ? 0 : tds;
}
