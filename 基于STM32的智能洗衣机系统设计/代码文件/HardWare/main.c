#include "stm32f10x.h"		//STM32头文件
#include "Delay.h"				//延时函数头文件
#include "math.h"
#include "oled.h"					//OLED屏头文件
#include "key.h"					//按键头文件
#include "LED.h"					//LED灯头文件
#include "max30102.h"
#include "ecg.h"


uint16_t i; 			//i页面标志位

uint8_t dis_hr_threshold = 130;
uint8_t dis_spo2_threshold = 80;


void ShowData(void);    //数据页面
void GetData(void);			//获取数据
void Alarm(void);				//自动控制函数
void Menu_key_set(void);//按键扫描
void Key_Mune(void);		//按键切换页面操作
void ShowTime(void);		//获取时间函数

int main(void)
{	
	SystemInit();  
	Delay_Init();					//滴答定时器初始化
	OLED_Init();					//OLED屏初始化
	Key_Init();						//按键初始化	
	OLED_Clear();
	OLED_Printf(0,32,OLED_8X16,"初始化");
	OLED_Update();
	max30102_init();        //max30102初始化
	max30102_blood();       //max30102数据样本处理

	OLED_Clear();	
	while(1)
	{		
		//若需要其他定时时间在Delay.c与.h中添加
		if(timekey[0]>=timekey[1]) //滴答定时器10ms扫描一次按键
		{
			Menu_key_set();					 //获取按键数据
			timekey[0] = 0;					 //滴答定时器计数清零
		}
		
		if(timedi[0]>=timedi[1])	 //1S执行一次
		{
			timedi[0] = 0;
		}
		
		if(timeled[0]>=timeled[1]) //2S执行一次
		{
			GetData();							 //获取传感器数据
		}
		
		Alarm();
		Key_Mune();								 //页面显示
		}
}


void ShowData(void)
{
	//数据页面显示
	OLED_Printf(0,16,OLED_8X16,"心率:%d\n血氧%d\n",dis_hr,dis_spo2);
}

void GetData(void)
{
	heart_SpO2();					//心率血样显示
}

void Menu_key_set(void)									//按键扫描
{
	if(key1 == 1)
	{
		i++;			//按键1按下页面更新
		key1 = 0;
	}
	if(i >= 2)														//有几个页面 i >= j，j就改为多少
	{
		i = 0;															//页面浏览完重回初始数据页面
	}
}

void Key_Mune(void)											//按键页面切换
{

		ShowData();

}

void Alarm(void)
{
}
