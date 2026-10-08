#include "Int_motor.h"

TIM_OC_InitTypeDef sConfigOC = {
    .OCMode = TIM_OCMODE_PWM1,
    .Pulse = 0,
    .OCPolarity = TIM_OCPOLARITY_HIGH,
    .OCNPolarity = TIM_OCNPOLARITY_HIGH,
    .OCFastMode = TIM_OCFAST_DISABLE,
    .OCIdleState = TIM_OCIDLESTATE_RESET,
    .OCNIdleState = TIM_OCNIDLESTATE_RESET,
};

/**
 * @brief 初始化电机
 */
void Int_motor_Init(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
}

/**
 * @brief 设置高阻(上下桥全断)  00
 */
void Int_motor_SetHighZ(Motor_Struct *motor)
{
    switch (motor->Channel)
    {
    case TIM_CHANNEL_1:
        motor->htim->Instance->CCER &= ~((1 << 0) | (1 << 2)); // CC1E=0，CC1NE=0 0：关闭通道输出
        motor->htim->Instance->CCR1 = 0;
        break;
    case TIM_CHANNEL_2:
        motor->htim->Instance->CCER &= ~((1 << 4) | (1 << 6)); // CC2E=0，CC2NE=0
        motor->htim->Instance->CCR2 = 0;
        break;
    case TIM_CHANNEL_3:
        motor->htim->Instance->CCER &= ~((1 << 8) | (1 << 10)); // CC3E=0，CC3NE=0
        motor->htim->Instance->CCR3 = 0;
        break;
    default:
        break;
    }
}

/**
 * @brief 设置下桥常导通  01
 */
void Int_motor_SetGround(Motor_Struct *motor)
{
    Int_motor_SetHighZ(motor); // 先安全关断

    switch (motor->Channel)
    {
    case TIM_CHANNEL_1:
        motor->htim->Instance->CCER &= ~((1 << 0) | (1 << 2)); // CC1E=0，CC1NE=0	0：关闭通道输出
        motor->htim->Instance->CCR1 = DUTY_MAX;                // CNT一直小于CCR1	输出有效电平
        motor->htim->Instance->CCER |= (1 << 2);               // CC1E=0，CC1NE=1	开启互补通道输出
        break;
    case TIM_CHANNEL_2:
        motor->htim->Instance->CCER &= ~((1 << 4) | (1 << 6)); // CC2E=0，CC2NE=0
        motor->htim->Instance->CCR2 = DUTY_MAX;                // CNT一直小于CCR2	输出有效电平
        motor->htim->Instance->CCER |= (1 << 6);               // CC2E=0，CC2NE=1
        break;
    case TIM_CHANNEL_3:
        motor->htim->Instance->CCER &= ~((1 << 8) | (1 << 10)); // CC3E=0，CC3NE=0
        motor->htim->Instance->CCR3 = DUTY_MAX;                 // CNT一直小于CCR3	输出有效电平
        motor->htim->Instance->CCER |= (1 << 10);               // CC3E=0，CC3NE=1
        break;
    default:
        break;
    }
}

/**
 * @brief 设置上桥PWM输出(调速)  10
 * @param duty 比较值 0~7199
 */
void Int_motor_SetDuty(Motor_Struct *motor, uint16_t duty)
{
    Int_motor_SetHighZ(motor); // 先安全关断

    switch (motor->Channel)
    {
    case TIM_CHANNEL_1:
        motor->htim->Instance->CCR1 = duty;
        motor->htim->Instance->CCER |= ((1 << 0) | (1 << 2)); // CC1E=1，CC1NE=1		开启通道输出
        break;
    case TIM_CHANNEL_2:
        motor->htim->Instance->CCR2 = duty;
        motor->htim->Instance->CCER |= ((1 << 4) | (1 << 6)); // CC2E=1，CC2NE=1
        break;
    case TIM_CHANNEL_3:
        motor->htim->Instance->CCR3 = duty;
        motor->htim->Instance->CCER |= ((1 << 8) | (1 << 10)); // CC3E=1，CC3NE=1
        break;
    default:
        break;
    }
}
