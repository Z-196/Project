#ifndef INT_VL53L1X_H
#define INT_VL53L1X_H

#include "vl53l1_platform.h"
#include "vl53l1X_api.h"
#include "VL53L1X_calibration.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"

/**
 * @brief 初始化激光测距仪，完成寄存器的配置
 */
void Int_VL53L1X_Init(void);

/**
 * @brief 获取激光测距仪的距离
 * 
 * @return uint16_t 距离，单位mm
 */
uint16_t Int_VL53L1X_GetDistance(void);


#endif // INT_VL53L1X_H
