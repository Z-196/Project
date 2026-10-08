#include "Int_VL53L0X.h"

/**
 * @brief 初始化入口
 * @return 0成功 1:I2C故障
 */
uint8_t Int_VL53L0X_Init(void)
{
    // Initialise the VL53L0X
    if (initVL53L0X(1, &hi2c1) != true)
        return 1;

    // Configure the sensor for high accuracy and speed in 20 cm.
    // 初始化成功后配置
    setSignalRateLimit(0.1f);
    setVcselPulsePeriod(VcselPeriodPreRange, 18);
    setVcselPulsePeriod(VcselPeriodFinalRange, 14);
    setMeasurementTimingBudget(20000UL); // 20 ms
    startContinuous(24); // 定时测距周期 24 ms
    return 0;
}

/**
 * @brief 读取距离，增加状态码判断调试
 * @return uint16_t 距离  0~2000
 * */
uint16_t Int_VL53L0X_ReadContinuous(void)
{
    statInfo_t_VL53L0X distanceStr;
    uint16_t distance = readRangeContinuousMillimeters(&distanceStr);
    if (distance > 0 && distance < 2000)
        return distance;
    return 0;
}
