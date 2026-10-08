#ifndef __APP_PROCESS_DATA_H__
#define __APP_PROCESS_DATA_H__

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Com_config.h"
#include "Com_tool.h"
#include "Int_joystick.h"
#include "Int_KEY.h"
#include "APP_display.h"

/**
 * @brief 处理按键数据 => 如果有按键按下，需要进行对应的记录
 */
void App_process_key_data(void);

/**
 * @brief 遥感数据读取初始化
 */
void App_process_joystick_data_init(void);

/**
 * @brief 处理摇杆数据 => 修正极性相位和标准值
 */
void App_process_joystick_data(void);

#endif //__APP_PROCESS_DATA_H__
