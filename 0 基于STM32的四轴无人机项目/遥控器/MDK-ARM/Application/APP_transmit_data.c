#include "APP_transmit_data.h"

extern Remote_Data_Struct remote_data; // 存储当前发送的遥控数据
extern Remote_State_Enum remote_state; // 当前遥控连接状态
// 发送缓冲区
uint8_t transmit_buff[TX_PLOAD_WIDTH] = {0};
// 发送失败次数 > 最大发送重试次数 => 即为连接失败
uint8_t retry_count = 0;

/**
 * @brief 自动切换SI24R1的模式 => 将采集完成的遥控数据打包发送到飞机
 *
 * @return  0：发送成功 1：发送失败
 */
uint8_t APP_transmit_data(void)
{
    // 1. 打包发送数据 => 唯一性和可靠性
    // (1) => 帧头校验指定发送给对应的设备 => 唯一性
    // (2) => 发送数据的结尾 => 添加校验和 => 可靠性

    // 声明校验和
    uint32_t sum = 0;

    // 数据本身是12字节 => 帧头校验设置为3字节 => 校验和设置为4字节 => 总共20字节
    transmit_buff[0] = FRAME_HEAD_CHECK1;
    transmit_buff[1] = FRAME_HEAD_CHECK2;
    transmit_buff[2] = FRAME_HEAD_CHECK3;

    // 定义为高8位在前
    transmit_buff[3] = (remote_data.thr >> 8) & 0xFF;
    transmit_buff[4] = remote_data.thr & 0xFF;
    transmit_buff[5] = (remote_data.yaw >> 8) & 0xFF;
    transmit_buff[6] = remote_data.yaw & 0xFF;
    transmit_buff[7] = (remote_data.pit >> 8) & 0xFF;
    transmit_buff[8] = remote_data.pit & 0xFF;
    transmit_buff[9] = (remote_data.rol >> 8) & 0xFF;
    transmit_buff[10] = remote_data.rol & 0xFF;
    taskENTER_CRITICAL(); // 进入临界区
    transmit_buff[11] = remote_data.shutdown;
    transmit_buff[12] = remote_data.lock;
    transmit_buff[13] = remote_data.return_home;
    transmit_buff[14] = remote_data.fix_height;
    remote_data.shutdown = 0;
    remote_data.lock = 0;
    remote_data.return_home = 0;
    remote_data.fix_height = 0;
    taskEXIT_CRITICAL(); // 退出临界区

    for (uint8_t i = 0; i < 15; i++)
    {
        sum += transmit_buff[i];
    }

    // 高位在前 => 将校验和添加到缓冲区
    transmit_buff[15] = (sum >> 24) & 0xFF;
    transmit_buff[16] = (sum >> 16) & 0xFF;
    transmit_buff[17] = (sum >> 8) & 0xFF;
    transmit_buff[18] = sum & 0xFF;

    debug_printf(":%d,%d,%d,%d,%d,%d,%d,%d\r\n",
                 remote_data.thr, remote_data.yaw,
                 remote_data.pit, remote_data.rol,
                 remote_data.shutdown, remote_data.lock,
                 remote_data.return_home, remote_data.fix_height);

    // 2. 切换到发送模式
    Int_SI24R1_TX_Mode();
    uint8_t res = Int_SI24R1_TxPacket(transmit_buff);

    // 3. 切换回接收模式
    Int_SI24R1_RX_Mode();
    // 判断是否发送成功 => 发送成功则接收回传数据
    if (res == 0)
    {
        // 成功发送遥控数据 => 准备接收飞控回传的数据
        APP_receive_data();
    }
    return res;
}

/**
 * @brief 处理连接状态
 *
 * @param res 上一次发送数据的返回值
 */
void App_process_remote_state(uint8_t res)
{
    if (0 == res)
    {
        // 发送数据成功一次 => 即为连接成功
        remote_state = REMOTE_CONNECTED;
        retry_count = 0;
    }
    else
    {
        // 发送数据失败 => 超出最大重试次数 => 即为连接失败/断开连接
        retry_count++;
        if (retry_count >= TRANSMIT_MAX_RETRY_TIMES)
        {
            remote_state = REMOTE_DISCONNECTED;
            retry_count = 0;
        }
    }
}
