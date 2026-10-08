#include "Com_tool.h"

/**
 * @brief 限制值在min和max之间
 * 
 * @param value 值
 * @param min 最小值
 * @param max 最大值
 * @return int16_t 限制后的值
 */
int16_t Com_limit(int16_t value, int16_t min, int16_t max)
{
    int16_t result = value;
    if (value < min)
    {
        result = min;
    }
    else if (value > max)
    {
        result = max;
    }
    return result;
}
