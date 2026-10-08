#ifndef INT_MPU6050_H
#define INT_MPU6050_H

#include "i2c.h"
#include "Com_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h" // abs()

// 从设备地址
// #define MPU6050_ADDR 0x68
#define MPU6050_ADDR 0xD0
// 读写地址
#define MPU6050_ADDR_WRITE 0xD0
#define MPU6050_ADDR_READ 0xD1
// 寄存器地址
#define MPU6050_SMPLRT_DIV 0x19
#define MPU6050_CONFIG 0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_INT_ENABLE 0x38
#define MPU6050_USER_CTRL 0x6A
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_PWR_MGMT_2 0x6C
#define MPU6050_WHO_AM_I 0x75

/**
 * @brief 初始化MPU6050芯片
 */
void Int_MPU6050_Init(void);

/**
 * @brief 在初始化MPU6050完成之后，对MPU6050进行零偏校准
 */
void Int_MPU6050_calculate_offset(void);

/**
 * @brief 读取三轴角速度
 *
 * @param gyro 三轴角速度结构体指针
 */
void Int_MPU6050_Get_Gyro(Gyro_Struct *gyro);

/**
 * @brief 读取三轴加速度
 *
 * @param acc 三轴加速度结构体指针
 */
void Int_MPU6050_Get_Acc(Accel_Struct *acc);

/**
 * @brief 获取所有的六轴数据
 *
 * @param data 六轴数据结构体指针
 */
void Int_MPU6050_Get_Data(Gyro_Accel_Struct *data);

#endif // INT_MPU6050_H
