#include "Int_joystick.h"

uint16_t adc_buff[5] = {0};

/**
 * @brief 初始化ADC摇杆 打开ADC
 */
void Int_joystick_init(void)
{
    // 直接使用HAL中的函数，开启ADC
    // 16位数据和32位数据的地址值都是32位的
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff, 5);
}

/**
 * @brief 获取摇杆数据
 * 
 * @param joystick 摇杆数据结构体指针
 */
void Int_joystick_get(Joystick_Struct *joystick)
{
    // DMA不依赖于CPU的计算的 => 读取的数据是实时保存在ADC_BUFF中
    // 确保读取数据与实际值一致
    joystick->pit = adc_buff[0];
    joystick->rol = adc_buff[1];
    joystick->thr = adc_buff[2];
    joystick->yaw = adc_buff[3];
}
