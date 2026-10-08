#include "Int_uart_receive.h"

uint8_t uart_rx_buf[UART_RX_BUF_SIZE];
uint16_t uart_rx_data = 0;

/**
 * @brief 初始化UART接收
 *
 */
void Int_uart_receive_Init(void)
{
    HAL_UART_Receive_DMA(&huart1, uart_rx_buf, UART_RX_BUF_SIZE);
}

/**
 * @brief DMA接收完成回调函数
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        // 解析数据包
        if (uart_rx_buf[0] == 'S' && uart_rx_buf[1] == ':' && uart_rx_buf[6] == '\n')
        {
            // 解析数据
            // 校验中间4位是否为数字（容错：过滤非数字字符）
            if (uart_rx_buf[2] >= '0' && uart_rx_buf[2] <= '9' &&
                uart_rx_buf[3] >= '0' && uart_rx_buf[3] <= '9' &&
                uart_rx_buf[4] >= '0' && uart_rx_buf[4] <= '9' &&
                uart_rx_buf[5] >= '0' && uart_rx_buf[5] <= '9')
            {
                uart_rx_buf[2] = uart_rx_buf[2] - '0';
                uart_rx_buf[3] = uart_rx_buf[3] - '0';
                uart_rx_buf[4] = uart_rx_buf[4] - '0';
                uart_rx_buf[5] = uart_rx_buf[5] - '0';
                uart_rx_data = uart_rx_buf[2] * 1000 + uart_rx_buf[3] * 100 + uart_rx_buf[4] * 10 + uart_rx_buf[5];
            }
        }
        // 清空缓冲区，清除脏数据
        memset(uart_rx_buf, 0, UART_RX_BUF_SIZE);
        HAL_UART_Receive_DMA(&huart1, uart_rx_buf, UART_RX_BUF_SIZE);
    }
}

/**
 * @brief 获取UART接收数据
 *
 * @return uint16_t 接收的16位无符号整数
 */
uint16_t Int_uart_receive_GetData(void)
{
    return uart_rx_data;
}
