#include "APP_motor.h"

extern Motor_Struct motor_U;
extern Motor_Struct motor_V;
extern Motor_Struct motor_W;

/**
 * @brief 初始化电机
 */
void APP_motor_Init(void)
{
    Int_motor_Init();
    Int_res_sampling_Init();

    // 上电默认全部高阻
    Int_motor_SetHighZ(&motor_U);
    Int_motor_SetHighZ(&motor_V);
    Int_motor_SetHighZ(&motor_W);
}

/**
 * @brief 六步换向
 * @param sector 0~5
 * @param duty 0~7200  0~100%
 */
void APP_Motor_SixStep_Update(uint8_t sector, uint16_t duty)
{
    switch (sector)
    {
    case 0: // U上PWM，V下通，W高阻
        Int_motor_SetDuty(&motor_U, duty);
        Int_motor_SetGround(&motor_V);
        Int_motor_SetHighZ(&motor_W);
        break;
    case 1: // U上PWM，W下通，V高阻
        Int_motor_SetDuty(&motor_U, duty);
        Int_motor_SetGround(&motor_W);
        Int_motor_SetHighZ(&motor_V);
        break;
    case 2: // V上PWM，W下通，U高阻
        Int_motor_SetDuty(&motor_V, duty);
        Int_motor_SetGround(&motor_W);
        Int_motor_SetHighZ(&motor_U);
        break;
    case 3: // V上PWM，U下通，W高阻
        Int_motor_SetDuty(&motor_V, duty);
        Int_motor_SetGround(&motor_U);
        Int_motor_SetHighZ(&motor_W);
        break;
    case 4: // W上PWM，U下通，V高阻
        Int_motor_SetDuty(&motor_W, duty);
        Int_motor_SetGround(&motor_U);
        Int_motor_SetHighZ(&motor_V);
        break;
    case 5: // W上PWM，V下通，U高阻
        Int_motor_SetDuty(&motor_W, duty);
        Int_motor_SetGround(&motor_V);
        Int_motor_SetHighZ(&motor_U);
        break;
    default:
        break;
    }
}

/**
 * @brief 获取电阻采样值、返回当前扇区的悬空相电压
 * @param sector 0~5
 * @param res_sampling 电阻采样结构体指针
 * @return uint16_t 当前扇区的悬空相电压值
 */
uint16_t APP_Motor_GetRes_Sampling(uint8_t sector,Res_Sampling_Struct *res_sampling)
{
    // 1.获取电阻采样值
    Int_res_sampling_GetValue(res_sampling);

    // 2.根据扇区计算悬空相电压
    switch (sector)
    {
        case 0: return res_sampling->wbemf; // W相悬空
        case 1: return res_sampling->vbemf; // V相悬空
        case 2: return res_sampling->ubemf; // U相悬空
        case 3: return res_sampling->wbemf; // W相悬空
        case 4: return res_sampling->vbemf; // V相悬空
        case 5: return res_sampling->ubemf; // U相悬空
        default: return 0;
    }
}
