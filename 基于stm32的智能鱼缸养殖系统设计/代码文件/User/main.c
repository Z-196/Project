#include "main.h"		// 枚举定义
#include "stm32f10x.h"
#include "Delay.h"
#include "math.h"
#include "oled.h"		// OLED
#include "key.h"		// KEY
#include "LED.h"		// LED
#include "Buzzer.h"		// 蜂鸣器
#include "esp8266.h"	// WiFi
#include "DS18B20.h"	// 水体温度检测模块
#include "BH1750.h"		// 光照强度检测模块
#include "water.h"		// 水位传感器
#include "TSB300.h"		// 浑浊度传感器
#include "tds.h"		// TDS传感器
#include "adc_all.h"	// 初始化
#include "DS1302.h"		// 外部RTC时钟
#include "servo.h"		// 舵机初始化
#include "relay.h"		// 继电器模块

void ShowData(void);            // 数据页面
void GetData(void);             // 获取数据
void Alarm(void);               // 自动控制函数
void Menu_key_set(void);        // 按键扫描
void Key_Mune(void);            // 按键切换页面操作
void WIFI_USE(void);			// WiFi
uint8_t WiFi_State = 0;			
void WIFI_Init(void);  
uint8_t WIFI_Send = 0;
void WIFI_RX(void);
uint8_t rx_state;

uint8_t buff[512] = {0};
Page_enum page = page_1;		// 页面标志位
uint8_t Line_select = 0;		// 光标选择	                     
uint8_t KeyNum=0;   			// 按键标志位
Workmode_enum workmode = work_auto;

uint8_t temp;	//水体温度
uint8_t temp_min = 10;
uint16_t lux;		//光照强度
uint16_t lux_min = 400;
uint8_t water_level;//水位
uint8_t water_level_max = 60;
uint8_t water_level_min = 10;
uint16_t hzd;		//浑浊度
uint16_t hzd_max = 2000;
uint16_t tds;			//水体TDS
uint16_t tds_max = 150;
uint16_t led_light = 700;	//LED强度
uint8_t time_max = 20;
uint8_t time_min = 18;

uint8_t state_servo;	//舵机
uint8_t state_relay1;	//进水水泵
uint8_t state_relay2;	//放水水泵
uint8_t state_relay3;	//加热片
uint8_t state_relay4;	//增氧泵
uint16_t state_led;		//LED开关
uint8_t state_buzzer;	//蜂鸣器
uint8_t state_feed;		//喂食
uint8_t state_change;	//换水
uint8_t state_temp;		//恒温
uint8_t state_o2;		//增氧

uint8_t feed_tick_now;
uint8_t feed_tick_set = 10;
uint8_t o2_tick_now;
uint8_t o2_tick_set = 45;

uint8_t s_temp_min;
uint16_t s_lux_min;
uint8_t s_water_level_max;
uint8_t s_water_level_min;
uint16_t s_hzd_max;
uint16_t s_tds_max;
uint16_t s_led_light;
uint8_t s_time_max;
uint8_t s_time_min;
uint8_t s_feed_tick_set;
uint8_t s_o2_tick_set;

uint8_t s_state_servo;	
uint8_t s_state_relay1;	
uint8_t s_state_relay2;	
uint8_t s_state_relay3;	
uint8_t s_state_relay4;	
uint16_t s_state_led;
uint8_t s_workmode;

int main(void)
{		
    SystemInit();  
    Delay_Init();           //滴答定时器初始化
	/*控制/通信*/
    OLED_Init();            //OLED屏初始化
	OLED_Clear();	
	OLED_Update();
    Key_Init();             //按键初始化
	Buzzer_Init();			//蜂鸣器初始化
	DS18B20_Init();			//温度传感器初始化
	Init_BH1750();			//光照强度传感器初始化
	ADC_ALL_Init();			//水位、PH、TDS初始化
	Servo_Init();			//舵机初始化
	Servo_SetAngle(0);
	relay_Init();			//继电器初始化	
	ds1302_gpio_init();
	ds1302_init();
	ds1302_read_realTime();
	
	WIFI_Init();	
    while(1)
    {   
        if(timekey[0]>=timekey[1])          //滴答定时器10ms扫描一次按键
        {
            Menu_key_set();                 //获取按键数据			
            timekey[0] = 0;                 //滴答定时器计数清零
        }       
        if(timedi[0]>=timedi[1])            //600ms执行一次
        {							
			WIFI_Send++;            
			GetData();     					//读取数据
			timedi[0] = 0;
        }
        if(timedc[0]>=timedc[1])            //1S执行一次
        {			
			if(workmode != work_hand)
			{
				feed_tick_now++;
				if(feed_tick_now >= feed_tick_set)	
				{
					state_feed = 1;feed_tick_now = 0;
				}
				if(workmode != work_cure)
				{
					o2_tick_now++;
					if(o2_tick_now >= o2_tick_set)	
					{
						state_o2 = 1;o2_tick_now = 0;
					}
				}
				if(state_o2)
				{
					static uint8_t count = 0;
					count ++;
					if(count > 5)
					{
						count = 0; state_o2 = 0;
					}
				}				
			}
			if(state_servo)
			{
				Servo_SetAngle(180);
				state_servo = 0;state_feed = 0;
			}
			else Servo_SetAngle(0);
					
			timedc[0] = 0;
		}
		WIFI_USE();WIFI_RX();	
		Alarm();    						// 自动控制函数
        Key_Mune();                         // 页面显示
        OLED_Update();                      // 数据更新
    }
}

void GetData(void)      	//读取数据
{
	if(page == page_1)
	{
		temp = (uint8_t)DS18B20_Get_Temp();
		lux = (uint16_t)read_BH1750();
		water_level = (uint8_t)Get_Water();			
		tds = (uint16_t)TDS_GetTDS();					
		hzd = (uint16_t)Get_HZD();
	}
}

void Menu_key_set(void)     //按键扫描
{
	KeyNum = Get_Key();
	if(KeyNum == 1)		
	{
		if(page == page_1)		page = page_2;
		else if(page == page_2)	
		{
			switch ((uint8_t)workmode)
			{
				case 0:page = page_7;break;
				case 1:page = page_3;break;
				case 2:page = page_5;break;
				case 3:page = page_6;break;
				default:break;
			}
		}
		else if(page == page_3)	page = page_4;
		else if(page == page_4)	page = page_1;
		else if(page == page_5)	page = page_1;
		else if(page == page_6)	page = page_1;
		else if(page == page_7)	page = page_1;
	}
	else if(KeyNum == 2)	
	{
		Line_select ++;Line_select %= 60;	
	}		
	else if(KeyNum == 3)
	{
		if(page == page_2)	workmode = (Workmode_enum)(((uint8_t)workmode + 1) % 4);
		else if(page == page_3)
		{
			uint8_t temp = Line_select % 4;
			switch (temp)
			{
				case 0:temp_min += 10;break;
				case 1:lux_min += 50;break;
				case 2:water_level_max += 10;break;
				case 3:water_level_min += 10;break;
				default:break;
			}
		}
		else if(page == page_4)
		{
			uint8_t temp = Line_select % 5;
			switch (temp)
			{
				case 0:hzd_max += 50;break;
				case 1:tds_max += 50;break;
				case 2:led_light += 100;led_light = (led_light <= 1000) ? (led_light+100) : 1000;break;
				case 3:feed_tick_set += 1;break;
				case 4:o2_tick_set += 1;break;
				default:break;
			}
		}
		else if(page == page_5)
		{
			uint8_t temp = Line_select % 2;
			if(temp)	time_max = (time_max < 24) ? (time_max + 1) : 24;
			else		time_min = (time_min < 24) ? (time_min + 1) : 24;
		}
		else if(page == page_6)
		{
			uint8_t temp = Line_select % 2;
			if(temp)	state_temp = (state_temp) ? 0 : 1;
			else		state_o2 = (state_o2) ? 0 : 1;
		}
		else if(page == page_7)
		{
			uint8_t temp = Line_select % 6;
			switch (temp)
			{
				case 0:state_relay2 = (state_relay2) ? 0 : 1;break;
				case 1:state_relay1 = (state_relay1) ? 0 : 1;break;
				case 2:state_servo = (state_servo) ? 0 : 1;break;
				case 3:state_relay3 = (state_relay3) ? 0 : 1;break;
				case 4:led_light = (led_light < 1000) ? (led_light + 100) : 1000;break;
				case 5:state_relay4 = (state_relay4) ? 0 : 1;break;
				default:break;
			}			
		}		
	}
	else if(KeyNum == 4)
	{
		if(page == page_2)	workmode = (Workmode_enum)(((uint8_t)workmode + 1) % 4);
		else if(page == page_3)
		{
			uint8_t temp = Line_select % 4;
			switch (temp)
			{
				case 0:temp_min -= 5;break;
				case 1:lux_min -= 10;break;
				case 2:water_level_max -= 5;break;
				case 3:water_level_min -= 5;break;
				default:break;
			}
		}
		else if(page == page_4)
		{
			uint8_t temp = Line_select % 5;
			switch (temp)
			{
				case 0:hzd_max = (hzd_max >= 5) ? (hzd_max - 5) : 0;break;
				case 1:tds_max = (tds_max >= 5) ? (tds_max - 5) : 0;break;
				case 2:led_light = (led_light >= 50) ? (led_light - 50) : 0;break;
				case 3:feed_tick_set = (feed_tick_set > 1) ? (feed_tick_set - 1) : 0;break;
				case 4:o2_tick_set = (o2_tick_set > 1) ? (o2_tick_set - 1) : 0;break;
				default:break;
			}
		}
		else if(page == page_5)
		{
			uint8_t temp = Line_select % 2;
			if(temp)	time_max = (time_max > 1) ? (time_max - 1) : 0;
			else		time_min = (time_min > 1) ? (time_min - 1) : 0;
		}
		else if(page == page_6)
		{
			uint8_t temp = Line_select % 2;
			if(temp)	state_temp = (state_temp) ? 0 : 1;
			else		state_o2 = (state_o2) ? 0 : 1;
		}
		else if(page == page_7)
		{
			uint8_t temp = Line_select % 6;
			switch (temp)
			{
				case 0:state_relay2 = (state_relay2) ? 0 : 1;break;
				case 1:state_relay1 = (state_relay1) ? 0 : 1;break;
				case 2:state_servo = (state_servo) ? 0 : 1;break;
				case 3:state_relay3 = (state_relay3) ? 0 : 1;break;
				case 4:led_light = (led_light >= 50) ? (led_light - 50) : 0;break;
				case 5:state_relay4 = (state_relay4) ? 0 : 1;break;
				default:break;
			}			
		}				
	}
}

void Key_Mune(void)         //按键页面切换
{
    OLED_Clear();
	ShowData();
}

void Alarm(void)            //数据处理，判断数据是否异常，异常后执行的操作
{	
	ds1302_read_realTime();
		
	if(workmode == work_auto)
	{
		state_change = (hzd > hzd_max || tds > tds_max) ? 1 : 0;
		state_servo = (state_feed) ? 1 : 0;
		if(!state_change)
		{
			state_relay1 = (water_level < water_level_min) ? 1 : 0;
			state_relay2 = (water_level > water_level_max) ? 1 : 0;			
			state_relay3 = (temp < temp_min) ? 1 : 0;			
		}
		else
		{
			state_relay3 = 1;
			static uint8_t state = 0;
			switch (state)
			{
				case 0 :					
				{
					if(water_level <= water_level_min)	{state_relay1 = 0;state_relay2 = 0;state = 1;}
					else {state_relay1 = 0;state_relay2 = 1;}						
					break;
				}
				case 1:
				{
					if(water_level >= water_level_max)	{state_relay1 = 0;state_relay2 = 0;state = 2;}
					else {state_relay1 = 1;state_relay2 = 0;}						
					break;						
				}
				case 2:
				{
					state = 0;state_change = 0;break;
				}
			}			
		}
		state_relay3 = (temp < temp_min) ? 1 : 0;
		state_relay4 = (state_o2) ? 1 : 0;
		state_led = (lux < lux_min) ? 1 : 0;
		state_buzzer = (state_relay1 || state_relay2 || state_relay3 ||state_relay4 || state_led) ? 1 : 0;			
	}
	else if(workmode == work_time)
	{
		if(TimeData.hour > time_min && TimeData.hour < time_max)
		{
			state_change = (hzd > hzd_max || tds > tds_max) ? 1 : 0;
			state_servo = (state_feed) ? 1 : 0;
			if(!state_change)
			{
				state_relay1 = (water_level < water_level_min) ? 1 : 0;
				state_relay2 = (water_level > water_level_max) ? 1 : 0;			
				state_relay3 = (temp < temp_min) ? 1 : 0;			
			}
			else
			{
				state_relay3 = 1;
				static uint8_t state = 0;
				switch (state)
				{
					case 0 :					
					{
						if(water_level <= water_level_min)	{state_relay1 = 0;state_relay2 = 0;state = 1;}
						else {state_relay1 = 0;state_relay2 = 1;}						
						break;
					}
					case 1:
					{
						if(water_level >= water_level_max)	{state_relay1 = 0;state_relay2 = 0;state = 2;}
						else {state_relay1 = 1;state_relay2 = 0;}						
						break;						
					}
					case 2:
					{
						state = 0;state_change = 0;break;
					}
				}			
			}	
			state_relay3 = (temp < temp_min) ? 1 : 0;
			state_relay4 = (state_o2) ? 1 : 0;
			state_led = (lux < lux_min) ? 1 : 0;
			state_buzzer = (state_relay1 || state_relay2 || state_relay3 ||state_relay4 || state_led) ? 1 : 0;			
		}
		else
		{				
			state_servo = 0;	
			state_relay1 = 0;	
			state_relay2 = 0;	
			state_relay3 = 0;	
			state_relay4 = 0;	
			state_led = 0;		
			state_buzzer = 0;					
		}
	}
	else if(workmode == work_cure)
	{
		state_change = (hzd > hzd_max || tds > tds_max) ? 1 : 0;
		state_servo = (state_feed) ? 1 : 0;
		if(!state_change)
		{
			state_relay1 = (water_level < water_level_min) ? 1 : 0;
			state_relay2 = (water_level > water_level_max) ? 1 : 0;
			state_relay3 = (temp < temp_min) ? 1 : 0;			
		}
		else
		{
			state_relay3 = 1;
			static uint8_t state = 0;
			switch (state)
			{
				case 0 :					
				{
					if(water_level <= water_level_min)	{state_relay1 = 0;state_relay2 = 0;state = 1;}
					else {state_relay1 = 0;state_relay2 = 1;}						
					break;
				}
				case 1:
				{
					if(water_level >= water_level_max)	{state_relay1 = 0;state_relay2 = 0;state = 2;}
					else {state_relay1 = 1;state_relay2 = 0;}						
					break;						
				}
				case 2:
				{
					state = 0;state_change = 0;break;
				}
			}			
		}	
		state_relay4 = (state_o2) ? 1 : 0;
		state_led = (lux < lux_min) ? 1 : 0;
		state_buzzer = (state_relay1 || state_relay2 || state_relay3 ||state_relay4 || state_led) ? 1 : 0;
	}	
	else if(workmode == work_hand)
	{
		state_buzzer = 0;
		state_led = 1;
	}

	(state_relay1) ? relay_ON(RELAY_1) : relay_OFF(RELAY_1);
	(state_relay2) ? relay_ON(RELAY_2) : relay_OFF(RELAY_2);
	(state_relay3) ? relay_ON(RELAY_3) : relay_OFF(RELAY_3);
	(state_relay4) ? relay_ON(RELAY_4) : relay_OFF(RELAY_4);
	(state_led) ? LED_SetBrightness(led_light) : LED_OFF();
	(state_buzzer) ? Buzzer_ON() : Buzzer_OFF();
}

void ShowData(void)         //数据页面显示
{	
	if(page	== page_1)
	{				
		OLED_ShowString(0,0,"T:",OLED_8X16);
		OLED_ShowNum(16,0,temp,3,OLED_8X16);
		OLED_ShowString(40,0,"Lux:",OLED_8X16);
		OLED_ShowNum(72,0,lux,5,OLED_8X16);
		
		OLED_ShowString(0,16,"L:",OLED_8X16);
		OLED_ShowNum(16,16,water_level,3,OLED_8X16);
		OLED_ShowString(40,16,"HZD:",OLED_8X16);
		OLED_ShowNum(72,16,hzd,5,OLED_8X16);
		
		OLED_ShowString(0,32,"TDS:",OLED_8X16);
		OLED_ShowNum(32,32,tds,5,OLED_8X16);
		switch ((uint8_t)workmode)
		{
			case 0:OLED_ShowChinese(80,32,"手动");break;
			case 1:OLED_ShowChinese(80,32,"自动");break;
			case 2:OLED_ShowChinese(80,32,"定时");break;
			case 3:OLED_ShowChinese(80,32,"治疗");break;
			default:break;
		}
		
		OLED_ShowString(0,48,"Feed:",OLED_8X16);
		OLED_ShowNum(40,48,feed_tick_now,2,OLED_8X16);
		OLED_ShowString(56,48,"/",OLED_8X16);
		OLED_ShowNum(64,48,feed_tick_set,2,OLED_8X16);
		
		OLED_ShowNum(88,48,o2_tick_now,2,OLED_8X16);
		OLED_ShowString(104,48,"/",OLED_8X16);
		OLED_ShowNum(112,48,o2_tick_set,2,OLED_8X16);		
	}
	else if(page == page_2)
	{
		OLED_ShowChinese(0,0,"工作模式：");
		switch ((uint8_t)workmode)
		{
			case 0:OLED_ShowChinese(80,0,"手动");break;
			case 1:OLED_ShowChinese(80,0,"自动");break;
			case 2:OLED_ShowChinese(80,0,"定时");break;
			case 3:OLED_ShowChinese(80,0,"治疗");break;
			default:break;
		}
		OLED_DrawLine(80,16,112,16);
				
		if(state_change)	OLED_ShowChinese(0,16,"换水");		
		(state_relay1) ? OLED_ShowChinese(0,32,"进水") : OLED_ShowChinese(0,32,"");
		(state_relay2) ? OLED_ShowChinese(0,48,"放水") : OLED_ShowChinese(0,48,"");
		
		OLED_ShowString(48,16,"L:",OLED_8X16);
		OLED_ShowNum(64,16,water_level,3,OLED_8X16);
		OLED_ShowString(48,32,"MAX:",OLED_8X16);
		OLED_ShowNum(80,32,water_level_max,3,OLED_8X16);
		OLED_ShowString(48,48,"MIN:",OLED_8X16);
		OLED_ShowNum(80,48,water_level_min,3,OLED_8X16);
	}
	else if(page == page_3)
	{
		OLED_ShowString(0,0,"T_MIN:",OLED_8X16);
		OLED_ShowNum(48,0,temp_min,3,OLED_8X16);
		OLED_ShowString(0,16,"Lux_MIN:",OLED_8X16);
		OLED_ShowNum(64,16,lux_min,5,OLED_8X16);
		OLED_ShowString(0,32,"Level_MAX:",OLED_8X16);
		OLED_ShowNum(80,32,water_level_max,3,OLED_8X16);
		OLED_ShowString(0,48,"Level_MIN:",OLED_8X16);
		OLED_ShowNum(80,48,water_level_min,3,OLED_8X16);

		uint8_t line = Line_select % 4;
		switch (line)
		{
			case 0:OLED_DrawRectangle(74,6,4,4,OLED_FILLED);break;
			case 1:OLED_DrawRectangle(106,22,4,4,OLED_FILLED);break;
			case 2:OLED_DrawRectangle(106,38,4,4,OLED_FILLED);break;
			case 3:OLED_DrawRectangle(106,54,4,4,OLED_FILLED);break;
			default:break;
		}
	}
	else if(page == page_4)
	{
		OLED_ShowString(0,0,"HZD_MAX:",OLED_8X16);
		OLED_ShowNum(64,0,hzd_max,5,OLED_8X16);
		OLED_ShowString(0,16,"TDS_MAX:",OLED_8X16);
		OLED_ShowNum(64,16,tds_max,5,OLED_8X16);
		OLED_ShowString(0,32,"LED_SET:",OLED_8X16);
		OLED_ShowNum(64,32,led_light,4,OLED_8X16);
		OLED_ShowString(0,48,"FEED:",OLED_8X16);
		OLED_ShowNum(40,48,feed_tick_set,2,OLED_8X16);
		OLED_ShowString(64,48,"O2:",OLED_8X16);
		OLED_ShowNum(88,48,o2_tick_set,2,OLED_8X16);

		uint8_t line = Line_select % 5;
		switch (line)
		{
			case 0:OLED_DrawRectangle(106,6,4,4,OLED_FILLED);break;
			case 1:OLED_DrawRectangle(106,22,4,4,OLED_FILLED);break;
			case 2:OLED_DrawRectangle(98,38,4,4,OLED_FILLED);break;
			case 3:OLED_DrawRectangle(58,54,4,4,OLED_FILLED);break;
			case 4:OLED_DrawRectangle(106,54,4,4,OLED_FILLED);break;
			default:break;
		}		
	}
	else if(page == page_5)
	{
		OLED_ShowNum(8,0,TimeData.year,4,OLED_8X16);					//Time
		OLED_ShowNum(56,0,TimeData.month,2,OLED_8X16);
		OLED_ShowNum(88,0,TimeData.day,2,OLED_8X16);
		OLED_ShowNum(30,16,TimeData.hour,2,OLED_8X16);
		OLED_ShowNum(54,16,TimeData.minute,2,OLED_8X16);
		OLED_ShowNum(78,16,TimeData.second,2,OLED_8X16);
		
		OLED_ShowString(0,32,"MAX:",OLED_8X16);
		OLED_ShowNum(32,32,time_max,2,OLED_8X16);
		OLED_ShowString(0,48,"MIN:",OLED_8X16);
		OLED_ShowNum(32,48,time_min,2,OLED_8X16);
		
		uint8_t line = Line_select % 2;
		if(line)	OLED_DrawRectangle(50,38,4,4,OLED_FILLED);
		else		OLED_DrawRectangle(50,54,4,4,OLED_FILLED);
	}	
	else if(page == page_6)
	{				
		OLED_ShowChinese(0,0,"恒温：");
		OLED_ShowChinese(0,16,"增氧：");
		(state_temp) ? OLED_ShowChinese(48,0,"开") : OLED_ShowChinese(48,0,"关");
		(state_o2) ? OLED_ShowChinese(48,16,"开") : OLED_ShowChinese(48,16,"关");
		
		uint8_t line = Line_select % 2;
		if(line)	OLED_DrawRectangle(66,6,4,4,OLED_FILLED);
		else		OLED_DrawRectangle(66,22,4,4,OLED_FILLED);
	}			
	else if(page == page_7)
	{
		OLED_ShowChinese(0,0, "放水：");
		OLED_ShowChinese(0,16,"进水：");
		OLED_ShowChinese(0,32,"喂食：");
		OLED_ShowChinese(0,48,"加热：");		
		OLED_ShowString(64,0,"LED:",OLED_8X16);
		OLED_ShowChinese(64,16,"增氧：");
		
		(state_relay2) ? OLED_ShowChinese(48,0,"开") : OLED_ShowChinese(48,0,"关");
		(state_relay1) ? OLED_ShowChinese(48,16,"开") : OLED_ShowChinese(48,16,"关");
		(state_feed) ? OLED_ShowChinese(48,32,"开") : OLED_ShowChinese(48,32,"关");
		(state_relay3) ? OLED_ShowChinese(48,48,"开") : OLED_ShowChinese(48,48,"关");
		OLED_ShowNum(96,0,led_light,4,OLED_8X16);
		(state_relay4) ? OLED_ShowChinese(112,16,"开") : OLED_ShowChinese(112,16,"关");
		
		uint8_t line = Line_select % 6;
		switch (line)
		{
			case 0:OLED_DrawLine(48,16,64,16);break;
			case 1:OLED_DrawLine(48,32,64,32);break;
			case 2:OLED_DrawLine(48,48,64,48);break;
			case 3:OLED_DrawRectangle(66,54,4,4,OLED_FILLED);break;
			case 4:OLED_DrawLine(96,16,128,16);break;
			case 5:OLED_DrawLine(112,32,128,32);break;
			default:break;
		}
	}
}

void WIFI_Init(void)
{
	OLED_ShowChinese(20,20,"联网中");
	Delay_ms(400);
	OLED_Update();
	ESP8266_Config();
	Delay_ms(500);
	ConnectToHotspot();	//连接网络
	Delay_ms(1000);
	WiFi_State = ConnectToBaffServer();	//连接巴法云
			
	if(WiFi_State == 1)
	{
		OLED_ShowString(0,0,">>>",OLED_8X16);
		OLED_ShowString(24,0,"SUCCESS",OLED_8X16);
		OLED_ShowString(80,0,"<<<",OLED_8X16);
		OLED_ShowString(0,16,"WIFI:CONNECTED",OLED_8X16);		
		OLED_ShowString(0,32,"System Running",OLED_8X16);		
		OLED_Update();
		// 订阅所有必要的主题，确保能接收所有控制命令
		sprintf((char*)buff,"cmd=1&uid=%s&topic=%s,%s\r\n",PRODUCTID,"DATA36","switch36");
		WifiSendStr((char*)buff);
		Delay_ms(1000);
		WiFi_State = 2;
	}
	else
	{
		OLED_ShowString(40,28,"ERROR",OLED_8X16);
	}
	OLED_Clear();
	OLED_Update();	
}

void WIFI_USE(void)
{
	if(WIFI_Send > 2)
	{		
		WIFI_Send = 0;
		sprintf((char*)buff,"cmd=2&uid=%s&topic=%s&msg=#%d#%d#%d#%d#%d#%d#%d#%d#%d#%d#%d#\r\n",
				PRODUCTID,"DATA36",
		temp,lux,water_level,hzd,tds,(uint8_t)workmode,feed_tick_set,o2_tick_set,led_light,time_max,time_min);
		WifiSendStr((char*)buff);
	}
}

void WIFI_RX(void)
{
	DataAnylize();		
	switch (rx_state)
	{
		case 1:temp_min = s_temp_min;break;
		case 2:lux_min = s_lux_min;break;
		case 3:water_level_max = s_water_level_max;break;
		case 4:water_level_min = s_water_level_min;break;
		case 5:hzd_max = s_hzd_max;break;
		case 6:tds_max = s_tds_max;break;
		case 7:led_light = s_led_light;break;
		case 8:time_max = s_time_max;break;
		case 9:time_min = s_time_min;break;
		case 10:feed_tick_set = s_feed_tick_set;break;
		case 11:o2_tick_set = s_o2_tick_set;break;
		case 12:workmode = work_hand;state_servo = s_state_servo;break;
		case 13:workmode = work_hand;state_relay1 = s_state_relay1;break;
		case 14:workmode = work_hand;state_relay2 = s_state_relay2;break;
		case 15:workmode = work_hand;state_relay3 = s_state_relay3;break;		
		case 16:workmode = work_hand;state_relay4 = s_state_relay4;break;		
		case 17:workmode = work_hand;state_led = s_state_led;break;
		case 18:workmode = (Workmode_enum)s_workmode;break;		
		default:break;		
	}
	rx_state = 0;
}
