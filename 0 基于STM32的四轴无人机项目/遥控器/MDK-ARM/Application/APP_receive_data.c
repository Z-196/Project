#include "APP_receive_data.h"

extern FLY_Z_Data_Struct fly_z_data; // 存储当前接收的飞控数据

// 接收缓冲区
uint8_t receive_buff[TX_PLOAD_WIDTH] = {0};

/**
 * @brief 接收飞控发送的数据
 *
 * @return 0: 校验通过 1: 没收到数据/校验失败
 */
uint8_t APP_receive_data(void)
{
    // 1. 清空接收缓冲区
    memset(receive_buff, 0, TX_PLOAD_WIDTH);

    // 2. 接收飞控数据包
    uint8_t count = 0;
    uint8_t res = 1;
    while (res == 1 && count < RECEIVE_MAX_RETRY_TIMES) // 若接收失败，最多重试5次
    {
        count++;
        res = Int_SI24R1_RxPacket(receive_buff);
    }
    if (res == 1 || count >= 5)
    {
        return 1; // 没收到飞控回传的数据
    }
    if (0 == strlen((char *)receive_buff))
    {
        return 1; // 接收到的数据包为空包
    }

    // 3. 帧头校验
    if (receive_buff[0] != FRAME_HEAD_CHECK1 ||
        receive_buff[1] != FRAME_HEAD_CHECK2 ||
        receive_buff[2] != FRAME_HEAD_CHECK3)
    {
        return 1;
    }

    // 4. 帧尾校验
    uint32_t sum = 0;
    uint32_t sum_receive = 0; // 高位在前
    for (uint8_t i = 0; i < (TX_PLOAD_WIDTH - 4); i++)
    {
        sum += receive_buff[i];
    }
    sum_receive = (receive_buff[TX_PLOAD_WIDTH - 4] << 24) |
                  (receive_buff[TX_PLOAD_WIDTH - 3] << 16) |
                  (receive_buff[TX_PLOAD_WIDTH - 2] << 8) |
                  receive_buff[TX_PLOAD_WIDTH - 1];
    if (sum != sum_receive)
    {
        return 1;
    }

    // 5. 保存数据
    // 高8位在前
    fly_z_data.euler.pitch = (int16_t)((receive_buff[3] << 8) | receive_buff[4]);
    fly_z_data.euler.roll = (int16_t)((receive_buff[5] << 8) | receive_buff[6]);
    fly_z_data.euler.yaw = (int16_t)((receive_buff[7] << 8) | receive_buff[8]);
    fly_z_data.height = (receive_buff[9] << 8) | receive_buff[10];
    fly_z_data.voltage = (receive_buff[11] << 8) | receive_buff[12];
    fly_z_data.voltage_state = (BAT_State_Enum)receive_buff[13];
    fly_z_data.flight_state = (Flight_State_Enum)receive_buff[14];

    fly_z_data.euler.pitch  /= 10;  
    fly_z_data.euler.roll /= 10;
    fly_z_data.euler.yaw /= 10;

    return 0;
}
