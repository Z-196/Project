#include "servo.h"

// 舵机初始化：配置PB0为TIM3_CH3复用PWM输出，50Hz频率
void Servo_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    // 1. 开启时钟（GPIO-B + TIM3）
    RCC_APB2PeriphClockCmd(SERVO_GPIO_CLK | RCC_APB2Periph_GPIOA, ENABLE);  // 使能GPIOB时钟
    RCC_APB1PeriphClockCmd(SERVO_TIM_CLK, ENABLE);   // 使能TIM3时钟

	// 2. 配置 PA7 复用推挽输出（TIM3_CH2）
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	
    // 2. 配置PB0为复用推挽输出（TIM3_CH3复用功能）
    GPIO_InitStruct.GPIO_Pin = SERVO_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;     // 复用推挽输出（定时器控制）
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   // 输出速率50MHz
    GPIO_Init(SERVO_GPIO_PORT, &GPIO_InitStruct);

    // 3. 配置TIM3时基参数（生成50Hz PWM波）
    // 系统时钟72MHz → APB1预分频默认2 → TIM3时钟=36MHz
    TIM_TimeBaseStruct.TIM_Period = 20000 - 1;      // ARR = 19999（周期20ms）
    TIM_TimeBaseStruct.TIM_Prescaler = 72 - 1;      // PSC = 71（计数精度1μs）
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(SERVO_TIM, &TIM_TimeBaseStruct);

    // 4. 配置TIM3通道3为PWM模式1
    TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_Pulse = 0;            
    TIM_OC2Init(TIM3, &TIM_OCInitStruct);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);	
	
    TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_Pulse = SERVO_0_DEG;       // 初始角度：90°
    TIM_OC3Init(SERVO_TIM, &TIM_OCInitStruct);       // 配置TIM3通道3

    // 5. 使能预加载
    TIM_OC2PreloadConfig(SERVO_TIM, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(SERVO_TIM, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(SERVO_TIM, ENABLE);

    // 6. 启动TIM3定时器
    TIM_Cmd(SERVO_TIM, ENABLE);
}

// 直接设置CCR值
void Servo_SetCCR(uint16_t ccr)
{
    if (ccr < SERVO_MIN_CCR) ccr = SERVO_MIN_CCR;
    if (ccr > SERVO_MAX_CCR) ccr = SERVO_MAX_CCR;
    TIM_SetCompare3(SERVO_TIM, ccr); // TIM3通道3 CCR
}

// 舵机角度控制（0°~180°）
void Servo_SetAngle(int16_t angle)
{
    uint16_t ccr;
    if (angle > 180) angle = 180;
    if (angle < 0) angle = 0;

    ccr = SERVO_0_DEG + (angle * (SERVO_180_DEG - SERVO_0_DEG)) / 180;
    Servo_SetCCR(ccr);
}
