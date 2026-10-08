#ifndef INT_VL53L0X_H
#define INT_VL53L0X_H

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "i2c.h"
#include "VL53L0X.h"
/**
 * @brief VL53L0X硬件复位+完整原厂校准
 * @return 0成功 1:I2C故障 2:ID不匹配 3:校准失败 4:启动测距失败
 */
uint8_t Int_VL53L0X_Init(void);

/**
 * @brief 非阻塞读取毫米距离，0代表无有效测量
 */
uint16_t Int_VL53L0X_ReadContinuous(void);

#endif
