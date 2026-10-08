#ifndef __MEASURE_H
#define __MEASURE_H

#include "stm32f10x.h"
#include "Delay.h"

/********************* 硬件参数定义 *********************/
// 共用ADC参数
#define MEASURE_ADCx            ADC1
#define MEASURE_ADC_CLK         RCC_APB2Periph_ADC1
#define MEASURE_ADC_PRESCALER   RCC_PCLK2_Div6  // ADC时钟分频（72M/6=12M）

// 浑浊度传感器（TSB300）参数
#define TSB300_GPIO_PORT        GPIOA
#define TSB300_GPIO_CLK         RCC_APB2Periph_GPIOA
#define TSB300_GPIO_PIN         GPIO_Pin_5
#define TSB300_ADC_CHANNEL      ADC_Channel_5
#define TSB300_AVG_TIMES        10              // 采集平均次数
#define TSB300_K                3291.3f         // 电压校正系数

// 水位传感器参数
#define WATER_GPIO_PORT         GPIOA
#define WATER_GPIO_CLK          RCC_APB2Periph_GPIOA
#define WATER_GPIO_PIN          GPIO_Pin_6
#define WATER_ADC_CHANNEL       ADC_Channel_6
#define WATER_AVG_TIMES         5               // 采集平均次数
#define WATER_VOLT_EMPTY        0.7f            // 无水电压(V)
#define WATER_VOLT_FULL         1.35f            // 满水电压(V)

/********************* 数据结构定义 *********************/
// 采集结果结构体（一次性返回所有传感器数据）
typedef struct
{
    float tsb300_volt;    // TSB300电压值(V)
    float tsb300_tu;      // TSB300浑浊度值(TU)
    float water_volt;     // 水位电压值(V)
    float water_percent;  // 水位百分比(0-100%)
} Measure_Result_t;

/********************* 函数声明 *********************/
// 采集模块初始化（初始化所有GPIO和ADC）
void Measure_Init(void);

// 单次采集所有传感器数据（返回结构体）
Measure_Result_t Measure_GetAllData(void);

// 单独采集浑浊度数据
float Measure_GetTSB300_TU(void);

// 单独采集水位百分比
float Measure_GetWater_Percent(void);

// 批量采集（带缓存，用于连续采样）
void Measure_BatchCollect(Measure_Result_t *buf, uint16_t count, uint16_t interval_ms);

#endif
