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

