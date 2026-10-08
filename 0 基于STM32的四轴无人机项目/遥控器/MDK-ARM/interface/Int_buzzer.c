#include "Int_buzzer.h"

/**
 * @brief 打开蜂鸣器
 */
void Int_buzzer_Turn_On(BUZZER_Struct *buzzer)
{
    // 直接修改引脚电平为低电平 => 打开蜂鸣器
    HAL_GPIO_WritePin(buzzer->port, buzzer->pin, GPIO_PIN_RESET);
}

/**
 * @brief 关闭蜂鸣器
 */
void Int_buzzer_Turn_Off(BUZZER_Struct *buzzer)
{
    // 直接修改引脚电平为高电平 => 关闭蜂鸣器
    HAL_GPIO_WritePin(buzzer->port, buzzer->pin, GPIO_PIN_SET);
}

/**
 * @brief 翻转蜂鸣器
 */
void Int_buzzer_Toggle(BUZZER_Struct *buzzer)
{
    // 直接翻转引脚电平 => 翻转蜂鸣器
    HAL_GPIO_TogglePin(buzzer->port, buzzer->pin);
}
