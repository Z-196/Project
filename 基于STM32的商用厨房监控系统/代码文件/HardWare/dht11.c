#include "stm32f10x.h"                  // Device header
#include "Delay.h"

uint8_t dat[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t sum = 0;

#define DHT11_Pin   GPIO_Pin_1           // 修改为 PB6
#define DHT11_PORT  GPIOB                 // 修改为 GPIOB

void DHT11_Input(void) // 输入
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // 修改为 GPIOB 时钟使能
    
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = DHT11_Pin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStruct);  // 修改为 GPIOB
}

void DHT11_Out(void) // 输出
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // 修改为 GPIOB 时钟使能
    
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Pin = DHT11_Pin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStruct);  // 修改为 GPIOB
}

uint8_t DHT11_Read_Byte(void)
{
    uint8_t temp;
    uint8_t ReadDat = 0;
    uint8_t relay = 0;
    int i = 0;
    
    for(i = 0; i < 8; i++)
    {
        while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_Pin) == RESET && relay < 100)
        {
            Delay_us(1);
            relay++;
        }
        relay = 0;
        
        Delay_us(30);
        temp = 0;
        
        if(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_Pin) == SET)
            temp = 1;
        while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_Pin) == SET && relay < 100)
        {
           Delay_us(1);
            relay++;
        }
        relay = 0;
        
        ReadDat = ReadDat << 1;
        ReadDat = ReadDat | temp;
    }
    return ReadDat;
}

uint8_t DHT11_Read(void)
{
    int i = 0;
    uint8_t relay = 0;
    
    DHT11_Input();
    GPIO_ResetBits(DHT11_PORT, DHT11_Pin);  // 修改为 GPIOB
    Delay_ms(18);
    GPIO_SetBits(DHT11_PORT, DHT11_Pin);    // 修改为 GPIOB
    Delay_us(40);
    
    DHT11_Out();
    Delay_us(20);
    if(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_Pin) == RESET)  // 修改为 GPIOB
    {
        while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_Pin) == RESET && relay < 100)
        {
            Delay_us(1);
            relay++;
        }
        relay = 0;
        while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_Pin) == SET && relay < 100)
        {
            Delay_us(1);
            relay++;
        }
        relay = 0;
        
        for(i = 0; i < 5; i++)
        {
            dat[i] = DHT11_Read_Byte();
        }
        Delay_us(50);
    }
    sum = dat[0] + dat[1] + dat[2] + dat[3];
    if(dat[4] == sum)
        return 1;
    else
        return 0;
}
