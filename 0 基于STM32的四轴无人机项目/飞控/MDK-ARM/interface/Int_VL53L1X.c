#include "Int_VL53L1X.h"

const uint16_t dev = 0x52; // 设备地址

/**
 * @brief 初始化激光测距仪，完成寄存器的配置
 */
void Int_VL53L1X_Init(void)
{
    // 1. 重置芯片 => XSHUT引脚拉低再拉高
    HAL_GPIO_WritePin(VX_XSHUT_GPIO_Port, VX_XSHUT_Pin, GPIO_PIN_RESET);
    vTaskDelay(20);
    HAL_GPIO_WritePin(VX_XSHUT_GPIO_Port, VX_XSHUT_Pin, GPIO_PIN_SET);
    vTaskDelay(70);

    uint8_t byteData = 0;
    uint16_t wordData = 0;
    uint8_t sensorState = 0;
    VL53L1_RdByte(dev, 0x010F, &byteData);
    debug_printf("VL53L1X Model_ID: %X\n", byteData);

    VL53L1_RdByte(dev, 0x0110, &byteData);
    debug_printf("VL53L1X Module_Type: %X\n", byteData);

    VL53L1_RdWord(dev, 0x010F, &wordData);
    debug_printf("VL53L1X: %X\n", wordData);

    while (sensorState == 0)
    {
        VL53L1X_BootState(dev, &sensorState);
        vTaskDelay(2);
    }
    debug_printf("Chip booted\n");

    VL53L1X_SensorInit(dev);
    VL53L1X_SetDistanceMode(dev, 2);          /* 1=short, 2=long */
    VL53L1X_SetTimingBudgetInMs(dev, 20);     /* in ms possible values [20, 50, 100, 200, 500] */
    VL53L1X_SetInterMeasurementInMs(dev, 30); /* in ms, IM must be > = TB */
    VL53L1X_StartRanging(dev);                /* This function has to be called to enable the ranging */
    debug_printf("VL53L1X Ultra Lite Driver Example running ...\n");
}

/**
 * @brief 获取激光测距仪的距离
 *
 * @return uint16_t 距离，单位mm
 */
uint16_t Int_VL53L1X_GetDistance(void)
{
    uint16_t distance;
    VL53L1X_GetDistance(dev, &distance);
    return distance;
}
