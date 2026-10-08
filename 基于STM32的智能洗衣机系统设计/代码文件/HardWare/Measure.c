#include "Measure.h"

/********************* 私有函数声明 *********************/
// ADC单次采集（核心：每次采集前重新配置通道）
static uint16_t Measure_ADC_Read(uint8_t adc_channel);

// ADC多次采集取平均
static uint16_t Measure_ADC_ReadAvg(uint8_t adc_channel, uint8_t times);

// 电压转浑浊度
static float Measure_TSB300_VoltToTU(float volt);

// 电压转水位百分比
static float Measure_Water_VoltToPercent(float volt);

/********************* 公有函数实现 *********************/
/**
 * @brief  采集模块初始化（初始化所有GPIO和ADC）
 */
void Measure_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // 1. 开启时钟
    RCC_APB2PeriphClockCmd(MEASURE_ADC_CLK | TSB300_GPIO_CLK | WATER_GPIO_CLK, ENABLE);

    // 2. 配置GPIO为模拟输入（两个传感器引脚）
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;    // 模拟输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;// 速度不影响模拟输入，仅占位
    // 配置TSB300引脚
    GPIO_InitStructure.GPIO_Pin = TSB300_GPIO_PIN;
    GPIO_Init(TSB300_GPIO_PORT, &GPIO_InitStructure);
    // 配置水位引脚
    GPIO_InitStructure.GPIO_Pin = WATER_GPIO_PIN;
    GPIO_Init(WATER_GPIO_PORT, &GPIO_InitStructure);

    // 3. 配置ADC时钟
    RCC_ADCCLKConfig(MEASURE_ADC_PRESCALER);

    // 4. 初始化ADC
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;          // 独立模式
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;       // 右对齐
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;         // 单次转换模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                // 非扫描模式
    ADC_InitStructure.ADC_NbrOfChannel = 1;                      // 通道数=1
    ADC_Init(MEASURE_ADCx, &ADC_InitStructure);

    // 5. 使能ADC并校准
    ADC_Cmd(MEASURE_ADCx, ENABLE);
    // ADC复位校准
    ADC_ResetCalibration(MEASURE_ADCx);
    while (ADC_GetResetCalibrationStatus(MEASURE_ADCx) == SET);
    // ADC开始校准
    ADC_StartCalibration(MEASURE_ADCx);
    while (ADC_GetCalibrationStatus(MEASURE_ADCx) == SET);
}

/**
 * @brief  单次采集所有传感器数据
 * @retval 包含所有数据的结构体
 */
Measure_Result_t Measure_GetAllData(void)
{
    Measure_Result_t result;

    // 1. 采集浑浊度传感器数据
    result.tsb300_volt = (float)Measure_ADC_ReadAvg(TSB300_ADC_CHANNEL, TSB300_AVG_TIMES) * 3.3f / 4096.0f;
    result.tsb300_tu = Measure_TSB300_VoltToTU(result.tsb300_volt);

    // 2. 采集水位传感器数据
    result.water_volt = (float)Measure_ADC_ReadAvg(WATER_ADC_CHANNEL, WATER_AVG_TIMES) * 3.3f / 4096.0f;
    result.water_percent = Measure_Water_VoltToPercent(result.water_volt);

    return result;
}

/**
 * @brief  单独采集浑浊度值（TU）
 * @retval 浑浊度值
 */
float Measure_GetTSB300_TU(void)
{
    float volt = (float)Measure_ADC_ReadAvg(TSB300_ADC_CHANNEL, TSB300_AVG_TIMES) * 3.3f / 4096.0f;
    return Measure_TSB300_VoltToTU(volt);
}

/**
 * @brief  单独采集水位百分比
 * @retval 水位百分比(0-100%)
 */
float Measure_GetWater_Percent(void)
{
    float volt = (float)Measure_ADC_ReadAvg(WATER_ADC_CHANNEL, WATER_AVG_TIMES) * 3.3f / 4096.0f;
    return Measure_Water_VoltToPercent(volt);
}

/**
 * @brief  批量采集（带缓存）
 * @param  buf: 数据缓存数组
 * @param  count: 采集次数
 * @param  interval_ms: 每次采集间隔(ms)
 */
void Measure_BatchCollect(Measure_Result_t *buf, uint16_t count, uint16_t interval_ms)
{
    if (buf == NULL || count == 0) return;

    for (uint16_t i = 0; i < count; i++)
    {
        buf[i] = Measure_GetAllData();
        Delay_ms(interval_ms);
    }
}

/********************* 私有函数实现 *********************/
/**
 * @brief  ADC单次采集（指定通道）
 * @param  adc_channel: ADC通道号
 * @retval 12位ADC值(0-4095)
 */
static uint16_t Measure_ADC_Read(uint8_t adc_channel)
{
    // 每次采集前必须重新配置通道！！
    ADC_RegularChannelConfig(MEASURE_ADCx, adc_channel, 1, ADC_SampleTime_55Cycles5);
    // 软件触发转换
    ADC_SoftwareStartConvCmd(MEASURE_ADCx, ENABLE);
    // 等待转换完成
    while (ADC_GetFlagStatus(MEASURE_ADCx, ADC_FLAG_EOC) == RESET);
    // 返回转换结果
    return ADC_GetConversionValue(MEASURE_ADCx);
}

/**
 * @brief  ADC多次采集取平均
 * @param  adc_channel: ADC通道号
 * @param  times: 平均次数
 * @retval 平均后的ADC值
 */
uint16_t Measure_ADC_ReadAvg(uint8_t adc_channel, uint8_t times)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < times; i++)
    {
        sum += Measure_ADC_Read(adc_channel);
        Delay_ms(5); // 单次采集间隔，滤除噪声
    }
    return (uint16_t)(sum / times);
}

/**
 * @brief  电压转浑浊度（TU）
 * @param  volt: 采集到的电压值(V)
 * @retval 浑浊度值
 */
static float Measure_TSB300_VoltToTU(float volt)
{
    // 公式：TU = -865.68 * 电压 + K
    return -865.68f * volt + TSB300_K;
}

/**
 * @brief  电压转水位百分比
 * @param  volt: 采集到的电压值(V)
 * @retval 水位百分比(0-100%)
 */
static float Measure_Water_VoltToPercent(float volt)
{
    // 边界值保护
    if (volt <= WATER_VOLT_EMPTY) return 0.0f;
    if (volt >= WATER_VOLT_FULL) return 100.0f;
    // 线性计算百分比
    return (volt - WATER_VOLT_EMPTY) / (WATER_VOLT_FULL - WATER_VOLT_EMPTY) * 100.0f;
}
