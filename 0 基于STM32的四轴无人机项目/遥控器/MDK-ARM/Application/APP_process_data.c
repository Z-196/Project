#include "APP_process_data.h"

extern Joystick_Struct joystick;       // 摇杆数据结构体
extern Remote_Data_Struct remote_data; // 存储当前发送的遥控数据
extern OLED_Page_Enum current_page;    // 当前页数

// 区分一下摇杆的控制值和按键的微调值
int16_t key_pit_offset = 0; // 往前飞是正
int16_t key_rol_offset = 0; // 往右飞是正

// 记录摇杆偏移量
int16_t joystick_thr_offset = 0;
int16_t joystick_yaw_offset = 0;
int16_t joystick_rol_offset = 0;
int16_t joystick_pit_offset = 0;

uint8_t fine_tuning_mode = 0; // 0: 退出微调模式 1: 进入微调模式

// 校准摇杆函数
void App_joystick_calibration(void)
{
    // 首先清空掉按键的微调值
    key_pit_offset = 0;
    key_rol_offset = 0;
    // 多次读取求平均值
    int16_t thr_sum = 0;
    int16_t yaw_sum = 0;
    int16_t rol_sum = 0;
    int16_t pit_sum = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        App_process_joystick_data();
        thr_sum += joystick.thr - 0;
        yaw_sum += joystick.yaw - 500;
        pit_sum += joystick.pit - 500;
        rol_sum += joystick.rol - 500;
        vTaskDelay(10);
    }

    // 零篇校准的偏移值没有累加的效果 => 会导致多次校准退回的效果
    joystick_thr_offset += thr_sum / 10;
    joystick_yaw_offset += yaw_sum / 10;
    joystick_rol_offset += rol_sum / 10;
    joystick_pit_offset += pit_sum / 10;
}

/**
 * @brief 遥感数据读取初始化
 */
void App_process_joystick_data_init(void)
{
    // 初始化摇杆ADC
    Int_joystick_init();
}

/**
 * @brief 处理摇杆数据
 *
 * @note => 两个任务会交替运行 => 使用临界区解决freeRTOS的任务切换问题
 */
void App_process_joystick_data(void)
{
    // 解决freeRTOS的任务切换问题 => 使用临界区解决
    taskENTER_CRITICAL();

    // 1. 获取摇杆监控的ADC值
    Int_joystick_get(&joystick);

    // 2. 处理范围和极性 => 将读取到的ADC值转换为有效范围
    joystick.thr = 1000 - ((int32_t)joystick.thr * 1000 / 4095.0f);
    joystick.yaw = 1000 - ((int32_t)joystick.yaw * 1000 / 4095.0f);
    joystick.rol = 1000 - ((int32_t)joystick.rol * 1000 / 4095.0f);
    joystick.pit = 1000 - ((int32_t)joystick.pit * 1000 / 4095.0f);

    // 3. 处理零偏校准
    joystick.thr -= joystick_thr_offset;
    joystick.yaw -= joystick_yaw_offset;
    joystick.rol -= joystick_rol_offset;
    joystick.pit -= joystick_pit_offset;

    // 4. 考虑按键的微调值
    joystick.pit += key_pit_offset;
    joystick.rol += key_rol_offset;

    // 4. 计算上偏移值可能超出范围 => 限制在0~1000
    joystick.thr = Com_limit(joystick.thr, 0, 1000);
    joystick.yaw = Com_limit(joystick.yaw, 0, 1000);
    joystick.rol = Com_limit(joystick.rol, 0, 1000);
    joystick.pit = Com_limit(joystick.pit, 0, 1000);

    // 退出临界区
    taskEXIT_CRITICAL();

    // 将处理完成的数据复制给遥控数据
    remote_data.thr = joystick.thr;
    remote_data.yaw = joystick.yaw;
    remote_data.rol = joystick.rol;
    remote_data.pit = joystick.pit;

    // debug_printf(":%d,%d,%d,%d\n", joystick.thr, joystick.yaw, joystick.rol, joystick.pit);
}

/**
 * @brief 处理按键数据 => 如果有按键按下，需要进行对应的记录 => 写入数据到遥控数据包结构体
 */
void App_process_key_data(void)
{
    Key_Type_Enum key = Int_key_get();
    // 根据按键的值进行记录 => 如果进行摇杆的校准 => 将按键的微调值调整为0
    if (key == KEY_1)
    {
        if (fine_tuning_mode)
        {
            // 向前飞微调 => 俯仰角+
            key_pit_offset += 10;
        }
        else
        {
            // 切换页面
            if (current_page == PAGE_1)
                current_page = PAGE_2;
            else if (current_page == PAGE_2)
                current_page = PAGE_3;
            else if (current_page == PAGE_2)
                current_page = PAGE_3;
            else if (current_page == PAGE_3)
                current_page = PAGE_4;
        }
    }
    else if (key == KEY_2)
    {
        if (fine_tuning_mode)
        {
            // 向后飞微调 => 俯仰角-
            key_pit_offset -= 10;
        }
        else
        {
            // 光标移动
        }
    }
    else if (key == KEY_3)
    {
        if (fine_tuning_mode)
        {
            // 向右飞微调 => 滚转角+
            key_rol_offset += 10;
        }
        else
        {
            // 数值增加/确认键
            if (current_page == PAGE_6)
            {
                current_page = PAGE_7;
                // 触发校准之后 => 摇杆的值 THR为0，YAW、PIT、ROL为500
                App_joystick_calibration();
            }
        }
    }
    else if (key == KEY_4)
    {
        if (fine_tuning_mode)
        {
            // 向左飞微调 => 滚转角-
            key_rol_offset -= 10;
        }
        else
        {
            // 数值减小/暂定键
        }
    }
    else if (key == KEY_5_SHORT)
    {
        // 上锁
        remote_data.lock = 1;
    }
    else if (key == KEY_5_LONG)
    {
        // 关机
        remote_data.shutdown = 1;
    }
    else if (key == KEY_6_SHORT)
    {
        // 返航功能 => 目前在硬件层不支持该功能
        remote_data.return_home = 1;
    }
    else if (key == KEY_6_LONG)
    {
        // 长按KEY_6 => 功能暂定
    }
    else if (key == KEY_7_SHORT)
    {
        // 进入/退出高度锁定模式
        remote_data.fix_height = 1;
    }
    else if (key == KEY_8_SHORT)
    {
        if (current_page == PAGE_5 || current_page == PAGE_6)
            current_page = PAGE_1;
        else if (current_page != PAGE_7)
            current_page = PAGE_6; // 进入摇杆校准页面 => Page_6 | Page_7
    }
    else if (key == KEY_8_LONG)
    {
        taskENTER_CRITICAL();                        // 进入临界区
        fine_tuning_mode = fine_tuning_mode ? 0 : 1; // 进入/退出微调模式
        taskEXIT_CRITICAL();                         // 退出临界区
        current_page = PAGE_5;
    }
}
