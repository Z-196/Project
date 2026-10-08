#ifndef APP_FLIGHT_H
#define APP_FLIGHT_H

#include "Int_MPU6050.h"
#include "Int_motor.h"
// #include "Int_VL53L1X.h"
#include "Int_VL53L0X.h"
#include "Com_debug.h"
#include "Com_filter.h"
#include "Com_imu.h"
#include "Com_pid.h"
#include "Com_config.h"

/**
 * @brief 飞控任务初始化
 */
void App_flight_init(void);

/**
 * @brief 根据陀螺仪测量的数据，计算出欧拉角
 */
void App_flight_get_euler(void);

/**
 * @brief 根据欧拉角，计算出PID的目标值
 */
void App_flight_pid_process(void);

/**
 * @brief 根据飞行状态控制电机
 */
void App_flight_control_motor(void);

/**
 * @brief 进入定高功能之后的PID计算
 */
void App_flight_fix_height_process(void);

#endif // APP_FLIGHT_H
