#ifndef COM_FILTER_H
#define COM_FILTER_H

#include "Com_debug.h"
#include "main.h"

typedef struct
{
	float Last_p;	//上一次的状态方差（或协方差）
	float Now_p;	//当前时刻的状态方差（或协方差）
	float Out;		//滤波器的输出值，即估计的状态
	float Kg;		//卡尔曼增益，用于调节预测值和测量值之间的权重
	float Q;		//过程噪声的方差，反映系统模型的不确定性
	float R;		//测量噪声的方差，反映测量过程的不确定性
} KalmanFilter_Struct;
extern KalmanFilter_Struct kfs[3];

/**
 * @brief 一阶低通滤波
 * 
 * @param newvalue 当前值
 * @param preFilterValue 上一次滤波后的值
 * 
 * @return 滤波后的值
 */
int16_t Com_Filter_LowPass(int16_t newvalue,int16_t preFilterValue);

/**
 * @brief 卡尔曼滤波
 * 
 * @param kf 卡尔曼滤波器结构体
 * @param input 输入值
 * 
 * @return 滤波后的值
 */
double Com_Filter_KalmanFilter(KalmanFilter_Struct *kf, double input);

#endif // COM_FILTER_H
