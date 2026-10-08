#ifndef COM_CONFIG_H // 存储基础配置信息
#define COM_CONFIG_H

#include "main.h"

// 连接状态
typedef enum
{
    REMOTE_CONNECTED = 0, // 遥控器已连接
    REMOTE_DISCONNECTED,  // 遥控器已断开连接
} Remote_State_Enum;

// 飞行状态
typedef enum
{
    IDLE = 0,    // 空闲状态 | 锁定状态
    NORMAL,      // 正常飞行状态
    FIX_HEIGHT,  // 定高飞行状态
    RETURN_HOME, // 返航状态
    FAIL,        // 故障状态
} Flight_State_Enum;

// 解算得到的欧拉角
typedef struct
{
    float yaw;
    float pitch;
    float roll;
} Euler_Struct;

// 电池状态
typedef enum
{
    BAT_NORMAL = 0,
    BAT_LOW_3800,
    BAT_LOW_3650,
} BAT_State_Enum;

// 遥控器数据结构
typedef struct
{
    int16_t thr;         // 0~1000
    int16_t yaw;         // 0~1000
    int16_t pit;         // 0~1000
    int16_t rol;         // 0~1000
    uint8_t shutdown;    // 0: 不执行操作 1: 关机
    uint8_t lock;        // 0: 不执行操作 1: 锁定/解锁
    uint8_t return_home; // 0: 不执行操作 1: 返回/停止返回
    uint8_t fix_height;  // 0: 不执行操作 1: 进入定高模式/退出定高模式
} Remote_Data_Struct;

// 飞控数据结构
typedef struct
{
    Euler_Struct euler;             // 横滚角、俯仰角、偏航角
    uint16_t height;                // 所处高度
    uint16_t voltage;               // 飞机的电池电压
    BAT_State_Enum voltage_state;   // 电池电压状态
    Flight_State_Enum flight_state; // 当前飞行状态
} FLY_Z_Data_Struct;

#endif // COM_CONFIG_H
