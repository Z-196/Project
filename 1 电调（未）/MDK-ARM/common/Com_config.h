#ifndef COM_CONFIG_H // 存储基础配置信息
#define COM_CONFIG_H

#include "main.h"

// 电机运行阶段枚举
typedef enum
{
    MOTOR_STAGE_POS = 0,  // 预定位
    MOTOR_STAGE_OPENLOOP, // 开环加速
    MOTOR_STAGE_CLOSELOOP // 闭环BEMF换相
} Motor_Run_Stage;

#endif // COM_CONFIG_H
