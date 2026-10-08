#include "Int_key.h"

/**
 * @brief 获取当前按键是否被按下
 *
 * @return Key_Type_Enum 返回当前被按下的按键类型
 */
Key_Type_Enum Int_key_get(void)
{
    if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
    {
        // 1. 电弧抖动 => 需要进行消抖
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
        {
            // 2. 人按下的时间通常比较长 => 为了不被多次判断 => 等待抬起按键才返回
            while (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_1; // KEY1被按下
        }
    }
    else if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
        {
            while (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_2; // KEY2被按下
        }
    }
    else if (HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == GPIO_PIN_RESET)
        {
            while (HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_3; // KEY3被按下
        }
    }
    else if (HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == GPIO_PIN_RESET)
        {
            while (HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            return KEY_4; // KEY4被按下
        }
    }
    else if (HAL_GPIO_ReadPin(KEY5_GPIO_Port, KEY5_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY5_GPIO_Port, KEY5_Pin) == GPIO_PIN_RESET)
        {
            TickType_t startTime = xTaskGetTickCount();
            while (HAL_GPIO_ReadPin(KEY5_GPIO_Port, KEY5_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            TickType_t endTime = xTaskGetTickCount();
            if (endTime - startTime > 1000)
            {
                return KEY_5_LONG; // KEY5被长按
            }
            else
            {
                return KEY_5_SHORT; // KEY5被短按
            }
        }
    }
    else if (HAL_GPIO_ReadPin(KEY6_GPIO_Port, KEY6_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY6_GPIO_Port, KEY6_Pin) == GPIO_PIN_RESET)
        {
            TickType_t startTime = xTaskGetTickCount();
            while (HAL_GPIO_ReadPin(KEY6_GPIO_Port, KEY6_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            TickType_t endTime = xTaskGetTickCount();
            if (endTime - startTime > 1000)
            {
                return KEY_6_LONG; // KEY6被长按
            }
            else
            {
                return KEY_6_SHORT; // KEY6被短按
            }
        }
    }
    else if (HAL_GPIO_ReadPin(KEY7_GPIO_Port, KEY7_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY7_GPIO_Port, KEY7_Pin) == GPIO_PIN_RESET)
        {
            TickType_t startTime = xTaskGetTickCount();
            while (HAL_GPIO_ReadPin(KEY7_GPIO_Port, KEY7_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            TickType_t endTime = xTaskGetTickCount();
            if (endTime - startTime > 1000)
            {
                return KEY_7_LONG; // KEY7被长按
            }
            else
            {
                return KEY_7_SHORT; // KEY7被短按
            }
        }
    }
    else if (HAL_GPIO_ReadPin(KEY8_GPIO_Port, KEY8_Pin) == GPIO_PIN_RESET)
    {
        vTaskDelay(5);
        if (HAL_GPIO_ReadPin(KEY8_GPIO_Port, KEY8_Pin) == GPIO_PIN_RESET)
        {
            TickType_t startTime = xTaskGetTickCount();
            while (HAL_GPIO_ReadPin(KEY8_GPIO_Port, KEY8_Pin) == GPIO_PIN_RESET)
            {
                vTaskDelay(1);
            }
            TickType_t endTime = xTaskGetTickCount();
            if (endTime - startTime > 1000)
            {
                return KEY_8_LONG; // KEY8被长按
            }
            else
            {
                return KEY_8_SHORT; // KEY8被短按
            }
        }
    }
    return KEY_NONE;
}
