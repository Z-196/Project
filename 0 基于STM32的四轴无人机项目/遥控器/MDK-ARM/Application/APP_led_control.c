#include "APP_led_control.h"

extern LED_Struct led1;
extern LED_Struct led2;
extern LED_Struct led3;
extern BUZZER_Struct buzzer;
extern FLY_Z_Data_Struct fly_z_data;   // 当前接收的飞控数据
extern Remote_State_Enum remote_state; // 连接状态
extern uint16_t battery_voltage;       // 遥控器的电池电压
extern BAT_State_Enum battery_state;   // 遥控器的电池电压状态
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
    if (fly_z_data.flight_state == IDLE)
    {
        // 空闲状态 => LED2慢闪 => 500ms亮，500ms灭
        if (0 == count % 5)
        {
            Int_LED_Toggle(&led2);
        }
    }
    else if (fly_z_data.flight_state == NORMAL)
    {
        // 正常飞行状态 => LED2快闪 => 200ms亮，200ms灭
        if (0 == count % 2)
        {
            Int_LED_Toggle(&led2);
        }
    }
    else if (fly_z_data.flight_state == FIX_HEIGHT)
    {
        // 定高飞行状态 => LED2常亮
        Int_LED_Turn_On(&led2);
    }
    else if (fly_z_data.flight_state == FAIL)
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
            battery_state = BAT_LOW_3800;
        }
    }
    else // 电池电压正常
    {
        Int_LED_Turn_Off(&led3);
        battery_state = BAT_NORMAL;
    }
}

/**
 * @brief 控制蜂鸣器
 */
void APP_buzzer(uint8_t count)
{
    if (fly_z_data.voltage_state == BAT_NORMAL)
    {
        // 飞机的电池电压正常 => 蜂鸣器不响
        Int_buzzer_Turn_Off(&buzzer);
    }
    else if (fly_z_data.voltage_state == BAT_LOW_3800)
    {
        // 飞机的电池电压低于3.8V => 蜂鸣器低频提醒 => 300ms响，300ms不响
        if (0 == count % 3)
        {
            Int_buzzer_Toggle(&buzzer);
        }
    }
    else if (fly_z_data.voltage_state == BAT_LOW_3650)
    {
        // 飞机的电池电压低于3.65V => 蜂鸣器常响报警
        Int_buzzer_Turn_On(&buzzer);
    }
}
