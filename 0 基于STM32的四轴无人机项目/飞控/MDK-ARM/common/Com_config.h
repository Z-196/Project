#ifndef COM_CONFIG_H // 存储基础配置信息
#define COM_CONFIG_H

#include "main.h"

/*（Z轴正方向朝上）
X轴方向为机头方向 => GYRO_Y轴对应俯仰角，GYRO_X轴对应横滚角 => 1
Y轴方向为机头方向 => GYRO_X轴对应俯仰角，GYRO_Y轴对应横滚角 => 0*/
#define MPU_SELECT_DIR 0
/* 电机转速最大值 */
#define MOTOR_MAX_SPEED 700

// 连接状态
typedef enum
{
    REMOTE_CONNECTED = 0, // 遥控器已连接
    REMOTE_DISCONNECTED,  // 遥控器已断开连接
} Remote_State_Enum;

// 飞行状态
typedef enum
{
    IDLE = 0,       // 空闲状态 | 锁定状态
    NORMAL,         // 正常飞行状态
    FIX_HEIGHT,     // 定高飞行状态
    HEIGHT_CONTROL, // 高度调制模式
    RETURN_HOME,    // 返航状态
    FAIL,           // 故障状态
} Flight_State_Enum;

// 电池状态
typedef enum
{
    BAT_NORMAL = 0, // 电池电压正常
    BAT_LOW_3800,   // 电压低于3.8V
    BAT_LOW_3650,   // 电压低于3.65V
} BAT_State_Enum;

// 油门解锁状态
typedef enum
{
    FREE = 0,
    MAX,
    LEAVE_MAX,
    MIN,
    UNLOCK,
} Thr_state_enum;

// 陀螺仪数据
typedef struct
{
    int16_t gyro_x; // 向后飞为正，表示俯仰角
    int16_t gyro_y; // 向右飞为正，表示横滚角
    int16_t gyro_z; // 逆时针转动为正，表示偏航角
} Gyro_Struct;

typedef struct
{
    int16_t accel_x; // 往右为正
    int16_t accel_y; // 往前为正，Y轴正方向为机头方向 => 按需修改四元数姿态解算函数
    int16_t accel_z; // 朝上的加速度为正，芯片朝上放置时，默认为+1g
} Accel_Struct;

typedef struct
{
    Gyro_Struct gyro;
    Accel_Struct accel;
} Gyro_Accel_Struct;

// 解算得到的欧拉角
typedef struct
{
    float yaw;
    float pitch;
    float roll;
} Euler_Struct;

// 遥控器数据结构
typedef struct
{
    int16_t thr;
    int16_t yaw;
    int16_t pit;
    int16_t rol;
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
