#include "Com_pid.h"

// 单次PID计算
void Com_PID_Calc(PID_Struct *pid)
{
    // 1. 目标值和测量值 => 计算误差值
    pid->err = pid->measure - pid->target;

    // 2. 计算积分误差
    pid->integral += pid->err;

    // 3. 计算微分误差
    if (pid->last_err == 0)
    {
        pid->last_err = pid->err;
    }
    float der = pid->err - pid->last_err;

    // 4. 计算输出
    pid->output = pid->kp * pid->err + (pid->ki * pid->integral * PID_PERIOD) + (pid->kd * der / PID_PERIOD);

    // 5. 保存上一次误差
    pid->last_err = pid->err;
}

// 串级PID计算 => 内环输出值作为最终结果
void Com_PID_Calc_Chain(PID_Struct *outer_pid, PID_Struct *inner_pid)
{
    // 1. 先计算外环
    Com_PID_Calc(outer_pid);

    // 2. 将外环的输出值作为内环的目标值
    inner_pid->target = outer_pid->output;

    // 3. 计算内环
    Com_PID_Calc(inner_pid);
}

/**
 * @brief 限幅函数
 * @param speed 需要限幅的值
 * @param max_speed 最大值
 * @param min_speed 最小值
 * @return 限幅后的值
 */
int16_t Com_limit(int16_t speed, int16_t max_speed, int16_t min_speed)
{
    if (speed > max_speed)
    {
        return max_speed;
    }
    else if (speed < min_speed)
    {
        return min_speed;
    }
    return speed;
}
