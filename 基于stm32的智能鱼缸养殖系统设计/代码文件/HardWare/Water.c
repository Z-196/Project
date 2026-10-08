#include "Water.h"
#include "adc_all.h"
#include "delay.h"

static uint16_t Get_AD_Average(uint8_t times)
{
    u32 temp_val = 0;
    u8 t;
    for(t=0; t<times; t++)
    {
        temp_val += ADC_Read_WaterLevel();
        Delay_ms(5);
    }
    return temp_val / times;
}

float Get_Water_V(void)
{
    uint16_t adx = Get_AD_Average(5);
    return (float)adx / 4096 * 3.3f;
}

float Get_Water(void)
{
    float volt = Get_Water_V();

    if(volt <= VOLT_EMPTY) return 0;
    if(volt >= VOLT_FULL)  return 100;
    return (volt - VOLT_EMPTY) / (VOLT_FULL - VOLT_EMPTY) * 100;
}
