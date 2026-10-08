#include "Int_motor.h"

/**
 * @brief 传入的参数，最大为1000
 * @param speed
 */
void Int_motor_set_speed(Motor_Struct *motor)
{
    if (motor->speed > 1000)
    {
        debug_printf("Motor speed is too high\r\n");
        motor->speed = 1000;
    }
    else if (motor->speed < 0)
    {
        debug_printf("Motor speed is negative\r\n");
        motor->speed = 0;
    }
    __HAL_TIM_SET_COMPARE(motor->tim, motor->channel, motor->speed);
}

/**
 * @brief 启动电机，传入具体电机的结构体
 * @param motor
 */
void Int_motor_start(Motor_Struct *motor)
{
    HAL_TIM_PWM_Start(motor->tim, motor->channel);
}
