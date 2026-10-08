#ifndef INT_TP4366_H
#define INT_TP4366_H

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief 启动TP4366电源，避免自动关机
 */
void Int_TP4366_start(void);

/**
 * @brief 软件执行关机指令
 */
void Int_TP4366_shutdown(void);

#endif // INT_TP4366_H

