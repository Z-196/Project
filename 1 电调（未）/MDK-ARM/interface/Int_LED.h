#ifndef INT_LED_H
#define INT_LED_H

#include "main.h"

typedef struct
{
    GPIO_TypeDef *port;  // LED所在的GPIO端口
    uint16_t pin;        // LED所在的GPIO引脚
} LED_Struct;

/**
 * @brief 打开LED灯
 */
void Int_LED_Turn_On(LED_Struct *led);

/**
 * @brief 关闭LED灯
 */
void Int_LED_Turn_Off(LED_Struct *led);

/**
 * @brief 翻转LED灯
 */
void Int_LED_Toggle(LED_Struct *led);

#endif  // __INT_LED_H
