#ifndef INT_BUZZER_H
#define INT_BUZZER_H

#include "main.h"

typedef struct
{
    GPIO_TypeDef *port;  // BUZZER所在的GPIO端口
    uint16_t pin;        // BUZZER所在的GPIO引脚
} BUZZER_Struct;

/**
 * @brief 打开蜂鸣器
 */
void Int_buzzer_Turn_On(BUZZER_Struct *buzzer);

/**
 * @brief 关闭蜂鸣器
 */
void Int_buzzer_Turn_Off(BUZZER_Struct *buzzer);

/**
 * @brief 翻转蜂鸣器
 */
void Int_buzzer_Toggle(BUZZER_Struct *buzzer);

#endif  // INT_BUZZER_H
