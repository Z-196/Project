#include "Int_LED.h"

/**
 * @brief 打开LED灯
 */
void Int_LED_Turn_On(LED_Struct *led)
{
    // 直接修改引脚电平为低电平 => 打开LED灯
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
}

/**
 * @brief 关闭LED灯
 */
void Int_LED_Turn_Off(LED_Struct *led)
{
    // 直接修改引脚电平为高电平 => 关闭LED灯
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
}

/**
 * @brief 翻转LED灯
 */
void Int_LED_Toggle(LED_Struct *led)
{
    // 直接翻转引脚电平 => 翻转LED灯
    HAL_GPIO_TogglePin(led->port, led->pin);
}

