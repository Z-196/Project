#ifndef COM_IMU_H
#define COM_IMU_H

#include "Com_debug.h"
#include "Com_config.h"
#include "math.h"

typedef struct
{
	float q0;
	float q1;
	float q2;
	float q3;
} Quaternion_Struct;

extern float RtA;
extern float Gyro_G;
extern float Gyro_Gr;

/**
 * @description: 根据MPU的6轴数据，获取表征姿态的欧拉角
 * @param {Gyro_Accel_Struct} *gyro_accel MPU的6轴数据
 * @param {Euler_Struct} *euler_angel 计算后得到的欧拉角
 * @param {float} dt 采样周期（单位s）
 */
void Com_Imu_GetEulerAngle(Gyro_Accel_Struct *gyro_accel,
						   Euler_Struct *euler_angel,
						   float dt);

/**
 * @description: 获取Z轴上的加速度（如果已经倾斜，则会考虑z轴上加速度的合成
 */
float Com_Imu_GetNormAccZ(void);

#endif // COM_IMU_H
