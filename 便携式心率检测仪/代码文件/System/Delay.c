#include "delay.h"

//us延时
uint8_t alarm_flag;
uint16_t alarm_time;
void Delay_us(uint64_t time)
{
	while(time--)
	{
		Delay_1us();
	}
}

//ms延时
void Delay_ms(uint64_t time)
{
	time *= 1000;
	while(time--)
	{
		Delay_1us();
	}	
}

void Delay_Init(void)
{
	if(SysTick_Config(72000) == 1)//定时1ms
	{
		while(1);
	}
}
//1800000
vu32 timeled[2]={0,1800000};
vu32 timelea[2]={0,10000};
vu32 timekey[2]={0,1};
vu32 timedc[2]={0,10};
vu32 timedi[2]={0,1000};
vu32 timedt[2]={0,1000};

//1ms发生中断1次
void SysTick_Handler(void)
{
	if(alarm_flag )
		alarm_time++;
	timeled[0]++;//每隔1ms就会执行1次
	timekey[0]++;
	timedc[0]++;
	timedi[0]++;
	timedt[0]++;
	timelea[0]++;
}
