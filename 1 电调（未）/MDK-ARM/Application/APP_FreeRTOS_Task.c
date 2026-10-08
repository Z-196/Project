#include "APP_FreeRTOS_Task.h"

// STM32F103C8T6 => SRAM为20KB => 分配12KB给操作系统，剩余8KB给用户使用
// 内存管理 => C语言中的结构体通常保存在堆中，不会自动垃圾回收 => 始终使用使用同一个结构体，不断循环使用

// LED
LED_Struct led = {.port = LED_GPIO_Port, .pin = LED_Pin};
// 采样电阻结构体
Res_Sampling_Struct res_sampling = {0};
// MOTOR
Motor_Struct motor_U = {.htim = &htim1, .Channel = TIM_CHANNEL_1};
Motor_Struct motor_V = {.htim = &htim1, .Channel = TIM_CHANNEL_2};
Motor_Struct motor_W = {.htim = &htim1, .Channel = TIM_CHANNEL_3};
// 电机运行阶段
Motor_Run_Stage motor_stage = MOTOR_STAGE_POS; // 预定位
// PWM占空比，0~7200
uint16_t motor_duty = 0;
// 目标转速
uint16_t target_rpm = 0;
// 当前转速
uint16_t now_rpm = 0;
// MOTOR_PID
PID_Struct motor_pid = {.kp = 3.2f, .ki = 0.0f, .kd = 0.43f};

// LED任务
void led_task(void *args);
#define LED_TASK_STACK_SIZE 128
#define LED_TASK_PRIORITY 1
TaskHandle_t led_Task_Handle;
#define LED_TASK_PERIOD 100

// MOTOR任务
void motor_task(void *args);
#define MOTOR_TASK_STACK_SIZE 128
#define MOTOR_TASK_PRIORITY 4
TaskHandle_t motor_Task_Handle;
#define MOTOR_TASK_PERIOD 2

// 数据接收任务
void receive_task(void *args);
#define RECEIVE_TASK_STACK_SIZE 128
#define RECEIVE_TASK_PRIORITY 3
TaskHandle_t receive_Task_Handle;
#define RECEIVE_TASK_PERIOD 10

/**
 * @brief 启动FreeRTOS操作系统
 */
void APP_FreeRTOS_start(void)
{
    // 1.创建LED任务
    xTaskCreate(led_task, "LEDTask", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, &led_Task_Handle);

    // 2.创建MOTOR任务
    xTaskCreate(motor_task, "MOTORTask", MOTOR_TASK_STACK_SIZE, NULL, MOTOR_TASK_PRIORITY, &motor_Task_Handle);

    // 3.创建UART任务
    xTaskCreate(receive_task, "UARTTask", RECEIVE_TASK_STACK_SIZE, NULL, RECEIVE_TASK_PRIORITY, &receive_Task_Handle);

    // 3.启动调度器
    vTaskStartScheduler();
}
void led_task(void *args)
{
    // 获取当前的基准时间
    TickType_t startTime = xTaskGetTickCount();
    uint8_t count = 0;
    while (1)
    {
        if (count == 0)
        {
            Int_LED_Toggle(&led);
        }
        count = (count + 1) % 5;
        vTaskDelayUntil(&startTime, LED_TASK_PERIOD);
    }
}

void motor_task(void *args)
{
    uint8_t cur_sector = 0;                        // 当前扇区 0~5
    uint32_t zero_cross_tick = 0;                  // 过零时刻
    uint32_t openloop_delay = 20;                  // 开环换相间隔(ms)
    uint32_t now_tick = 0;                         // 当前时间戳
    uint32_t last_comm_tick = xTaskGetTickCount(); // 上一次换相时刻
    APP_motor_Init();

    while (1)
    {
        // 1.更新ADC采样值，得到悬空相电压
        uint16_t bemf = APP_Motor_GetRes_Sampling(cur_sector, &res_sampling);
        uint16_t mid = res_sampling.mid;

        // 2. 状态切换
        if (motor_stage == MOTOR_STAGE_POS) // 预定位
        {
            now_tick = xTaskGetTickCount();
            APP_Motor_SixStep_Update(0, motor_duty); // 预定位

            if (now_tick - last_comm_tick >= 150)
            {
                // 达到150ms定位，切换到开环加速
                motor_stage = MOTOR_STAGE_OPENLOOP;
                last_comm_tick = xTaskGetTickCount();
            }
        }
        else if (motor_stage == MOTOR_STAGE_OPENLOOP) // 开环加速
        {
            now_tick = xTaskGetTickCount();
            // 达到开环换相间隔，执行换相
            if (now_tick - last_comm_tick >= openloop_delay)
            {
                cur_sector = (cur_sector + 1) % 6;
                APP_Motor_SixStep_Update(cur_sector, 3600);
                last_comm_tick = xTaskGetTickCount(); // 记录本次换相时间戳

                // 判断BEMF幅值是否足够，满足则切闭环
                if (abs((int16_t)bemf - (int16_t)mid) > 60)
                {
                    motor_stage = MOTOR_STAGE_CLOSELOOP;
                }
                // 逐步缩小开环间隔，提升转速(最低限制5ms)
                else if (openloop_delay > 5)
                {
                    openloop_delay -= 1;
                }
            }
        }
        else if (motor_stage == MOTOR_STAGE_CLOSELOOP) // 闭环BEMF换相
        {
            now_tick = xTaskGetTickCount();
            uint8_t bemf_state = 1;

            // 带滞环的状态判断，杜绝噪声抖动
            if (bemf > mid + 8)
                bemf_state = 1;
            else if (bemf < mid - 8)
                bemf_state = 0;

            // 检测到过零点：1 → 0 翻转
            if (0 == bemf_state)
            {
                zero_cross_tick = now_tick;
            }
            // 计算：两次换相间隔 = 60°电角度 → 30° = 间隔 / 2
            uint32_t total_60deg = zero_cross_tick - last_comm_tick;
            uint32_t delay_30deg = total_60deg / 2;

            // 计算当前转速（RPM）
            if (total_60deg > 0)
            {
                now_rpm = 2500 / total_60deg;
            }

            // 延时30°电角度后执行换相
            if (now_tick - zero_cross_tick >= delay_30deg)
            {
                cur_sector = (cur_sector + 1) % 6;
                APP_Motor_SixStep_Update(cur_sector, motor_duty);
                last_comm_tick = xTaskGetTickCount(); // 记录本次换相时间戳
            }
        }

        debug_printf(":%d\n", cur_sector); // 打印当前扇区
        debug_printf(": %lu\n", now_rpm);  // 打印当前转速
        vTaskDelay(MOTOR_TASK_PERIOD);
    }
}

void receive_task(void *args)
{
    // 获取当前的基准时间
    TickType_t startTime = xTaskGetTickCount();
    Int_uart_receive_Init();
    Int_pwm_in_Init();
    while (1)
    {
        target_rpm = Int_uart_receive_GetData();
        float duty_cycle = Int_pwm_in_GetData();
        if (duty_cycle > 5.0 && duty_cycle < 10.0)
        {
            target_rpm = (duty_cycle - 5.0) * 1440.0;
        }

        // 3. PID调速
        motor_pid.target = target_rpm;
        motor_pid.measure = now_rpm;
        Com_PID_Calc(&motor_pid);
        motor_duty = (uint16_t)motor_pid.output;

        debug_printf(":%.2f\n", duty_cycle);
        debug_printf(":%u,%u,%u\n", target_rpm, now_rpm, motor_duty);
        vTaskDelayUntil(&startTime, RECEIVE_TASK_PERIOD);
    }
}
