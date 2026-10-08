#ifndef APP_RECEIVE_DATA_H
#define APP_RECEIVE_DATA_H

#include "Com_config.h"
#include "Int_SI24R1.h"
// #include "Int_VL53L1X.h"
#include "Int_VL53L0X.h"
#include "APP_transmit_data.h"

// 定义帧头校验的值
#define FRAME_HEAD_CHECK1 'z'
#define FRAME_HEAD_CHECK2 'j'
#define FRAME_HEAD_CHECK3 'x'

// 最大重试次数
#define RECEIVE_MAX_RETRY_TIMES 5

/**
 * @brief 接收遥控器发送的遥控数据 => 解析为结构体
 *
 * @return uint8_t 0: 校验通过 1: 没收到数据/校验失败
 */
uint8_t APP_receive_data(void);

/**
 * @brief 处理连接状态
 *
 * @param res 上一次接收数据的返回值
 */
void App_process_remote_state(uint8_t res);

/**
 * @brief 处理飞机的飞行状态
 */
void App_process_flight_state(void);

#endif // APP_RECEIVE_DATA_H
