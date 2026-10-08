#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

// PA7 = TIM3_CH2
#define LED_GPIO_PIN        GPIO_Pin_7
#define LED_GPIO_PORT       GPIOA
#define LED_TIM             TIM3

void LED_ON(void);
void LED_OFF(void);
void LED_Turn(void);
void LED_SetBrightness(uint16_t brightness);  // ÁÁ¶È¿ØÖÆº¯Êý

#endif
