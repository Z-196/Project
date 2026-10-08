#ifndef INT_RES_SAMPLING_H
#define INT_RES_SAMPLING_H

#include "adc.h"
#include <stdlib.h>

typedef struct
{
    uint16_t ubemf;
    uint16_t vbemf;
    uint16_t wbemf;
    uint16_t mid;
} Res_Sampling_Struct;

/**
 * @brief 初始化采样电阻
 */
void Int_res_sampling_Init(void);

/**
 * @brief 获取电阻采样值
 *
 * @return uint16_t 电阻采样值
 */
void Int_res_sampling_GetValue(Res_Sampling_Struct *res_sampling);

#endif // INT_RES_SAMPLING_H
