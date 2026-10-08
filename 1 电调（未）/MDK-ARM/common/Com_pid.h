#ifndef COM_PID_H
#define COM_PID_H

#include "main.h"
#define PID_PERIOD 0.006f	//PID计算周期 6ms

// PID结构体 若CPU性能足够，可选用double
// kp,ki,kd需要在初始化时确定，目标值和测量值需要在计算时传递
typedef struct
{
	float kp;		// 比例 => 值越大，响应速度越快
	float ki;		// 积分 => 解决稳态误差，无人机控制中一般不使用
	float kd;		// 微分 => 值越大，抑制效果越好，解决过调震荡
	float err;		// 误差值
	float target;	// 目标值
	float measure;	// 测量值
	float last_err; // 上一次的误差
	float integral; // 积分累积
	float output;	// 输出结果
} PID_Struct;

// 单次PID计算
void Com_PID_Calc(PID_Struct *pid);

// 串级PID计算 => 内环输出值作为最终结果
void Com_PID_Calc_Chain(PID_Struct *outer_pid, PID_Struct *inner_pid);

/**
 * @brief 限幅函数
 * @param speed 需要限幅的值
 * @param max_speed 最大值
 * @param min_speed 最小值
 * @return 限幅后的值
 */
int16_t Com_limit(int16_t speed, int16_t max_speed,int16_t min_speed);

#endif // COM_PID_H
