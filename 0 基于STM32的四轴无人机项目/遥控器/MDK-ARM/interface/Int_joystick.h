#ifndef __INT_JOYSTICK_H
#define __INT_JOYSTICK_H

#include "adc.h"

typedef struct
{
    int16_t thr; // 油门
    int16_t yaw; // 航向
    int16_t pit; // 俯仰
    int16_t rol; // 横滚
} Joystick_Struct;

/**
 * @brief 初始化ADC摇杆 打开ADC
 */
void Int_joystick_init(void);

/**
 * @brief 获取摇杆数据
 * 
 * @param joystick 摇杆数据结构体指针
 */
void Int_joystick_get(Joystick_Struct *joystick);

#endif //__INT_JOYSTICK_H
