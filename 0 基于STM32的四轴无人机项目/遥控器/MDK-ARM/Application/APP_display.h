#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include "Int_oled.h"
#include "FreeRTOS.h"
#include "task.h"
#include "APP_transmit_data.h"

/**
 * 0.96寸OLED显示模块 => 128 * 64
 * 行数（ROW): 16 * 4 => 4行  | 8 * 8 => 8行
 * 列数（COL）：128 * 16 => 16列 | 128 * 8 => 32列
 */
typedef enum
{
    PAGE_1 = 0,
    PAGE_2,
    PAGE_3,
    PAGE_4,
    PAGE_5,
    PAGE_6,
    PAGE_7,
} OLED_Page_Enum;

/**
 * @brief 初始化显示模块
 */
void APP_display_init(void);

/**
 * @brief 循环执行刷写屏幕
 */
void APP_display_show(void);

#endif // APP_DISPLAY_H
