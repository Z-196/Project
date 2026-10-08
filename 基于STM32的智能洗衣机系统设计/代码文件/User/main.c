#include "stm32f10x.h"
#include "Delay.h"
#include "math.h"
#include "oled.h"		// OLED
#include "key.h"		// KEY
#include "Buzzer.h"		// 蜂鸣器
#include "esp8266.h"

#include "HX711.h"		// 称重
#include "Measure.h"	// 浑浊度+水位

#include "Motor.h"		// L298N电机
#include "relay.h"		// 继电器——>微型水泵

#include "main.h"
#include "W25Q64.h"
#include <string.h>		// 包含memcmp内存比较函数

uint8_t buff[1024] = {0};
Page_enum page = page_1;						// i页面标志位
Work_mode_enum 		work_mode 	= auto_work;	// 工作模式选择 （自动/手动）
Wash_enum			wash 		= wash_close;	// 洗衣机开关	（启动/关闭）
Wash_mode_enum 		wash_mode;					// 洗衣模式选择	（标准/快速/强力）
Water_level_enum 	water_level;				// 水位挡位选择 （低/中/高）

uint8_t Line_select = 0;		// 光标选择	                     
uint8_t KeyNum=0;   			// 按键标志位
uint8_t control_state = 4;		// 洗衣流程控制标志位（手动模式）
uint8_t wash_state = 4;			// 洗衣流程标志位    （自动模式）
uint8_t wash_over_state = 0;	// 洗衣结束标志位


uint16_t HZD;			// 400——3300
uint16_t HZD_min;		// 浑浊度下限

//耦合于水位挡位
float Water;					// 水位百分比
uint8_t Water_ok;
uint8_t Water_low  = 30;		// 低挡水位
uint8_t Water_mid  = 60;		// 中挡水位
uint8_t Water_high = 80;		// 高挡水位
uint8_t Water_min  = 5;			//设定最低排水水位为5%

//耦合于洗衣模式
int8_t Speed;			// -100~100
int8_t Speed_stand = 25;
int8_t Speed_fast  = 35;
int8_t Speed_strong= 50;

uint8_t Count;
uint8_t Count_ok;
uint8_t Count_stand 	= 3;	//标准模式电机转2s
uint8_t Count_fast  	= 5;	//快速模式电机转5s
uint8_t Count_strong	= 8;	//强力模式电机转8s

uint8_t Count_Buzzer;

void ShowData(void);            // 数据页面
void GetData(void);             // 获取数据
void Alarm(void);               // 自动控制函数
void Menu_key_set(void);        // 按键扫描
void Key_Mune(void);            // 按键切换页面操作
void WIFI_USE(void);
uint8_t WiFi_State = 0;	// WiFi
void WIFI_Init(void);
uint8_t WIFI_SEND;

// 全局配置变量
uint8_t now_config[4];        // 当前生效的配置
uint8_t last_config[4]; // 上次保存到Flash的配置
uint32_t weight_init;
/*
0：wash_mode
1：water_level
2：HZD_min的低8位
3：HZD_min的高8位
*/

uint16_t s_hzd_min;
uint8_t s_washmode;
uint8_t s_waterlevel;
uint8_t s_workmode;
uint8_t s_handcontrol;
uint8_t s_start;
uint8_t rx_state;

int main(void)
{   
    SystemInit();  
    Delay_Init();           //滴答定时器初始化
	/*控制/通信*/
    OLED_Init();            //OLED屏初始化
	OLED_Clear();	
    Key_Init();             //按键初始化
	Buzzer_Init();			//蜂鸣器初始化
	
	/*传感器*/
	HX711_Init();			// 称重
	Measure_Init();			// 浑浊度+水位
	
	/*电机/水泵*/
	Motor_Init();			// 电机
	relay_Init();			// 水泵
	
	relay_OFF(RELAY_1);
	relay_OFF(RELAY_2);	
	OLED_Update();
	
	W25Q64_Init();
	
	W25Q64_ReadData(0x000000, last_config, 4);			//读取外部存储的数据到指定数组中
	wash_mode = (Wash_mode_enum)last_config[0];
	water_level = (Water_level_enum)last_config[1];
	HZD_min = last_config[2]|(last_config[3]<<8);

	WIFI_Init();
    while(1)		
    {   	
		if(page == page_5)	
		{
			now_config[0] = (uint8_t)wash_mode;
			now_config[1] = (uint8_t)water_level;
			now_config[2] = (uint8_t)HZD_min;
			now_config[3] = (uint8_t)(HZD_min>>8);
			if(memcmp(&last_config,&now_config,4) != 0)			//如果不相等
			{
				/*W25Q64功能函数*/
				W25Q64_SectorErase(0x000000);					//扇区擦除
				W25Q64_PageProgram(0x000000, now_config, 4);	//将写入数据的测试数组写入到W25Q64中
				memcpy(&last_config,&now_config,4);
			}			
		}	
        if(timekey[0]>=timekey[1])          //滴答定时器10ms扫描一次按键
        {
            Menu_key_set();                 //获取按键数据
            timekey[0] = 0;                 //滴答定时器计数清零
        }     
        if(timedi[0]>=timedi[1])            //1s执行一次
        {				
            GetData();     //读取数据
			if(WIFI_SEND == 0) WIFI_SEND = 1;
			WIFI_USE();
			timedi[0] = 0;
        }
        if(timedc[0]>=timedc[1])            //1S执行一次
        {
			if(wash_state == 1)
			{
				Count++;
				if(Count > Count_ok)	{wash_state = 2;Count = 0;}
			}
			else if(wash_state == 3)
			{
				Count++;
				if(Count > Count_ok)	{wash_state = 4;Count = 0;}
			}
			if(GPIO_ReadOutputDataBit(GPIO_BUZZER,BUZZER_X) == 0)	//蜂鸣器响
			{
				Count_Buzzer++;
				if(Count_Buzzer > 2)	//蜂鸣器响3s自动停止
				{wash_over_state = 1;Buzzer_OFF();Count_Buzzer = 0;}	
			}	
			timedc[0] = 0;
        }
		
		switch (rx_state)
		{
			case 1: HZD_min = s_hzd_min;break;
			case 2: work_mode = (Work_mode_enum)s_workmode;break;
			case 3: water_level = (Water_level_enum)s_waterlevel;break;
			case 4: wash_mode = (Wash_mode_enum)s_washmode;break;
			case 5: control_state = s_handcontrol;break;
			case 6:	wash = (Wash_enum)s_start;break;
			default:break;
		}
		rx_state = 0;
		DataAnylize();		
        Alarm();    						// 自动控制函数
        Key_Mune();                         // 页面显示
        OLED_Update();                      // 数据更新
    }
}

void GetData(void)      //读取数据
{
	Get_Weight();		//重量存于Weight_Shiwu变量
	HZD = (uint16_t)Measure_GetTSB300_TU();		//浑浊度
	Water = Measure_GetWater_Percent();			//水位百分比
}

void Menu_key_set(void)     //按键扫描
{
    KeyNum = Get_Key();
    if(KeyNum == 1)   //按键1按下页面更新
	{
		if(page == page_1)		page = page_5;
		else if(page == page_2)	page = page_3;
		else if(page == page_3)	page = page_4;
		else if(page == page_4)	page = page_1;
		else if(page == page_5)	
		{
			if(wash == wash_start)
			{
				if(work_mode == auto_work)	page = page_6;
				else 						page = page_7;
			}
			else	page = page_2;
		}
		else if(page == page_6)				page = page_5;
		else if(page == page_7)				page = page_5;
	}
	if(KeyNum == 2)		//光标更新
	{
		Line_select ++;Line_select %= 30;	
		if(page == page_1)
		{
			uint8_t temp = Line_select % 2;
			if(temp)	work_mode = auto_work;
			else 		work_mode = hand_work;
		}
		else if(page == page_2)
		{
			uint8_t temp = Line_select % 3;
			if(temp == 0)			wash_mode = stand_mode;
			else if(temp == 1)		wash_mode = fast_mode;
			else if(temp == 2)		wash_mode = strong_mode;
		}
		else if(page == page_3)
		{
			uint8_t temp = Line_select % 3;
			if(temp == 0)			water_level = low_level;
			else if(temp == 1)		water_level = mid_level;
			else if(temp == 2)		water_level = high_level;
		}
		else if(page == page_4)		{HZD_min -= 300;if(HZD_min > 6000) HZD_min = 2600;}
		else if(page == page_5)
		{
			uint8_t temp = Line_select % 2;
			if(temp)	wash = wash_start;
			else 		wash = wash_close;
		}
		else if(page == page_7)		control_state = Line_select % 5;
	}
}

void Key_Mune(void)         //按键页面切换
{
    OLED_Clear();
	ShowData();
}

void Alarm(void)            //数据处理，判断数据是否异常，异常后执行的操作
{
	if(wash == wash_start)
	{
		if(work_mode == auto_work)
		{
			if(HZD <= HZD_min && wash_state!=2)		//已漂洗干净
			{
				wash_over_state = 1;
				wash = wash_close;
				Buzzer_ON();
			}
			else	{wash_over_state = 0;}			//未漂洗干净
		}
		else // hand_work
		{
			if(HZD <= HZD_min && control_state!=2)	//已漂洗干净
			{
				wash_over_state = 1;
				Buzzer_ON();
				
				Motor_SetSpeed(0);
				relay_OFF(RELAY_1);
				relay_OFF(RELAY_2);
			}
			else	{wash_over_state = 0;}			//未漂洗干净
		}
	}
	if(wash_mode == stand_mode) 		{Speed = Speed_stand; Count_ok = Count_stand;}
	else if(wash_mode == fast_mode) 	{Speed = Speed_fast;  Count_ok = Count_fast;}
	else if(wash_mode == strong_mode) 	{Speed = Speed_strong;Count_ok = Count_strong;}
	
	if(water_level == low_level) 		Water_ok = Water_low;
	else if(water_level == mid_level) 	Water_ok = Water_mid;
	else if(water_level == high_level) 	Water_ok = Water_high;
	
	if(wash == wash_start)
	{
		if(work_mode == auto_work)
		{
			switch (wash_state)
			{
				case 0://进水
					if(Water < Water_ok)	relay_ON(RELAY_1);
					else	{relay_OFF(RELAY_1);wash_state = 1;}
					break;
				case 1://洗涤
					Motor_SetSpeed(Speed);
					break;
				case 2://排水
					Motor_SetSpeed(0);
					if(Water > Water_min)	relay_ON(RELAY_2);
					else	{relay_OFF(RELAY_2);wash_state = 3;}
					break;
				case 3://脱水
					Motor_SetSpeed(Speed*2);
					break;
				case 4://空置	无操作，作为空状态
					Motor_SetSpeed(0);
					wash_state = 0;
					break;
				default:
					break;
			}
		}
		else if(work_mode == hand_work)
		{
			switch (control_state)
			{
				case 0://进水
					if(Water < Water_ok)	relay_ON(RELAY_1);
					else	{relay_OFF(RELAY_1);}
					break;
				case 1://洗涤
					relay_OFF(RELAY_1);
					Motor_SetSpeed(Speed);
					break;
				case 2://排水
					Motor_SetSpeed(0);
					if(Water > Water_min)	relay_ON(RELAY_2);
					else	{relay_OFF(RELAY_2);}
					break;
				case 3://脱水
					relay_OFF(RELAY_2);
					Motor_SetSpeed(Speed*2);
					break;
				case 4://空置	无操作，作为空状态
					Motor_SetSpeed(0);
					break;
				default:
					break;
			}	
		}
	}
	else	//wash = wash_close
	{
		Count = 0;
		Motor_SetSpeed(0);
		relay_OFF(RELAY_1);
		relay_OFF(RELAY_2);
	}
}

void ShowData(void)         //数据页面显示
{
    if(page == page_1)       
    {			
		uint8_t MID;
		uint16_t DID;
		W25Q64_ReadID(&MID, &DID);
		/*第一行*/
		OLED_ShowChinese(0,0,"重量：");						//重量
		OLED_ShowString(80,0,"g",OLED_8X16);
		OLED_ShowNum(48,0,Weight_Shiwu,4,OLED_8X16);
					
		/*第二行*/
		OLED_ShowChinese(0,16,"浑浊度：");					//浑浊度
		OLED_ShowNum(64,16,(int16_t)HZD,4,OLED_8X16);
		
		/*第三行*/
		OLED_ShowChinese(0,32,"水位：");					//水位
		OLED_ShowString(72,32,"%",OLED_8X16);							
		OLED_ShowNum(48,32,Water,3,OLED_8X16);
		
		/*第四行*/	
		OLED_ShowChinese(0,48,"控制模式：");				//控制模式
		if(work_mode)   OLED_ShowChinese(80,48,"自动");
		else			OLED_ShowChinese(80,48,"手动");         						
    }
    else if(page == page_2)       
    {
		OLED_ShowChinese(0, 0,"洗衣模式设置");
		OLED_ShowChinese(0,32,"标准");
		OLED_ShowChinese(48,32,"快速");
		OLED_ShowChinese(96,32,"强力");
		if(wash_mode == stand_mode)			OLED_DrawLine(0,48,32,48);
		else if(wash_mode == fast_mode)		OLED_DrawLine(48,48,80,48);
		else if(wash_mode == strong_mode)	OLED_DrawLine(96,48,128,48);		
    }
    else if(page == page_3)       
    {
		OLED_ShowChinese(0, 0,"水位挡位设置");
		OLED_ShowChinese(0,32,"低");
		OLED_ShowChinese(32,32,"中");
		OLED_ShowChinese(64,32,"高");
		if(water_level == low_level)		OLED_DrawLine(0,48,16,48);
		else if(water_level == mid_level)	OLED_DrawLine(32,48,48,48);
		else if(water_level == high_level)	OLED_DrawLine(64,48,80,48);
    }
    else if(page == page_4)
	{
		OLED_ShowChinese(0,0,"浑浊度阈值设置");
		OLED_ShowString(0,16,"MIN:",OLED_8X16);
		OLED_ShowNum(32,16,(int16_t)HZD_min,4,OLED_8X16);
		OLED_DrawLine(32,32,64,32);
	}
    else if(page == page_5)
	{
		OLED_ShowChinese(0,0,"洗衣模式：");
		if(wash_mode == stand_mode)			OLED_ShowChinese(80,0,"标准");
		else if(wash_mode == fast_mode)		OLED_ShowChinese(80,0,"快速");
		else if(wash_mode == strong_mode)	OLED_ShowChinese(80,0,"强力");
		
		OLED_ShowChinese(0,16,"水位挡位：");
		if(water_level == low_level)		OLED_ShowChinese(80,16,"低");
		else if(water_level == mid_level)	OLED_ShowChinese(80,16,"中");
		else if(water_level == high_level)	OLED_ShowChinese(80,16,"高");
		
		OLED_ShowChinese(0,32,"浑浊度");				//浑浊度
		OLED_ShowString(48,32,":",OLED_8X16);		
		OLED_ShowNum(56,32,(int16_t)HZD,4,OLED_8X16);
		OLED_ShowString(88,32,"/",OLED_8X16);
		OLED_ShowNum(96,32,(int16_t)HZD_min,4,OLED_8X16);
				
		OLED_ShowChinese(0, 48,"启动");		
		OLED_ShowChinese(64,48,"关闭");
		if(wash == wash_start)			OLED_DrawRectangle(34,52,8,8,OLED_FILLED);
		else if(wash == wash_close)		OLED_DrawRectangle(100,52,8,8,OLED_FILLED);
	}
	else if(page == page_6)
	{
		OLED_ShowChinese(0,0,"自动控制");
		if(wash_over_state)	OLED_ShowChinese(0,48,"洗衣完成");
		
		OLED_ShowChinese( 0,16,"进水");
		OLED_ShowChinese(48,16,"洗涤");
		OLED_ShowChinese(96,16,"排水");
		OLED_ShowChinese(16,32,"脱水");
		OLED_ShowChinese(80,32,"空置");

		if(wash_state == 0)			OLED_DrawLine(0 ,32, 32,32);   //进水
		else if(wash_state == 1)	OLED_DrawLine(48,32, 80,32);   //洗涤
		else if(wash_state == 2)	OLED_DrawLine(96,32,128,32);   //排水
		else if(wash_state == 3)	OLED_DrawLine(16,48, 48,48);   //脱水
		else if(wash_state == 4)	OLED_DrawLine(80,48,112,48);   //空置
	}	
	else if(page == page_7)
	{
		OLED_ShowChinese(0,0,"手动控制");
		if(wash_over_state)	OLED_ShowChinese(0,48,"洗衣完成");
		
		OLED_ShowChinese( 0,16,"进水");
		OLED_ShowChinese(48,16,"洗涤");
		OLED_ShowChinese(96,16,"排水");
		OLED_ShowChinese(16,32,"脱水");
		OLED_ShowChinese(80,32,"空置");

		if(control_state == 0)		OLED_DrawLine(0 ,32, 32,32);   //进水
		else if(control_state == 1)	OLED_DrawLine(48,32, 80,32);   //洗涤
		else if(control_state == 2)	OLED_DrawLine(96,32,128,32);   //排水
		else if(control_state == 3)	OLED_DrawLine(16,48, 48,48);   //脱水
		else if(control_state == 4)	OLED_DrawLine(80,48,112,48);   //空置
	}	
}

void WIFI_Init(void)
{
	OLED_ShowChinese(20,20,"联网中");
	OLED_Update();
	ESP8266_Config();
	Delay_ms(500);
	ConnectToHotspot();	//连接网络
	Delay_ms(2000);
	WiFi_State = ConnectToBaffServer();	//连接巴法云
			
	if(WiFi_State == 1)
	{
		OLED_ShowString(0,0,">>>",OLED_8X16);
		OLED_ShowString(24,0,"SUCCESS",OLED_8X16);
		OLED_ShowString(80,0,"<<<",OLED_8X16);
		OLED_ShowString(0,16,"WIFI:CONNECTED",OLED_8X16);		
		OLED_ShowString(0,32,"System Running",OLED_8X16);		
		OLED_Update();
		WifiSendStr("AT+CIPSEND=74\r\n");
		Delay_ms(500);   // 等待 >
		// 订阅所有必要的主题，确保能接收所有控制命令
		sprintf((char*)buff,"cmd=1&uid=%s&topic=%s,%s\r\n",PRODUCTID,"DATA","switch");
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
	if(WIFI_SEND)
	{
		sprintf((char*)buff,"cmd=2&uid=%s&topic=%s&msg=#%d#%d#%d#%d#%d#%d#%d#%d#\r\n",
				PRODUCTID,"DATA",
				Weight_Shiwu,(int16_t)HZD,(uint8_t)Water,(uint8_t)work_mode,
				(uint8_t)wash_state,control_state,wash_over_state,(uint8_t)wash);
		WifiSendStr((char*)buff);
		WIFI_SEND = 0;
	}
}



