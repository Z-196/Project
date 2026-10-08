#ifndef INT_PWM_IN_H
#define INT_PWM_IN_H

#include "tim.h"

/**
 * @brief 初始化PWM信号接收
 *
 */
void Int_pwm_in_Init(void);

/**
 * @brief 获取PWM信号的占空比
 *
 * @return float PWM信号当前的占空比    1%~2% => 0~7200
 */
float Int_pwm_in_GetData(void);

#endif // INT_PWM_IN_H
