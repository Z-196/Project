//#include "delay.h"

//static u8  fac_us=0;							//us延时倍乘数			   
//static u16 fac_ms=0;							//ms延时倍乘数

////初始化延迟函
////SYSTICK的时钟固定为HCLK时钟的1/8
////SYSCLK:系统时钟
//void Delay_Init(void)
//{
//	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
//	fac_us=SystemCoreClock/8000000;				//为系统时钟的1/8  
//	fac_ms=(uint16_t)fac_us*1000;					   
//}

////延时nus
////nus为要延时的us数.		    								   
//void delay_us(uint32_t nus)
//{		
//	uint32_t temp;	    	 
//	SysTick->LOAD=nus*fac_us; 					//时间加载	  		 
//	SysTick->VAL=0x00;        					//清空计数器
//	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数	  
//	do
//	{
//		temp=SysTick->CTRL;
//	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
//	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
//	SysTick->VAL =0X00;      					 //清空计数器	 
//}

////延时nms
////注意nms的范围
////SysTick->LOAD为24位寄存器,所以,最大延时为:
////nms<=0xffffff*8*1000/SYSCLK
////SYSCLK单位为Hz,nms单位为ms
////对72M条件下,nms<=1864 
//void delay_ms(uint16_t nms)
//{	 		  	  
//	uint32_t temp;		   
//	SysTick->LOAD=(uint32_t)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
//	SysTick->VAL =0x00;							//清空计数器
//	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
//	do
//	{
//		temp=SysTick->CTRL;
//	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
//	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
//	SysTick->VAL =0X00;       					//清空计数器	  	    
//} 
#include "delay.h"

//us延时
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

vu32 timeled[2]={0,1000};
vu32 timekey[2]={0,10};
vu32 timedc[2]={0,1000};
vu32 timedi[2]={0,1000};

//1ms发生中断1次
void SysTick_Handler(void)
{
	timeled[0]++;//每隔1ms就会执行1次
	timekey[0]++;
	timedc[0]++;
	timedi[0]++;
}
