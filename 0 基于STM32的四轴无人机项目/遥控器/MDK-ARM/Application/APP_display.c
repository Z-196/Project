#include "APP_display.h"

extern Remote_Data_Struct remote_data; // 存储当前发送的遥控数据
extern FLY_Z_Data_Struct fly_z_data;   // 存储当前接收的飞控数据
extern Remote_State_Enum remote_state; // 当前遥控连接状态
extern uint16_t battery_voltage;       // 遥控器当前的电池电压
extern BAT_State_Enum battery_state;   // 遥控器当前的电池电压状态
extern int16_t key_pit_offset;         // 俯仰角微调值
extern int16_t key_rol_offset;         // 横滚角微调值
extern int16_t joystick_thr_offset;    // 记录摇杆偏移量
extern int16_t joystick_yaw_offset;
extern int16_t joystick_rol_offset;
extern int16_t joystick_pit_offset;

// 当前页数
OLED_Page_Enum current_page = PAGE_1;

/**
 * @brief 初始化显示模块
 */
void APP_display_init(void)
{
    /* 延时1000ms */
    vTaskDelay(1000);

    /* 初始化OLED */
    Int_OLED_Init();
}

/**
 * @brief 循环执行刷写屏幕
 */
void APP_display_show(void)
{
    // 1. 清空显存数据
    Int_OLED_Clear();

    // 2. 更新显存数据
    char buf[128];
    if (current_page == PAGE_1) // 页面1——>遥控器连接状态显示、ZJX、当前飞行状态、显示摇杆值显示
    {
        switch (remote_state)
        {
        case REMOTE_CONNECTED:
            Int_OLED_ShowString(0, 0, "Remote:OK", Int_OLED_8X16);
            break;
        case REMOTE_DISCONNECTED:
            Int_OLED_ShowString(0, 0, "Remote:ERR", Int_OLED_8X16);
            break;
        default:
            break;
        }
        Int_OLED_ShowString(104, 0, "ZJX", Int_OLED_8X16);

        switch (fly_z_data.flight_state)
        {
        case IDLE:
            Int_OLED_ShowString(0, 16, "Fly:IDLE", Int_OLED_8X16);
            break;
        case NORMAL:
            Int_OLED_ShowString(0, 16, "Fly:NORMAL", Int_OLED_8X16);
            break;
        case FIX_HEIGHT:
            Int_OLED_ShowString(0, 16, "Fly:FIX_HEIGHT", Int_OLED_8X16);
            break;
        case RETURN_HOME:
            Int_OLED_ShowString(0, 16, "Fly:RETURN_HOME", Int_OLED_8X16);
            break;
        case FAIL:
            Int_OLED_ShowString(0, 16, "Fly:FAIL", Int_OLED_8X16);
            break;
        default:
            break;
        }

        Int_OLED_ShowString(0, 36, "Thr:", Int_OLED_6X8);
        Int_OLED_DrawRectangle(24, 34, (uint8_t)(remote_data.thr / 100), 10, Int_OLED_FILLED);
        Int_OLED_ShowString(0, 52, "Yaw:", Int_OLED_6X8);
        Int_OLED_DrawRectangle(24, 50, (uint8_t)(remote_data.yaw / 100), 10, Int_OLED_FILLED);
        Int_OLED_ShowString(64, 36, "Pit:", Int_OLED_6X8);
        Int_OLED_DrawRectangle(88, 34, (uint8_t)(remote_data.pit / 100), 10, Int_OLED_FILLED);
        Int_OLED_ShowString(64, 52, "Rol:", Int_OLED_6X8);
        Int_OLED_DrawRectangle(88, 50, (uint8_t)(remote_data.rol / 100), 10, Int_OLED_FILLED);
    }
    else if (current_page == PAGE_2) // 页面2——>俯仰角；横滚角；偏航角；当前高度
    {
        sprintf(buf, "pitch:%.1f", fly_z_data.euler.pitch);
        Int_OLED_ShowString(0, 0, buf, Int_OLED_6X8);
        sprintf(buf, "roll:%.1f", fly_z_data.euler.roll);
        Int_OLED_ShowString(0, 16, buf, Int_OLED_6X8);
        sprintf(buf, "yaw:%.1f", fly_z_data.euler.yaw);
        Int_OLED_ShowString(0, 32, buf, Int_OLED_6X8);
        sprintf(buf, "height:%04umm", fly_z_data.height);
        Int_OLED_ShowString(0, 48, buf, Int_OLED_6X8);
    }
    else if (current_page == PAGE_3) // 页面3——>遥控器电池电压显示、飞机的电池电压、遥控器电池电压状态、飞机的电池电压状态
    {
        sprintf(buf, "Fly:%04dmv", fly_z_data.voltage);
        Int_OLED_ShowString(0, 0, buf, Int_OLED_8X16);
        switch (fly_z_data.voltage_state)
        {
        case BAT_NORMAL:
            Int_OLED_ShowString(0, 16, "State:NORMAL", Int_OLED_8X16);
            break;
        case BAT_LOW_3650:
            Int_OLED_ShowString(0, 16, "State:WARNING", Int_OLED_8X16);
            break;
        case BAT_LOW_3800:
            Int_OLED_ShowString(0, 16, "State:DANGER", Int_OLED_8X16);
            break;
        default:
            break;
        }

        sprintf(buf, "Remote:%04dmv", battery_voltage);
        Int_OLED_ShowString(0, 32, buf, Int_OLED_8X16);
        switch (battery_state)
        {
        case BAT_NORMAL:
            Int_OLED_ShowString(0, 48, "State:NORMAL", Int_OLED_8X16);
            break;
        case BAT_LOW_3650:
            Int_OLED_ShowString(0, 48, "State:WARNING", Int_OLED_8X16);
            break;
        case BAT_LOW_3800:
            Int_OLED_ShowString(0, 48, "State:DANGER", Int_OLED_8X16);
            break;
        default:
            break;
        }
    }
    else if (current_page == PAGE_4) // 页面4——>查看三个指示灯、蜂鸣器状态显示
    {
        switch (remote_state)
        {
        case REMOTE_CONNECTED:
            Int_OLED_ShowString(0, 0, "LED1:CONNECTED", Int_OLED_8X16);
            break;
        case REMOTE_DISCONNECTED:
            Int_OLED_ShowString(0, 0, "LED1:DISCONNECT", Int_OLED_8X16);
            break;
        default:
            break;
        }

        switch (fly_z_data.flight_state)
        {
        case IDLE:
            Int_OLED_ShowString(0, 16, "LED2:IDLE", Int_OLED_8X16);
            break;
        case NORMAL:
            Int_OLED_ShowString(0, 16, "LED2:NORMAL", Int_OLED_8X16);
            break;
        case FIX_HEIGHT:
            Int_OLED_ShowString(0, 16, "LED2:FIX_HEIGHT", Int_OLED_8X16);
            break;
        case RETURN_HOME:
            Int_OLED_ShowString(0, 16, "LED2:RETURN_HOME", Int_OLED_8X16);
            break;
        case FAIL:
            Int_OLED_ShowString(0, 16, "LED2:FAIL", Int_OLED_8X16);
            break;
        default:
            break;
        }

        switch (battery_state)
        {
        case BAT_NORMAL:
            Int_OLED_ShowString(0, 32, "LED3:NORMAL", Int_OLED_8X16);
            break;
        case BAT_LOW_3650:
            Int_OLED_ShowString(0, 32, "LED3:WARNING", Int_OLED_8X16);
            break;
        case BAT_LOW_3800:
            Int_OLED_ShowString(0, 32, "LED3:DANGER", Int_OLED_8X16);
            break;
        default:
            break;
        }

        switch (fly_z_data.voltage_state)
        {
        case BAT_NORMAL:
            Int_OLED_ShowString(0, 48, "BUZEER:NORMAL", Int_OLED_8X16);
            break;
        case BAT_LOW_3650:
            Int_OLED_ShowString(0, 48, "BUZEER:WARNING", Int_OLED_8X16);
            break;
        case BAT_LOW_3800:
            Int_OLED_ShowString(0, 48, "BUZEER:DANGER", Int_OLED_8X16);
            break;
        default:
            break;
        }
    }
    else if (current_page == PAGE_5) // 页面5——>微调模式页面
    {
        sprintf(buf, "Joy_pit:%04d", remote_data.pit);
        Int_OLED_ShowString(0, 0, buf, Int_OLED_8X16);
        sprintf(buf, "Key_pit:%04d", key_pit_offset);
        Int_OLED_ShowString(0, 16, buf, Int_OLED_8X16);
        sprintf(buf, "Joy_rol:%04d", remote_data.rol);
        Int_OLED_ShowString(0, 32, buf, Int_OLED_8X16);
        sprintf(buf, "Key_rol:%04d", key_rol_offset);
        Int_OLED_ShowString(0, 48, buf, Int_OLED_8X16);
    }
    else if (current_page == PAGE_6) // 页面6——>摇杆校准页面 => 按下按键3开始校准 => 跳转到页面7
    {
        sprintf(buf, "Thr:%4d/0   %03d", remote_data.thr, joystick_thr_offset);
        Int_OLED_ShowString(0, 0, buf, Int_OLED_8X16);
        sprintf(buf, "Yaw:%4d/500 %03d", remote_data.yaw, joystick_yaw_offset);
        Int_OLED_ShowString(0, 16, buf, Int_OLED_8X16);
        sprintf(buf, "Pit:%4d/500 %03d", remote_data.pit, joystick_pit_offset);
        Int_OLED_ShowString(0, 32, buf, Int_OLED_8X16);
        sprintf(buf, "Rol:%4d/500 %03d", remote_data.rol, joystick_rol_offset);
        Int_OLED_ShowString(0, 48, buf, Int_OLED_8X16);
    }
    else if (current_page == PAGE_7) // 页面7——> 校准中
    {
        Int_OLED_ShowString(8, 20, "calibrating...", Int_OLED_8X16);
        static uint8_t count = 0;
        count++;
        if (count >= 15)
        {
            current_page = PAGE_6;
            count = 0;
        }
    }
    // 3. 刷新屏幕
    Int_OLED_Update();
}
