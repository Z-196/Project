#include "APP_led_control.h"

extern LED_Struct led1;
extern LED_Struct led2;
extern LED_Struct led3;
extern Remote_State_Enum remote_state;
extern Flight_State_Enum flight_state;
extern uint16_t battery_voltage;
extern BAT_State_Enum battery_state;

/**
 * @brief 控制LED1
 */
void APP_led1(uint8_t count)
{
    if (remote_state == REMOTE_DISCONNECTED)
    {
        // 断开连接 => LED1快闪 => 100ms亮，100ms灭
        Int_LED_Toggle(&led1);
    }
    else if (remote_state == REMOTE_CONNECTED)
    {
        // 已连接 => LED1常灭
        Int_LED_Turn_Off(&led1);
    }
}

/**
 * @brief 控制LED2
 */
void APP_led2(uint8_t count)
{
    if (flight_state == IDLE)
    {
        // 空闲状态 => LED2慢闪 => 500ms亮，500ms灭
        if (0 == count % 5)
        {
            Int_LED_Toggle(&led2);
        }
    }
    else if (flight_state == FIX_HEIGHT)
    {
        // 定高飞行状态 => LED2快闪 => 200ms亮，200ms灭
        if (0 == count % 2)
        {
            Int_LED_Toggle(&led2);
        }
    }
    else if (flight_state == NORMAL)
    {
        // 正常飞行状态 => LED2常亮
        Int_LED_Turn_On(&led2);
    }
    else if (flight_state == FAIL)
    {
        // 故障状态 => LED2常灭
        Int_LED_Turn_Off(&led2);
    }
}

/**
 * @brief 控制LED3
 */
void APP_led3(uint8_t count)
{
    battery_voltage = Int_bat_ADC_Read();

    if (battery_voltage < 3650)
    {
        // 电池电压低于3.65V => 接近过放（3.6V） => LED3快闪 => 100ms亮，100ms灭
        Int_LED_Toggle(&led3);
        battery_state = BAT_LOW_3650;
    }
    else if (battery_voltage < 3800)
    {
        // 电池电压低于3.8V => LED3慢闪 => 500ms亮，500ms灭
        if (0 == count % 5)
        {
            Int_LED_Toggle(&led3);
        }
        battery_state = BAT_LOW_3800;
    }
    else // 电池电压正常
    {
        Int_LED_Turn_Off(&led3);
        battery_state = BAT_NORMAL;
    }
}
