#include "main.h"		// 枚举定义
#include "stm32f10x.h"
#include "Delay.h"
#include "math.h"
#include "oled.h"		// OLED
#include "key.h"		// KEY
#include "LED.h"		// LED
#include "Servo.h"		// SG90
#include "esp8266.h"	// ESP8266
#include "Serial.h"		// ASRPRO
#include "DS1302.h"		// 外部RTC	
#include "AS608_use.h"
#include "Store.h"

void ShowData(void);            // 数据页面
void GetData(void);             // 获取数据
void Alarm(void);               // 自动控制函数
void Menu_key_set(void);        // 按键扫描
void Key_Mune(void);            // 按键切换页面操作
void WIFI_USE(void);
uint8_t WiFi_State = 0;			// WiFi
void WIFI_Init(void);  
uint8_t WIFI_Send = 0;
void WIFI_RX(void);
uint8_t rx_state;
char RX_data;

uint8_t buff[512] = {0};
uint8_t KeyNum = 0;
uint8_t Line_select = 0;
Page_enum page = page_1;

uint8_t led_low = 10;
uint8_t	led_mid = 60;
LEDmode_enum ledmode = close;

uint8_t time_define[6];
uint8_t time_state;

uint8_t servo_state = 0;	//0上锁、1开锁

u8 fingerprint_num1 = 0;
unsigned int fr_timeout = 300;
int fr_verify_result = 0;
int fr_error_count = 0;          // 指纹验证错误次数
const int FR_MAX_ERROR = 3;      // 指纹最大错误次数（可自定义）

uint8_t lock_state = 0;

uint16_t s_year;
uint8_t s_month;
uint8_t s_day;
uint8_t s_hour;
uint8_t s_min;
uint8_t s_second;
uint8_t s_ledmode;
uint8_t	s_sleep;

int main(void)
{		
    SystemInit();  
    Delay_Init();           //滴答定时器初始化
	/*控制/通信*/
    OLED_Init();            //OLED屏初始化
	OLED_Clear();	
	OLED_Update();
    Key_Init();             //按键初始化
	LED_Init();				//LED初始化
	Servo_Init();			//舵机初始化
	Serial_Init();			//ASRPRO初始化
	ds1302_gpio_init();		//外部RTC初始化
//	ds1302_init();			
	ds1302_read_realTime(); //获取时间

	Store_Init();
	AS608_Init();    		// 指纹模块初始化

	WIFI_Init();	
    while(1)
    {   
        if(timekey[0]>=timekey[1])          //滴答定时器10ms扫描一次按键
        {
            Menu_key_set();                 //获取按键数据			
            timekey[0] = 0;                 //滴答定时器计数清零
        }       
        if(timedi[0]>=timedi[1])            //1S执行一次
        {				
            GetData();     					//读取数据
			timedi[0] = 0;
        }
        if(timedc[0]>=timedc[1])            //800ms执行一次
        {
			WIFI_Send ++;
			if(servo_state)	Servo_SetAngle(170);
			else			Servo_SetAngle(30);
		}
		
		WIFI_USE();WIFI_RX();		
        Alarm();    						// 自动控制函数
        Key_Mune();                         // 页面显示
        OLED_Update();                      // 数据更新
    }
}

void GetData(void)      	//读取数据
{
	if(Serial_GetRxFlag())
	{
		RX_data = Serial_GetRxData();
		switch (RX_data)
		{
			case 'A':
				Serial_Printf("20%04d%02d%02d%02d%02d%02d",TimeData.year,TimeData.month,TimeData.day,
											  TimeData.hour,TimeData.minute,TimeData.second);
				break;
			case 'B':
				ledmode ++; ledmode %= 4;
				switch ((uint8_t)ledmode)
				{
					case 0:Serial_SendString("6");break;
					case 1:Serial_SendString("5");break;
					case 2:Serial_SendString("4");break;
					case 3:Serial_SendString("7");break;					
				}					
				break;
			case 'C':
				fr_verify_result = 0;
				page = page_6;
				break;
			case 'D':
				Servo_SetAngle(30);
				servo_state = 0;
				Serial_SendString("3");
				break;
			case 'E'://睡眠模式
				Servo_SetAngle(30);servo_state = 0;				
				ledmode = close;LED_OFF();
				Serial_SendString("9");
				break;			
			case 'F'://退出睡眠模式
				Serial_SendString("8");
				break;			
			default:
				break;
		}			
	}
}

void Menu_key_set(void)     //按键扫描
{
	KeyNum = Get_Key();
	if(KeyNum == 1)
	{
		if(page == page_1)		page = page_2;
		else if(page == page_2) page = page_3;		
		else if(page == page_3) page = page_1;		
		else if(page == page_4) page = page_3;		
		else if(page == page_5) page = page_3;		
	}
	else if(KeyNum == 2)	{Line_select ++;Line_select %= 7;}
	else if(KeyNum == 3)
	{
		if(page == page_1)	
		{
			if(ledmode == low)		ledmode = mid;
			else if(ledmode == mid)	ledmode = max;
			else if(ledmode == max)	ledmode = close;
			else if(ledmode == close) ledmode = low;
		}
		else if(page == page_2)
		{
			switch (Line_select)
			{
				case 0:if(time_define[0] == 100)	time_define[0] = 100; 
					else	time_define[0] += 1;break;
				case 1:if(time_define[1] == 12)	time_define[1] = 0; 
					else	time_define[1] += 1;break;
				case 2:if(time_define[2] == 31)	time_define[2] = 0; 
					else	time_define[2] += 1;break;
				case 3:if(time_define[3] == 24)	time_define[3] = 0; 
					else	time_define[3] += 1;break;
				case 4:if(time_define[4] == 59)	time_define[4] = 0; 
					else	time_define[4] += 1;break;
				case 5:if(time_define[5] == 59)	time_define[5] = 0; 
					else	time_define[5] += 1;break;
				default:break;
			}			
		}
		else if(page == page_3)
		{
			uint8_t temp = Line_select % 3;
			if(temp == 0)			{page = page_4;}
			else if(temp == 1)		{page = page_5;}			
			else if(temp == 2)		{page = page_6;}			
		}
	}
	else if(KeyNum == 4)
	{
		if(page == page_1)
		{
			servo_state = 0;
			lock_state = 0;
			page = page_7;
		}
		else if(page == page_2)
		{
			switch (Line_select)
			{
				case 0:if(time_define[0] == 0)	time_define[0] = 0; 
					else	time_define[0] -= 1;break;			
				case 1:if(time_define[1] == 0)	time_define[1] = 12; 
					else	time_define[1] -= 1;break;
				case 2:if(time_define[2] == 0)	time_define[2] = 31; 
					else	time_define[2] -= 1;break;
				case 3:if(time_define[3] == 0)	time_define[3] = 24; 
					else	time_define[3] -= 1;break;
				case 4:if(time_define[4] == 0)	time_define[4] = 59; 
					else	time_define[4] -= 1;break;
				case 5:if(time_define[5] == 0)	time_define[5] = 59; 
					else	time_define[5] -= 1;break;
				case 6:time_state = 1;page = page_1;break;						
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
	ds1302_read_realTime();		//时间存于TimeData结构体中		
	if(page == page_1 && time_state == 0)	
	{
		time_define[0] = TimeData.year % 100;
		time_define[1] = TimeData.month;
		time_define[2] = TimeData.day;
		time_define[3] = TimeData.hour;
		time_define[4] = TimeData.minute;
		time_define[5] = TimeData.second;		
	}	
	if(time_state)	
	{
		time_define[0] = DecToBCD(time_define[0]);
		time_define[1] = DecToBCD(time_define[1]);
		time_define[2] = DecToBCD(time_define[2]);
		time_define[3] = DecToBCD(time_define[3]);
		time_define[4] = DecToBCD(time_define[4]);
		time_define[5] = DecToBCD(time_define[5]);
		
		time_set(time_define);time_state = 0;
	}
	
	if(ledmode == low)		LED_Set(led_low);
	else if(ledmode == mid)	LED_Set(led_mid);
	else if(ledmode == max)	LED_ON();
	else if(ledmode == close)	LED_OFF();
}

void ShowData(void)         //数据页面显示
{	
	if(page == page_1)
	{
		OLED_ShowString(34,0,"DISPLAY",OLED_8X16);
		OLED_ShowNum(8,16,TimeData.year,4,OLED_8X16);					//Time
		OLED_ShowNum(56,16,TimeData.month,2,OLED_8X16);
		OLED_ShowNum(88,16,TimeData.day,2,OLED_8X16);
		OLED_ShowNum(30,32,TimeData.hour,2,OLED_8X16);
		OLED_ShowNum(54,32,TimeData.minute,2,OLED_8X16);
		OLED_ShowNum(78,32,TimeData.second,2,OLED_8X16);
		
		OLED_ShowString(0,48,"LED:",OLED_8X16);
		OLED_DrawRectangle(74,54,4,4,OLED_FILLED);
		if(ledmode == low)		OLED_ShowString(32,48,"Low",OLED_8X16);
		else if(ledmode == mid)	OLED_ShowString(32,48,"Mid",OLED_8X16);
		else if(ledmode == max)	OLED_ShowString(32,48,"Max",OLED_8X16);
		else if(ledmode == close)	OLED_ShowString(32,48,"close",OLED_8X16);
	}
	else if(page == page_2)
	{		
		OLED_ShowNum(8,0,(time_define[0]+2000),4,OLED_8X16);			//Time
		OLED_ShowNum(56,0,time_define[1],2,OLED_8X16);
		OLED_ShowNum(88,0,time_define[2],2,OLED_8X16);
		OLED_ShowNum(30,16,time_define[3],2,OLED_8X16);
		OLED_ShowNum(54,16,time_define[4],2,OLED_8X16);
		OLED_ShowNum(78,16,time_define[5],2,OLED_8X16);		
		OLED_ShowString(0,32,"Change Time?",OLED_8X16);		
		switch (Line_select)
		{
			case 0:OLED_DrawRectangle(42,6,4,4,OLED_FILLED);break;
			case 1:OLED_DrawRectangle(74,6,4,4,OLED_FILLED);break;
			case 2:OLED_DrawRectangle(106,6,4,4,OLED_FILLED);break;
			case 3:OLED_DrawRectangle(48,22,4,4,OLED_FILLED);break;
			case 4:OLED_DrawRectangle(72,22,4,4,OLED_FILLED);break;
			case 5:OLED_DrawRectangle(96,22,4,4,OLED_FILLED);break;
			case 6:OLED_DrawRectangle(98,38,4,4,OLED_FILLED);break;
			default:break;
		}		
	}
	else if(page == page_3)
	{		
		OLED_ShowChinese(0,16,"录入指纹");
		OLED_ShowChinese(0,32,"删除指纹");
		OLED_ShowChinese(0,0,"识别指纹");
		
		uint8_t temp = Line_select % 3;
		if(temp == 0)			OLED_DrawRectangle(66,22,4,4,OLED_FILLED);
		else if(temp == 1)		OLED_DrawRectangle(66,38,4,4,OLED_FILLED);			
		else if(temp == 2)		OLED_DrawRectangle(66,6,4,4,OLED_FILLED);			
	}
	else if(page == page_4)
	{
		//录入指纹
		OLED_Clear();
		OLED_ShowChinese(0, 16, "添加模式");
		OLED_ShowChinese(0, 32, "按指纹录入"); // 提示切换到指纹操作
		OLED_Update();
		Delay_ms(200);

		int fr_add_result = 0;
		u8 newID = fingerprint_num1 + 1;

		// 修复指纹ID重复逻辑：只检查有效ID
		if(fingerprint_num1 > 0)
		{
			for(int i = 0; i < MAX_FINGERPRINTS; i++)
			{
				if(fingerprintDB[i].fingerprint_id != 0 && newID == fingerprintDB[i].fingerprint_id)
				{
					newID++;
				}
			}
		}

		Delay_ms(50);
		record_fingerprints();          // 采集指纹
		fr_add_result = Add_FR(newID);  // 添加指纹模板
		
		if(fr_add_result)
		{
			fr_add_result = SaveToFingerprintDB(newID);
			if(fr_add_result)
			{
				fingerprint_num1++;
				UpdateFingerprintDBToFlash();
			}
		}
		Delay_ms(1);

		// 指纹录入结果反馈
		OLED_Clear();
		if(fr_add_result)
		{
			OLED_ShowChinese(0, 32, "指纹录入成功");
		}
		else
		{
			OLED_ShowChinese(0, 32, "指纹录入失败");
		}
		OLED_Update();
		Delay_ms(1000);

		page = page_3;          // 自动返回菜单
	}
	else if(page == page_5)
	{
		//删除指纹
		del_all_Fingerprints();
		OLED_Clear();
		OLED_ShowChinese(0, 32, "指纹已清");
		OLED_Update();
		Delay_ms(1000);

		page = page_3;          // 自动返回菜单
	}
	else if(page == page_6)
	{
		//识别指纹
		OLED_Clear();
		OLED_ShowChinese(0, 16, "验证模式");
		OLED_ShowChinese(0, 32, "按指纹验证");
		OLED_Update();
		Delay_ms(200);

		Delay_ms(50);
		fr_verify_result = identify(); // 成功返回1，失败返回0
		Delay_ms(1);

		// 指纹结果
		OLED_Clear();
		if(fr_verify_result == 1)
		{
			OLED_ShowChinese(0, 32, "指纹验证成功");
			servo_state = (servo_state) ? 0 : 1;
			Serial_SendString("2");
		}
		else
		{
			OLED_ShowChinese(0, 32, "指纹验证失败");
			servo_state = 0;
		}
		OLED_Update();
		Delay_ms(1000);
		
		page = page_3;
	}	
	else if(page == page_7)
	{
		//识别指纹
		OLED_Clear();
		OLED_ShowString(0,20,"Locked",OLED_8X16);
		OLED_Update();
		Delay_ms(200);

		Delay_ms(50);
		lock_state = identify(); // 成功返回1，失败返回0
		Delay_ms(1);

		// 指纹结果
		OLED_Clear();
		if(lock_state == 1)
		{
			OLED_ShowChinese(0, 32, "指纹验证成功");
			OLED_Update();Delay_ms(1000);			
			servo_state = (servo_state) ? 0 : 1;
			Serial_SendString("2");
			page = page_1;
		}
		else
		{
			OLED_ShowChinese(0, 32, "指纹验证失败");
			OLED_Update();Delay_ms(1000);			
			servo_state = 0;
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
		sprintf((char*)buff,"cmd=1&uid=%s&topic=%s,%s,%s\r\n",PRODUCTID,"DATA","switch","warning");
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
	if(WIFI_Send >= 2)
	{		
		WIFI_Send = 0;
		sprintf((char*)buff,"cmd=2&uid=%s&topic=%s&msg=#%d#%d#%d#%d#%d#%d#%d#%d#\r\n",
				PRODUCTID,"DATA",TimeData.year,TimeData.month,TimeData.day,
								TimeData.hour,TimeData.minute,TimeData.second,servo_state,ledmode);		
		WifiSendStr((char*)buff);
	}
}

void WIFI_RX(void)
{    
	DataAnylize(); // 调用数据解析函数
	uint8_t time_temp[6];
	if(rx_state != 7 && rx_state != 0)
	{
		time_temp[0] = TimeData.year % 100;
		time_temp[1] = TimeData.month;
		time_temp[2] = TimeData.day;
		time_temp[3] = TimeData.hour;
		time_temp[4] = TimeData.minute;
		time_temp[5] = TimeData.second;			
	}	
	switch (rx_state)
	{
		case 1:time_temp[0]= (uint16_t)s_year % 100;break;
		case 2:time_temp[1]= s_month;break;
		case 3:time_temp[2]= s_day;break;
		case 4:time_temp[3]= s_hour;break;
		case 5:time_temp[4]= s_min;break;		
		case 6:time_temp[5]= s_second;break;
		case 7:ledmode = s_ledmode;break;
		case 8:
		{
			if(s_sleep)	//睡眠模式
			{
				Servo_SetAngle(30);servo_state = 0;				
				ledmode = close;LED_OFF();
				Serial_SendString("9");
			}
			else	//退出睡眠模式
			{
				Serial_SendString("8");		
			}
			break;
		}
		default:break;	
	}
	if(rx_state != 7 && rx_state != 8 && rx_state != 0)	
	{
		for(uint8_t i = 0; i< 6; i++)	time_temp[i] = DecToBCD(time_temp[i]);	
		time_set(time_temp);
	}
	rx_state = 0;		
}
