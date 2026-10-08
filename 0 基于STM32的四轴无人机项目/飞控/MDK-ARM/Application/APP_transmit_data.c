#include "APP_transmit_data.h"

extern Flight_State_Enum flight_state;
extern uint16_t now_height;
extern Euler_Struct euler_angle;
extern uint16_t battery_voltage;
extern BAT_State_Enum battery_state;
extern FLY_Z_Data_Struct fly_z_data;

uint8_t tx_buff[TX_PLOAD_WIDTH] = {0};
/**
 * @brief 更新数据到结构体中
 */
void APP_update_fly_struct(void)
{
    // 1. 传入当前姿态角*10
    fly_z_data.euler = euler_angle;
    fly_z_data.euler.pitch *= 10;
    fly_z_data.euler.roll *= 10;
    fly_z_data.euler.yaw *= 10;

    // 2. 传入当前所处高度 => mm
    fly_z_data.height = now_height;

    // 3. 获取当前电池电压 => mv
    fly_z_data.voltage = battery_voltage;

    // 4. 获取当前电压状态
    fly_z_data.voltage_state = battery_state;

    // 5. 传入当前飞行状态
    fly_z_data.flight_state = flight_state;
}
extern uint8_t VL53L1X_Status;  
/**
 * @brief 将采集完成的飞控数据打包发送到遥控器
 *
 * @return uint8_t 0: 发送成功  1: 发送失败
 */
uint8_t APP_transmit_data(void)
{
    // 1. 更新数据到结构体中
    APP_update_fly_struct();

    // 2. 将数据打包到缓冲区中
    uint32_t sum = 0; // 声明校验和

    // 数据本身是12字节 => 帧头校验设置为3字节 => 校验和设置为4字节 => 总共20字节
    tx_buff[0] = FRAME_HEAD_CHECK1;
    tx_buff[1] = FRAME_HEAD_CHECK2;
    tx_buff[2] = FRAME_HEAD_CHECK3;

    // 定义为高8位在前
    tx_buff[3] = ((int16_t)fly_z_data.euler.pitch >> 8) & 0xFF;
    tx_buff[4] = (int16_t)fly_z_data.euler.pitch & 0xFF;
    tx_buff[5] = ((int16_t)fly_z_data.euler.roll >> 8) & 0xFF;
    tx_buff[6] = (int16_t)fly_z_data.euler.roll & 0xFF;
    tx_buff[7] = ((int16_t)fly_z_data.euler.yaw >> 8) & 0xFF;
    tx_buff[8] = (int16_t)fly_z_data.euler.yaw & 0xFF;
    tx_buff[9] = (fly_z_data.height >> 8) & 0xFF;
    tx_buff[10] = fly_z_data.height & 0xFF;
    tx_buff[11] = (fly_z_data.voltage >> 8) & 0xFF;
    tx_buff[12] = fly_z_data.voltage & 0xFF;
    tx_buff[13] = fly_z_data.voltage_state;
    tx_buff[14] = fly_z_data.flight_state;

    for (uint8_t i = 0; i < 15; i++)
    {
        sum += tx_buff[i];
    }

    // 高位在前 => 将校验和添加到缓冲区
    tx_buff[15] = (sum >> 24) & 0xFF;
    tx_buff[16] = (sum >> 16) & 0xFF;
    tx_buff[17] = (sum >> 8) & 0xFF;
    tx_buff[18] = sum & 0xFF;

    // 3. 切换到发送模式
    Int_SI24R1_TX_Mode();
    uint8_t res = 1;
    uint8_t count = 0;
    while (res == 1 && count < TRANSMIT_MAX_RETRY_TIMES) // 若发送失败，最多重试5次
    {
        count++;
        res = Int_SI24R1_TxPacket(tx_buff);
    }

    // 4. 切换到接收模式
    Int_SI24R1_RX_Mode();
    return res;
}
