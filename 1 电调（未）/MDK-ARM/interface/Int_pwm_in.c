#include "Int_pwm_in.h"

uint32_t capture_high = 0;
uint32_t capture_low = 0;
uint8_t capture_flag = 0;
float duty_cycle = 0.0f;    // 占空比，0~100

/**
 * @brief 初始化PWM信号接收
 *
 */
void Int_pwm_in_Init(void)
{
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_4);
}

/**
 * @brief PWM信号接收中断回调函数
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // CH3(PA2) 上升沿捕获
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
        {
            capture_high = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
            // 重置计数器，为下降沿捕获做准备
            __HAL_TIM_SET_COUNTER(htim, 0);
            capture_flag = 1;
        }
        // CH4 下降沿捕获
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
        {
            if (capture_flag == 1)
            {
                capture_low = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
                // 计算占空比
                duty_cycle = (float)capture_high * 100.0f / (capture_high +capture_low);
                capture_flag = 0;
            }
        }
    }
}

/**
 * @brief 获取PWM信号的占空比
 *
 * @return float PWM信号当前的占空比
 */
float Int_pwm_in_GetData(void)
{
    return duty_cycle;
}
