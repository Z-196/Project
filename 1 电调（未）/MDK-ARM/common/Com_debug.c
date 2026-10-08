#include "Com_debug.h"

// 重定向编写一个函数
int fputc(int ch, FILE *f)
{
    // 将字符 ch 通过串口发送出去
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 200); // 发送一个字符，等待200ms

    return ch; // 返回写入的字符
}
