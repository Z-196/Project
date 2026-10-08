#ifndef APP_MOTOR_H
#define APP_MOTOR_H

#include "Int_motor.h"
#include "Int_res_sampling.h"

/**
 * @brief 初始化电机
 */
void APP_motor_Init(void);

/**
 * @brief 六步换向
 * @param sector 0~5
 * @param duty 0~7199  0~100%  100%为最大速速
 */
void APP_Motor_SixStep_Update(uint8_t sector, uint16_t duty);

/**
 * @brief 获取电阻采样值、返回当前扇区的悬空相电压
 * @param sector 0~5
 * @param res_sampling 电阻采样结构体指针
 * @return uint16_t 当前扇区的悬空相电压值
 */
uint16_t APP_Motor_GetRes_Sampling(uint8_t sector, Res_Sampling_Struct *res_sampling);

#endif // APP_MOTOR_H
