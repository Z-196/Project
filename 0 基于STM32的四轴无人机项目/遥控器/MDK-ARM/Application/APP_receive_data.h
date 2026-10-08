#ifndef APP_RECEIVE_DATA_H
#define APP_RECEIVE_DATA_H

#include "Com_config.h"
#include "Int_SI24R1.h"

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

#endif // APP_RECEIVE_DATA_H
