#ifndef __APP_FREERTOS_TASK_H__
#define __APP_FREERTOS_TASK_H__

#include "FreeRTOS.h"
#include "task.h"
#include "Com_debug.h"
#include "Com_config.h"

/* 引入硬件接口层 */
#include "Int_TP4366.h"
#include "Int_motor.h"
#include "Int_LED.h"
#include "Int_SI24R1.h"
#include "Int_bat_ADC.h"

/* 引入应用层 */
#include "APP_flight.h"
#include "APP_receive_data.h"
#include "APP_transmit_data.h"
#include "APP_led_control.h"

/**
 * @brief 启动FreeRTOS操作系统
 */
void APP_FreeRTOS_start(void);


#endif  //__APP_FREERTOS_TASK_H__
