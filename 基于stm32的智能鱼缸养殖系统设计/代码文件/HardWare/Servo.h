#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f10x.h"

// ===================== 改为 PB0 + TIM3_CH3 =====================
#define SERVO_TIM          TIM3                // 定时器 TIM3
#define SERVO_CHANNEL      TIM_Channel_3       // 通道3
#define SERVO_GPIO_PORT    GPIOB               // 端口 GPIOB
#define SERVO_GPIO_PIN     GPIO_Pin_0          // 引脚 PB0
#define SERVO_TIM_CLK      RCC_APB1Periph_TIM3 // TIM3时钟
#define SERVO_GPIO_CLK     RCC_APB2Periph_GPIOB// GPIOB时钟

// 舵机角度-CCR值映射（不变）
#define SERVO_0_DEG        500     // 0°
#define SERVO_90_DEG       1500    // 90°
#define SERVO_180_DEG      2500    // 180°
#define SERVO_MIN_CCR      400     // 最小保护
#define SERVO_MAX_CCR      2600     // 最大保护

// 函数声明
void Servo_Init(void);
void Servo_SetAngle(int16_t angle);
void Servo_SetCCR(uint16_t ccr);

#endif
