#include "APP_FreeRTOS_Task.h"

// STM32F103C8T6 => SRAM为20KB => 分配12KB给操作系统，剩余8KB给用户使用

// LED
LED_Struct led1 = {.port = LED1_GPIO_Port, .pin = LED1_Pin};
LED_Struct led2 = {.port = LED2_GPIO_Port, .pin = LED2_Pin};
LED_Struct led3 = {.port = LED3_GPIO_Port, .pin = LED3_Pin};
// BUZZER
BUZZER_Struct buzzer = {.port = BUZZER_GPIO_Port, .pin = BUZZER_Pin};
// 遥控器当前的电池电压
uint16_t battery_voltage = 0;
// 遥控器当前的电池电压状态
BAT_State_Enum battery_state = BAT_NORMAL;

// 摇杆数据结构体
Joystick_Struct joystick = {0};
// 表示当前连接状态
Remote_State_Enum remote_state = REMOTE_CONNECTED;

// 存储当前发送的遥控数据
Remote_Data_Struct remote_data = {.thr = 0, .yaw = 500, .pit = 500, .rol = 500, .shutdown = 0, .lock = 0, .return_home = 0, .fix_height = 0};
// 存储当前接收的飞控数据
FLY_Z_Data_Struct fly_z_data = {0};

// 电源任务
void power_task(void *args);
#define POWER_TASK_STACK_SIZE 128   // 最小推荐值填写128 => 128*4 = 512字节
#define POWER_TASK_PRIORITY 4       // 任务优先级 => 数值越大，优先级越高 => 最大4 => 不推荐使用最小优先级
TaskHandle_t power_task_handle;     // 任务句柄
#define POWER_TASK_PERIOD 14 * 1000 // 任务周期 => 每14s执行一次

// 通讯任务
void com_task(void *args);
#define COM_TASK_STACK_SIZE 128
#define COM_TASK_PRIORITY 3
TaskHandle_t com_task_handle;
#define COM_TASK_PERIOD 10

// 按键任务
void key_task(void *args);
#define KEY_TASK_STACK_SIZE 128
#define KEY_TASK_PRIORITY 2
TaskHandle_t key_task_handle;
#define KEY_TASK_PERIOD 20

// 摇杆任务
void joystick_task(void *args);
#define JOYSTICK_TASK_STACK_SIZE 128
#define JOYSTICK_TASK_PRIORITY 2
TaskHandle_t joystick_task_handle;
#define JOYSTICK_TASK_PERIOD 20

// 屏幕任务
void oled_task(void *args);
#define OLED_TASK_STACK_SIZE 128
#define OLED_TASK_PRIORITY 2
TaskHandle_t oled_task_handle;
#define OLED_TASK_PERIOD 100

// LED任务 => 控制LED、蜂鸣器，读取电池电压
void led_task(void *args);
#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY 1
TaskHandle_t led_task_handle;
#define LED_TASK_PERIOD 100

/**
 * @brief 启动FreeRTOS操作系统
 */
void APP_FreeRTOS_start(void)
{
    // 1. 创建电源管理任务
    xTaskCreate(power_task, "PowerTask", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_task_handle);

    // 2. 创建通讯任务
    xTaskCreate(com_task, "ComTask", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &com_task_handle);

    // 3. 创建按键任务
    xTaskCreate(key_task, "KeyTask", KEY_TASK_STACK_SIZE, NULL, KEY_TASK_PRIORITY, &key_task_handle);

    // 4. 创建摇杆任务
    xTaskCreate(joystick_task, "JoystickTask", JOYSTICK_TASK_STACK_SIZE, NULL, JOYSTICK_TASK_PRIORITY, &joystick_task_handle);

    // 5. 创建声光任务
    xTaskCreate(led_task, "LedTask", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, &led_task_handle);

    // 6. 创建屏幕任务
    xTaskCreate(oled_task, "OledTask", OLED_TASK_STACK_SIZE, NULL, OLED_TASK_PRIORITY, &oled_task_handle);

    // 7. 启动任务调度器
    vTaskStartScheduler();
}

void power_task(void *args)
{
    // 获取当前的基准时间
    TickType_t startTime = xTaskGetTickCount();
    while (1)
    {
        // 每14s执行一次 => 启动电源，避免自动关机
        vTaskDelayUntil(&startTime, POWER_TASK_PERIOD);

        // 启动电源
        Int_TP4366_start();
    }
}

void key_task(void *args)
{
    TickType_t startTime = xTaskGetTickCount();
    while (1)
    {
        // 统一的处理方式
        App_process_key_data();
        vTaskDelayUntil(&startTime, KEY_TASK_PERIOD);
    }
}

void joystick_task(void *args)
{
    TickType_t startTime = xTaskGetTickCount();
    App_process_joystick_data_init();
    while (1)
    {
        // 统一的处理方式
        App_process_joystick_data();
        vTaskDelayUntil(&startTime, JOYSTICK_TASK_PERIOD);
    }
}

void com_task(void *args)
{
    while (1)
    {
        // 将打包完成的数据发送到飞控
        uint8_t res = APP_transmit_data();

        // 判断遥控状态
        App_process_remote_state(res);

        // vTaskDelayUntil()算上函数运行的时间，总计延时6ms
        // 为保证两端的收发保持一致 => 在第一次匹配上完成收发之后
        // => 两端两端一起延时一个时间，再次收发
        vTaskDelay(COM_TASK_PERIOD);
    }
}

void led_task(void *args)
{
    // 获取当前的基准时间
    TickType_t startTime = xTaskGetTickCount();
    // 初始化电池电压测量
    Int_bat_ADC_Init();
    uint8_t count = 0;
    while (1)
    {
        // 1.判断当前连接状态  LED1 => 连接状态
        APP_led1(count);

        // 2.判断当前飞行状态 LED2 => 飞行状态
        APP_led2(count);

        // 3.判断当前电池状态 LED3 => 电池状态
        APP_led3(count);

        // 4.判断飞机电池的状态 BUZZER => 飞机电池的状态
        APP_buzzer(count);

        count = (count + 1) % 30;
        vTaskDelayUntil(&startTime, LED_TASK_PERIOD);
    }
}

void oled_task(void *args)
{
    // 获取当前的基准时间
    TickType_t startTime = xTaskGetTickCount();
    APP_display_init();
    while (1)
    {
        APP_display_show();

        vTaskDelayUntil(&startTime, OLED_TASK_PERIOD);
    }
}
