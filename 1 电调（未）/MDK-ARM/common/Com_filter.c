#include "Com_filter.h"

#define LOWPASS_ALPHA 0.15 /* 一阶低通滤波系数，指数加权系数 */
/**
 * @brief 一阶低通滤波
 *
 * @param newvalue 当前值
 * @param preFilterValue 上一次滤波后的值
 *
 * @return 滤波后的值
 */
int16_t Com_Filter_LowPass(int16_t newvalue, int16_t preFilterValue)
{
    return LOWPASS_ALPHA * newvalue + (1 - LOWPASS_ALPHA) * preFilterValue;
}

/* 卡尔曼滤波参数 */
KalmanFilter_Struct kfs[3] = {
    {0.02, 0, 0, 0, 0.001, 0.543}, // x
    {0.02, 0, 0, 0, 0.001, 0.543}, // y
    {0.02, 0, 0, 0, 0.001, 0.543}, // z
};
/**
 * @brief 卡尔曼滤波
 *
 * @param kf 卡尔曼滤波器结构体
 * @param input 输入值
 *
 * @return 滤波后的值
 */
double Com_Filter_KalmanFilter(KalmanFilter_Struct *kf, double input)
{
    kf->Now_p = kf->Last_p + kf->Q;
    kf->Kg = kf->Now_p / (kf->Now_p + kf->R);
    kf->Out = kf->Out + kf->Kg * (input - kf->Out);
    kf->Last_p = (1 - kf->Kg) * kf->Now_p;
    return kf->Out;
}
