#ifndef __APP_FREERTOS_TASK_H__
#define __APP_FREERTOS_TASK_H__

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Com_config.h"
#include "Com_pid.h"

/* 引入硬件接口层 */
#include "Int_LED.h"
#include "Int_motor.h"
#include "Int_uart_receive.h"
#include "Int_pwm_in.h"

/* 引入应用层 */
#include "APP_motor.h"

/**
 * @brief 启动FreeRTOS操作系统
 */
void APP_FreeRTOS_start(void);

#endif //__APP_FREERTOS_TASK_H__
