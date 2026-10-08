#include "APP_FreeRTOS_Task.h"

// STM32F103C8T6 => SRAM为20KB => 分配12KB给操作系统，剩余8KB给用户使用
// 内存管理 => C语言中的结构体通常保存在堆中，不会自动垃圾回收 => 始终使用使用同一个结构体，不断循环使用

// LED
LED_Struct led1 = {.port = LED1_GPIO_Port, .pin = LED1_Pin};
LED_Struct led2 = {.port = LED2_GPIO_Port, .pin = LED2_Pin};
LED_Struct led3 = {.port = LED3_GPIO_Port, .pin = LED3_Pin};
// 表示当前连接状态
Remote_State_Enum remote_state = REMOTE_DISCONNECTED;
// 表示当前的飞行状态
Flight_State_Enum flight_state = IDLE;
// 进入定高模式时，飞机所处的高度
uint16_t fix_height;
// 进入高度调制模式时，飞机的初始高度
uint16_t height_control;
// 飞机目前所处的高度
uint16_t now_height;
// 当前的电池电压
uint16_t battery_voltage;
// 当前的电池状态
BAT_State_Enum battery_state = BAT_NORMAL;

// 存储当前接收的遥控数据
Remote_Data_Struct remote_data = {.thr = 0, .yaw = 500, .pit = 500, .rol = 500, .shutdown = 0, .lock = 0, .return_home = 0, .fix_height = 0};
// 存储当前发送的飞控数据
FLY_Z_Data_Struct fly_z_data;

// 电源管理任务
void power_task(void *args);
#define POWER_TASK_STACK_SIZE 128 // 最小推荐值填写128 => 128*4 = 512字节
#define POWER_TASK_PRIORITY 4     // 任务优先级 => 数值越大，优先级越高 => 最大4 => 不推荐使用最小优先级
TaskHandle_t power_Task_Handle;   // 任务句柄
#define POWER_TASK_PERIOD 14000   // 定义任务的周期(ms)

// 飞行控制任务
void fight_task(void *args);
#define FIGHT_TASK_STACK_SIZE 256
#define FIGHT_TASK_PRIORITY 3
TaskHandle_t fight_Task_Handle;
#define FIGHT_TASK_PERIOD 6

// 高度调制任务
void height_task(void *args);
#define HEIGHT_TASK_STACK_SIZE 128
#define HEIGHT_TASK_PRIORITY 2
TaskHandle_t height_Task_Handle;
#define HEIGHT_TASK_PERIOD 30

// LED任务 => 控制LED，读取电池电压
void led_task(void *args);
#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY 1
TaskHandle_t led_Task_Handle;
#define LED_TASK_PERIOD 100

// 通讯任务
void com_task(void *args);
#define COM_TASK_STACK_SIZE 256
#define COM_TASK_PRIORITY 4
TaskHandle_t com_task_handle;
#define COM_TASK_PERIOD 10

/**
 * @brief 启动FreeRTOS操作系统
 */
void APP_FreeRTOS_start(void)
{
    // 1.创建任务
    xTaskCreate(power_task, "PowerTask", POWER_TASK_STACK_SIZE, NULL, POWER_TASK_PRIORITY, &power_Task_Handle);

    // 2.创建飞行控制任务
    xTaskCreate(fight_task, "FightTask", FIGHT_TASK_STACK_SIZE, NULL, FIGHT_TASK_PRIORITY, &fight_Task_Handle);

    // 3.创建定高任务
    xTaskCreate(height_task, "HeightTask", HEIGHT_TASK_STACK_SIZE, NULL, HEIGHT_TASK_PRIORITY, &height_Task_Handle);

    // 4.创建LED任务
    xTaskCreate(led_task, "LEDTask", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, &led_Task_Handle);

    // 5.创建通讯任务
    xTaskCreate(com_task, "ComTask", COM_TASK_STACK_SIZE, NULL, COM_TASK_PRIORITY, &com_task_handle);

    // 6.启动调度器
    vTaskStartScheduler();
}

void power_task(void *args)
{
    // 获取当前的基准时间
    TickType_t startTime = xTaskGetTickCount();
    while (1)
    {
        // // 每14s执行一次 => 启动电源，避免自动关机
        // vTaskDelayUntil(&startTime, POWER_TASK_PERIOD);

        // // 启动电源
        // Int_TP4366_start();

        // 使用直接任务通知的接收方式 => 实现14s调用一次
        // 一直等任务通知，直到收到通知 res=1 或是 超时 res=0
        uint32_t res = ulTaskNotifyTake(pdTRUE, POWER_TASK_PERIOD);
        if (res != 0)
        {
            // 收到通知 => 关机
            Int_TP4366_shutdown();
        }
        else
        {
            // 未收到通知 => 正常执行启动电源
            Int_TP4366_start();
        }
    }
}

void fight_task(void *args)
{
    // 获取当前的基准时间
    TickType_t startTime = xTaskGetTickCount();
    // 1. 飞控任务初始化
    App_flight_init();
    while (1)
    {
        // 2. 获取MPU6050测量的数据 => 姿态解算得到欧拉角
        App_flight_get_euler();

        // 3. 根据当前的欧拉角 => 进行PID计算
        App_flight_pid_process();

        // 4. 根据PID计算的结果 => 对电机进行控制
        App_flight_control_motor();

        vTaskDelayUntil(&startTime, FIGHT_TASK_PERIOD);
    }
}

void height_task(void *args)
{
    // 获取当前的基准时间
    TickType_t startTime = xTaskGetTickCount();
    while (1)
    {
        // 获取当前高度 => 进行PID计算
        // 激光测距仪的数据20ms采集一次 => 30ms查询处理一次
        App_flight_fix_height_process();

        vTaskDelayUntil(&startTime, HEIGHT_TASK_PERIOD);
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

        count = (count + 1) % 10;
        vTaskDelayUntil(&startTime, LED_TASK_PERIOD);
    }
}

void com_task(void *args)
{
    Int_SI24R1_Init(); // SI24R1初始化
    while (1)
    {
        // 1. 接收遥控器数据，回传数据
        uint8_t res = APP_receive_data();

        // 2. 根据接收数据的返回值，处理当前飞控的连接状态
        App_process_remote_state(res);

        // 3. 处理关机命令
        if (remote_data.shutdown == 1)
        {
            // 使用freeRTOS直接任务通知 => 通知电源任务 => 执行关机
            xTaskNotifyGive(power_Task_Handle);
        }

        // 4. 处理飞机的飞行状态 => 遇到故障状态（失联） => 会一直等待任务通知
        App_process_flight_state();

        // 6ms执行一次，接收数据的时间间隔应该等于发送数据的时间间隔
        // vTaskDelayUntil()算上函数运行的时间，总计延时6ms
        // 为保证两端的收发保持一致 => 在第一次匹配上完成收发之后
        // => 两端两端一起延时一个时间，再次收发
        vTaskDelay(COM_TASK_PERIOD);
    }
}
