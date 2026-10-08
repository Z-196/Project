#ifndef INT_MOTOR_H
#define INT_MOTOR_H

#include "tim.h"
#include "Com_debug.h"

typedef enum
{
    MOTOR_R1 = TIM_CHANNEL_1,
    MOTOR_R2 = TIM_CHANNEL_2,
    MOTOR_L1 = TIM_CHANNEL_3,
    MOTOR_L2 = TIM_CHANNEL_4,
}Motor_Channel_Enum;

typedef struct 
{
    TIM_HandleTypeDef *tim;
    Motor_Channel_Enum channel;
    int16_t speed;
}Motor_Struct;

/**
 * @brief 传入的参数，最大为1000
 * @param motor
 */
void Int_motor_set_speed(Motor_Struct *motor);

/**
 * @brief 启动电机，传入具体电机的结构体
 * @param motor
 */
void Int_motor_start(Motor_Struct *motor);

#endif // __INT_MOTOR_H__
