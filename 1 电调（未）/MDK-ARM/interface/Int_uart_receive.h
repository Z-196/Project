#ifndef INT_UART_RECEIVE_H
#define INT_UART_RECEIVE_H

#include "Com_debug.h"

#define UART_RX_BUF_SIZE 7
/* 数据包格式：(S:xxxx\n) => 不得多发、不能少发
帧头：'S',':'
数据：xxxx四位数字
帧尾：'\n'
 */

/**
 * @brief 初始化UART接收
 *
 */
void Int_uart_receive_Init(void);

/**
 * @brief 获取UART接收数据
 *
 * @return uint16_t 接收的16位无符号整数
 */
uint16_t Int_uart_receive_GetData(void);

#endif // INT_UART_RECEIVE_H__
