#ifndef __TDS_H
#define __TDS_H

#include "stm32f10x.h"

float TDS_GetConductivity(void);    // 获取电导率（μS/cm）
float TDS_GetTDS(void);             // 获取TDS值（ppm）

#endif
