#ifndef APP_TRANSMIT_DATA_H
#define APP_TRANSMIT_DATA_H

#include "Com_config.h"
#include "Int_SI24R1.h"
#include "Int_bat_ADC.h"

// 定义帧头校验的值
#define FRAME_HEAD_CHECK1 'z'
#define FRAME_HEAD_CHECK2 'j'
#define FRAME_HEAD_CHECK3 'x'

// 最大发送重试次数
#define TRANSMIT_MAX_RETRY_TIMES 5

/**
 * @brief 更新数据到结构体中
 */
void APP_update_fly_struct(void);

/**
 * @brief 自动切换SI24R1的模式 => 将采集完成的飞控数据打包发送到遥控器
 */
uint8_t APP_transmit_data(void);

#endif // APP_TRANSMIT_DATA_H
