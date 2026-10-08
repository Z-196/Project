#ifndef COM_DEBUG_H
#define COM_DEBUG_H

#include "usart.h"
#include "stdio.h"
#include "stdarg.h" // （__VA_ARGS__）可变参数列表头文件
#include "string.h" // 字符串处理函数头文件

// 日志输出打印在CPU上非常占用资源 -> 通过比特率进行计算，打印10字节大概需要1ms，影响姿态控制
// 所以在后续飞机需要正常飞行的时候 需要关闭日志输出
// 设计一个日志输出的宏定义，方便后续关闭日志输出
// #define DEBUG_PRINTF_ON 1

#ifdef DEBUG_PRINTF_ON
    //使用宏定义的方式，只打印文件名，不打印路径名称
    // strrchr函数用于在字符串中查找最后一次出现指定字符的位置，返回一个指向该位置的指针，如果没有找到则返回NULL
    #define __FILE_NAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__) // 获取文件名
    #define __FILE_NAME (strrchr(__FILE_NAME__, '/') ? strrchr(__FILE_NAME__, '/') + 1 : __FILE_NAME__) // 获取文件名

    // 使用宏定义的方式能实现打印日志之前，先添加行号和文件名，方便调试
    #define debug_printf(format, ...) printf("[%s,%d]  " format, __FILE_NAME, __LINE__, ##__VA_ARGS__)
#else   
    // 如果关闭日志输出，定义一个空的宏，调用时不会有任何输出
    #define debug_printf(format, ...)
#endif // ifdef

#endif // COM_DEBUG_H__
