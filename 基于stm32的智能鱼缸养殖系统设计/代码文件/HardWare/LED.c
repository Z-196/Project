#include "LED.h"

// 设置亮度：0~1000
// 0 = 最暗，1000 = 最亮
void LED_SetBrightness(uint16_t brightness)
{
    if(brightness > 999) brightness = 999;  // 限幅
	brightness = 1000 - brightness;
    TIM_SetCompare2(TIM3, brightness*20);
}

// 亮（100%亮度）
void LED_ON(void)
{
    LED_SetBrightness(1000);
}

// 灭（0%亮度）
void LED_OFF(void)
{
    LED_SetBrightness(0);
}

// 翻转亮灭
void LED_Turn(void)
{
    if(TIM_GetCapture2(TIM3) == 0)
        LED_ON();
    else
        LED_OFF();
}
