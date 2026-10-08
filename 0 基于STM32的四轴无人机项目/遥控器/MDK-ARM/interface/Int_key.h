#ifndef __INT_KEY_H
#define __INT_KEY_H

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

typedef enum
{
    KEY_NONE = 0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5_SHORT,
    KEY_5_LONG,
    KEY_6_SHORT,
    KEY_6_LONG,
    KEY_7_SHORT,
    KEY_7_LONG,
    KEY_8_SHORT,
    KEY_8_LONG,
}Key_Type_Enum;
/**
 * @brief 获取当前按键是否被按下
 * 
 * @return Key_Type_Enum 返回当前被按下的按键类型
 */
Key_Type_Enum Int_key_get(void);

#endif  //__INT_KEY_H

