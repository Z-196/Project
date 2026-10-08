#ifndef APP_LED_CONTROL_H
#define APP_LED_CONTROL_H

#include "Com_config.h"
#include "Int_LED.h"
#include "Int_buzzer.h"
#include "Int_bat_ADC.h"

/**
 * @brief 控制LED1
 */
void APP_led1(uint8_t count);

/**
 * @brief 控制LED2
 */
void APP_led2(uint8_t count);

/**
 * @brief 控制LED3
 */
void APP_led3(uint8_t count);

/**
 * @brief 控制蜂鸣器
 */
void APP_buzzer(uint8_t count);

#endif //APP_LED_CONTROL_H
