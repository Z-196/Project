#include "servo.h"

// 舵机初始化：配置PB6为TIM4_CH1复用PWM输出，50Hz频率
void Servo_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    // 1. 开启时钟（GPIO-B + TIM4）
    RCC_APB2PeriphClockCmd(SERVO_GPIO_CLK, ENABLE);  // 使能GPIOB时钟
    RCC_APB1PeriphClockCmd(SERVO_TIM_CLK, ENABLE);   // 使能TIM4时钟（APB1需注意）

    // 2. 配置PB6为复用推挽输出（TIM4_CH1复用功能）
    GPIO_InitStruct.GPIO_Pin = SERVO_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;     // 复用推挽输出（定时器控制）
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   // 输出速率50MHz
    GPIO_Init(SERVO_GPIO_PORT, &GPIO_InitStruct);

    // 3. 配置TIM4时基参数（生成50Hz PWM波）
    // 系统时钟72MHz → APB1预分频默认2 → TIM4时钟=72MHz/2=36MHz
    TIM_TimeBaseStruct.TIM_Period = 20000 - 1;      // ARR = 19999（周期20ms）
    TIM_TimeBaseStruct.TIM_Prescaler = 72 - 1;      // PSC = 71（72MHz/72=1MHz）
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟不分频
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInit(SERVO_TIM, &TIM_TimeBaseStruct);

    // 4. 配置TIM4通道1为PWM模式1
    TIM_OCStructInit(&TIM_OCInitStruct);             // 初始化默认参数
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;   // PWM模式1（CNT<CCR时输出高电平）
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; // 使能通道输出
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High; // 输出高电平有效（适配舵机）
    TIM_OCInitStruct.TIM_Pulse = SERVO_90_DEG;       // 初始角度：90°（避免上电冲击）
    TIM_OC1Init(SERVO_TIM, &TIM_OCInitStruct);       // 配置TIM4通道1

    // 5. 使能预加载（CCR/ARR值同步更新，避免PWM波形抖动）
    TIM_OC1PreloadConfig(SERVO_TIM, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(SERVO_TIM, ENABLE);

    // 6. 启动TIM4定时器
    TIM_Cmd(SERVO_TIM, ENABLE);
}

// 直接设置CCR值（底层接口，用于精准校准）
void Servo_SetCCR(uint16_t ccr)
{
    // 限制CCR范围，防止舵机机械损坏
    if (ccr < SERVO_MIN_CCR) ccr = SERVO_MIN_CCR;
    if (ccr > SERVO_MAX_CCR) ccr = SERVO_MAX_CCR;
    TIM_SetCompare1(SERVO_TIM, ccr); // 设置TIM4通道1的CCR值
}

// 舵机角度控制（0°~180°）：线性映射角度到CCR值
void Servo_SetAngle(int16_t angle)
{
    uint16_t ccr;
    // 限制角度范围（避免无效值）
    if (angle > 180) angle = 180;
    if (angle < 0) angle = 0;

    // 角度→CCR线性映射公式：ccr = 最小CCR + (角度/180)×(最大CCR-最小CCR)
    ccr = SERVO_0_DEG + (angle * (SERVO_180_DEG - SERVO_0_DEG)) / 180;
    Servo_SetCCR(ccr); // 调用底层函数设置CCR
}

