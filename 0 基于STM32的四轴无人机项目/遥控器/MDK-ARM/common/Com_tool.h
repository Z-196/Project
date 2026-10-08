#ifndef COM_TOOL_H
#define COM_TOOL_H

#include "main.h"

/**
 * @brief 限制值在min和max之间
 * 
 * @param value 值
 * @param min 最小值
 * @param max 最大值
 * @return int16_t 限制后的值
 */
int16_t Com_limit(int16_t value, int16_t min, int16_t max);

#endif //COM_TOOL_H
