#ifndef INT_MOTOR_H
#define INT_MOTOR_H

#include "tim.h"
#include "Com_debug.h"

#define DUTY_MAX 7200
#define MOTOR_POLARITY 4 // 电机极对数

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t Channel;
} Motor_Struct;

/**
 * @brief 初始化电机
 */
void Int_motor_Init(void);

/**
 * @brief 设置高阻(上下桥全断)  00
 */
void Int_motor_SetHighZ(Motor_Struct *motor);

/**
 * @brief 设置上桥PWM输出(调速)  10
 * @param duty 比较值 0~7199
 */
void Int_motor_SetDuty(Motor_Struct *motor, uint16_t duty);

/**
 * @brief 设置下桥常导通  01
 */
void Int_motor_SetGround(Motor_Struct *motor);

#endif // __INT_MOTOR_H__
