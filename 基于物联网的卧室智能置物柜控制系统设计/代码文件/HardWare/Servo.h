#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f10x.h"

// 舵机核心参数宏定义（可根据实际舵机微调）
#define SERVO_TIM          TIM3                // 舵机使用的定时器（PB6对应TIM4_CH1）
#define SERVO_CHANNEL      TIM_Channel_1       // 定时器通道
#define SERVO_GPIO_PORT    GPIOA               // 舵机控制引脚端口
#define SERVO_GPIO_PIN     GPIO_Pin_6          // 舵机控制引脚（PB6）
#define SERVO_TIM_CLK      RCC_APB1Periph_TIM3 // 定时器时钟（APB1总线）
#define SERVO_GPIO_CLK     RCC_APB2Periph_GPIOA// GPIO时钟（APB2总线）

// 舵机角度-CCR值映射（50Hz PWM：周期20ms，1μs计数精度）
#define SERVO_0_DEG        500     // 0° → 0.5ms高电平（1000×1μs）
#define SERVO_90_DEG       1500    // 90° → 1.5ms高电平
#define SERVO_180_DEG      2500    // 180° → 2.5ms高电平
#define SERVO_MIN_CCR      400     // 最小CCR（防止舵机卡死）
#define SERVO_MAX_CCR      2600    // 最大CCR（防止舵机过载）

// 函数声明
void Servo_Init(void);                // 舵机初始化（TIM4+PB6配置）
void Servo_SetAngle(int16_t angle);   // 舵机角度控制（0°~180°）
void Servo_SetCCR(uint16_t ccr);      // 直接设置CCR值（底层精准控制）

#endif

