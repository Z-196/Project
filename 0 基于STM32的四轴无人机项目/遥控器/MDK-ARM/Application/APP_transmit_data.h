#ifndef APP_TRANSMIT_DATA_H
#define APP_TRANSMIT_DATA_H

#include "FreeRTOS.h"
#include "task.h"
#include "Int_SI24R1.h"
#include "APP_process_data.h"
#include "APP_receive_data.h"

// 定义帧头校验的值
#define FRAME_HEAD_CHECK1 'z'
#define FRAME_HEAD_CHECK2 'j'
#define FRAME_HEAD_CHECK3 'x'

// 最大发送重试次数
#define TRANSMIT_MAX_RETRY_TIMES 5

/**
 * @brief 自动切换SI24R1的模式 => 将采集完成的遥控数据打包发送到飞机
 *
 * @return  0：发送成功 1：发送失败
 */
uint8_t APP_transmit_data(void);

/**
 * @brief 处理连接状态
 *
 * @param res 上一次发送数据的返回值
 */
void App_process_remote_state(uint8_t res);

#endif // APP_TRANSMIT_DATA_H
