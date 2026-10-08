#include "Int_TP4366.h"

/**
 * @brief 启动TP4366电源，避免自动关机
 */
void Int_TP4366_start(void)
{
    HAL_GPIO_WritePin(KEY_POWER_GPIO_Port, KEY_POWER_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(KEY_POWER_GPIO_Port, KEY_POWER_Pin, GPIO_PIN_SET);
}

/**
 * @brief 软件执行关机指令
 */
void Int_TP4366_shutdown(void)
{
    // 两次按键间隔不超过500ms => 关机
    HAL_GPIO_WritePin(KEY_POWER_GPIO_Port, KEY_POWER_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(KEY_POWER_GPIO_Port, KEY_POWER_Pin, GPIO_PIN_SET);

    vTaskDelay(250);

    HAL_GPIO_WritePin(KEY_POWER_GPIO_Port, KEY_POWER_Pin, GPIO_PIN_RESET);
    vTaskDelay(100);
    HAL_GPIO_WritePin(KEY_POWER_GPIO_Port, KEY_POWER_Pin, GPIO_PIN_SET);
}
