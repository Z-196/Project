#include "main.h"		// 枚举定义
#include "stm32f10x.h"
#include "Delay.h"
#include "math.h"
#include "oled.h"		// OLED
#include "key.h"		// KEY
#include "LED.h"		// LED
#include "Buzzer.h"		// 蜂鸣器
#include "esp8266.h"	// ESP8266
#include "HCSR501.h"
#include "dht11.h"
#include "MQ2.h"
#include "MQ4.h"
#include "MFRC522.h"
#include "relay.h"
#include "BH1750.h"

#define SELECT	1	//使用红外传感器将此值设置为2，使用热释电传感器将此值设置为1

#if	SELECT == 1
#define	PERSON_EXIST	1
#define	PERSON_NONE		0
#elif	SELECT == 2
#define	PERSON_EXIST	0
#define	PERSON_NONE		1
#endif


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

uint8_t buff[512] = {0};
uint8_t KeyNum = 0;
uint8_t Line_select = 0;
Page_enum page = page_1;

Work_enum 	work_mode = work_auto;	//工作模式

uint8_t temp;
uint8_t temp_max = 90;
uint8_t temp_min = 20;

uint8_t wet;
uint8_t wet_max = 90;
uint8_t wet_min = 20;
float mq2;
uint16_t mq2_max = 3000;
uint16_t mq2_min = 100;
float mq4;
uint16_t mq4_max = 3000;
uint16_t mq4_min = 100;
float lux;
uint16_t lux_min = 300;

Person_enum person 	  = none;		//有无行人

char KeyLock[CARD_ID_LEN] = {0xF3,0xB8,0xCA,0x30};
unsigned char cardID[CARD_ID_LEN];
uint8_t key_lock = 0;
uint8_t state_0,state_1,state_2;	//风扇、水泵、LED

uint8_t s_temp_max;
uint8_t s_temp_min;
uint8_t s_wet_max;
uint8_t s_wet_min;
uint16_t s_mq2_max;
uint16_t s_mq2_min;
uint16_t s_mq4_max;
uint16_t s_mq4_min;
uint16_t s_lux_min;

uint8_t s_state_0;
uint8_t s_state_1;
uint8_t s_state_2;

uint8_t s_work_mode;

uint8_t rx_state;

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
	LED_Init();				//LED初始化
	
	HCSR501_Init();			//热释电红外传感器初始化
	//DHT
	MQ2_Init();
	MQ4_Init();
	MFRC522_Init();
	relay_Init();
	Init_BH1750();
	
	LED_OFF();
	relay_OFF(RELAY_1);
	relay_OFF(RELAY_2);
	
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
			GetCardID(cardID);
			if(memcmp(cardID,KeyLock,CARD_ID_LEN) == 0)		//识别到指定卡号，才可解锁
			{
				key_lock = 1;page = page_10;
			}      
			timedc[0] = 0;
		}
		WIFI_USE();					
		WIFI_RX();
        Alarm();    						// 自动控制函数
        Key_Mune();                         // 页面显示
        OLED_Update();                      // 数据更新
    }
}

void GetData(void)      	//读取数据
{
	DHT11_Read();		
	temp = dat[2];
	wet  = dat[0];
	mq2 = MQ2_GetPPM(3);
	mq4 = MQ4_GetPPM(3);	
	lux = read_BH1750();
}

void Menu_key_set(void)     //按键扫描
{
	if(key_lock)
	{
		KeyNum = Get_Key();
		if(KeyNum == 1)
		{
			static uint8_t i = 0;
			if(page == page_10)	{page = page_2;i=0;}
			else
			{
				if(work_mode == work_auto)
				{
					i++;
					if(i <= 6)	page = (Page_enum)i;
					else 
					{
						if(page == page_7) 		page = page_9;
						else if(page == page_9)	{page = page_2;i=0;}
					}
				}
				else
				{
					if(page == page_2)		page = page_8;
					else if(page == page_8)	page = page_9;				
					else if(page == page_9)	page = page_2;				
				}			
			}
		}
		else if(KeyNum == 2) {Line_select ++; Line_select %= 6;}
		else if(KeyNum == 3)
		{
			if(page == page_2)		work_mode = ((uint8_t)work_mode) ? work_auto : work_hand;
			else if(page == page_7)	lux_min += 100;			
			else if(page == page_8)
			{
				uint8_t Line_temp = Line_select % 3;			
				switch (Line_temp)
				{
					case 0:state_0 = (state_0) ? 0 : 1;break;
					case 1:state_1 = (state_1) ? 0 : 1;break;
					case 2:state_2 = (state_2) ? 0 : 1;break;
					default:break;					
				}	
			}
			else
			{
				uint8_t Line_temp = Line_select % 2;			
				if(Line_temp)
				{
					if(page == page_3)	temp_max += 10;
					else if(page == page_4)	wet_max += 10;
					else if(page == page_5)	mq2_max += 100;
					else if(page == page_6)	mq4_max += 100;				
				}
				else
				{
					if(page == page_3)	temp_min += 10;
					else if(page == page_4)	wet_min += 10;
					else if(page == page_5)	mq2_min += 100;
					else if(page == page_6)	mq4_min += 100;			
				}								
			}					
		}
		else if(KeyNum == 4)
		{
			if(page == page_7)		lux_min -= 50;
			else if(page == page_9)	{key_lock = 0;page = page_1;}
			else
			{
				uint8_t Line_temp = Line_select % 2;			
				if(Line_temp)
				{
					if(page == page_3)	temp_max -= 5;
					else if(page == page_4)	wet_max -= 5;
					else if(page == page_5)	mq2_max -= 50;
					else if(page == page_6)	mq4_max -= 50;				
				}
				else
				{
					if(page == page_3)	temp_min -= 5;
					else if(page == page_4)	wet_min -= 5;
					else if(page == page_5)	mq2_min -= 50;
					else if(page == page_6)	mq4_min -= 50;			
				}								
			}
		}		
	}
}

void Key_Mune(void)         //按键页面切换
{
    OLED_Clear();
	ShowData();
}

void Alarm(void)            	//数据处理，判断数据是否异常，异常后执行的操作
{
	if(key_lock)
	{
		if(HCSR501_ReadData(HCSR1) == PERSON_EXIST)	person = exist;
		else	person = none;
		
		if(temp > temp_max || temp < temp_min
		|| wet > wet_max || wet < wet_min
		|| mq2 > mq2_max || mq2 < mq2_min
		|| mq4 > mq4_max || mq4 < mq4_min)	Buzzer_ON();
		else	Buzzer_OFF();
		
		if(work_mode == work_auto)
		{
			if( mq2 > mq2_max || mq2 < mq2_min 
			|| mq4 > mq4_max || mq4 < mq4_min)	{relay_ON(RELAY_1);relay_ON(RELAY_2);}		
			else {relay_OFF(RELAY_1);relay_OFF(RELAY_2);}
			
			if(lux < lux_min && person == exist)	LED_ON();
			else	LED_OFF();		
		}
		else
		{
			if(state_0)	relay_ON(RELAY_1);
			else		relay_OFF(RELAY_1);
			if(state_1)	relay_ON(RELAY_2);
			else		relay_OFF(RELAY_2);
			if(state_2)	LED_ON();
			else		LED_OFF();		
		}		
	}
}

void ShowData(void)         //数据页面显示
{	
	uint8_t Line_temp = Line_select % 2;
	if(page == page_1)
	{
		OLED_ShowChinese(16,16,"请刷卡解锁");
//		OLED_ShowString(0,0,"NOW_ID:",OLED_8X16);
//		OLED_ShowHexNum(0,32,cardID[0],2,OLED_8X16);
//		OLED_ShowHexNum(24,32,cardID[1],2,OLED_8X16);
//		OLED_ShowHexNum(48,32,cardID[2],2,OLED_8X16);
//		OLED_ShowHexNum(72,32,cardID[3],2,OLED_8X16);
	}
	else if(page == page_2)
	{
		OLED_ShowString(0,0,"T:",OLED_8X16);
		OLED_ShowNum(16,0,temp,3,OLED_8X16);
		OLED_ShowString(40,0,"W:",OLED_8X16);
		OLED_ShowNum(56,0,wet,3,OLED_8X16);
		if(person == exist)	OLED_ShowChinese(96,0,"有人");
		else				OLED_ShowChinese(96,0,"无人");
		OLED_ShowString(0,16,"MQ2:",OLED_8X16);
		OLED_ShowNum(32,20,mq2,5,OLED_6X8);
		OLED_ShowString(64,16,"MQ4:",OLED_8X16);
		OLED_ShowNum(96,20,mq4,5,OLED_6X8);
		
		OLED_ShowString(0,32,"Lux:",OLED_8X16);
		OLED_ShowNum(32,32,lux,5,OLED_8X16);
		OLED_ShowString(72,32,"/",OLED_8X16);
		OLED_ShowNum(80,32,lux_min,5,OLED_8X16);		
		
		OLED_ShowChinese(0,48,"工作模式：");
		if(work_mode == work_auto)	OLED_ShowChinese(80,48,"自动");
		else			OLED_ShowChinese(80,48,"手动");	
	}
	else if(page == page_3)
	{
		OLED_ShowChinese(32,0,"温度阈值设置");
		OLED_ShowString(0,16,"T:",OLED_8X16);
		OLED_ShowNum(32,16,temp,3,OLED_8X16);
		OLED_ShowString(0,32,"max:",OLED_8X16);
		OLED_ShowNum(32,32,temp_max,5,OLED_8X16);
		OLED_ShowString(0,48,"min:",OLED_8X16);
		OLED_ShowNum(32,48,temp_min,5,OLED_8X16);
		if(Line_temp)	OLED_DrawRectangle(74,38,4,4,OLED_FILLED);
		else			OLED_DrawRectangle(74,54,4,4,OLED_FILLED);	
	}
	else if(page == page_4)
	{
		OLED_ShowChinese(32,0,"湿度阈值设置");
		OLED_ShowString(0,16,"W:",OLED_8X16);
		OLED_ShowNum(32,16,wet,3,OLED_8X16);
		OLED_ShowString(0,32,"max:",OLED_8X16);
		OLED_ShowNum(32,32,wet_max,3,OLED_8X16);
		OLED_ShowString(0,48,"min:",OLED_8X16);
		OLED_ShowNum(32,48,wet_min,3,OLED_8X16);
		if(Line_temp)	OLED_DrawRectangle(74,38,4,4,OLED_FILLED);
		else			OLED_DrawRectangle(74,54,4,4,OLED_FILLED);	
	}
	else if(page == page_5)
	{
		OLED_ShowString(40,0,"MQ2",OLED_8X16);
		OLED_ShowChinese(64,0,"阈值设置");
		OLED_ShowString(0,16,"mq2:",OLED_8X16);
		OLED_ShowNum(32,16,mq2,5,OLED_8X16);
		OLED_ShowString(0,32,"max:",OLED_8X16);
		OLED_ShowNum(32,32,mq2_max,5,OLED_8X16);
		OLED_ShowString(0,48,"min:",OLED_8X16);
		OLED_ShowNum(32,48,mq2_min,5,OLED_8X16);
		if(Line_temp)	OLED_DrawRectangle(74,38,4,4,OLED_FILLED);
		else			OLED_DrawRectangle(74,54,4,4,OLED_FILLED);	
	}
	else if(page == page_6)
	{
		OLED_ShowString(40,0,"MQ4",OLED_8X16);
		OLED_ShowChinese(64,0,"阈值设置");
		OLED_ShowString(0,16,"mq4:",OLED_8X16);
		OLED_ShowNum(32,16,mq4,5,OLED_8X16);
		OLED_ShowString(0,32,"max:",OLED_8X16);
		OLED_ShowNum(32,32,mq4_max,5,OLED_8X16);
		OLED_ShowString(0,48,"min:",OLED_8X16);
		OLED_ShowNum(32,48,mq4_min,5,OLED_8X16);
		if(Line_temp)	OLED_DrawRectangle(74,38,4,4,OLED_FILLED);
		else			OLED_DrawRectangle(74,54,4,4,OLED_FILLED);	
	}
	else if(page == page_7)
	{
		OLED_ShowChinese(0,0,"光照强度阈值设置");
		OLED_ShowString(0,16,"Lux:",OLED_8X16);
		OLED_ShowNum(32,16,lux,5,OLED_8X16);
		OLED_ShowString(0,48,"min:",OLED_8X16);
		OLED_ShowNum(32,48,lux_min,5,OLED_8X16);
		OLED_DrawRectangle(74,54,4,4,OLED_FILLED);	
	}
	else if(page == page_8)
	{
		OLED_ShowChinese(32,0,"手动控制");
		OLED_ShowChinese(0,16,"风扇：");
		OLED_ShowChinese(0,32,"水泵：");
		OLED_ShowString(0,48,"LED:",OLED_8X16);
		
		uint8_t Line_temp = Line_select % 3;			
		switch (Line_temp)
		{
			case 0:OLED_DrawRectangle(66,22,4,4,OLED_FILLED);break;
			case 1:OLED_DrawRectangle(66,38,4,4,OLED_FILLED);break;
			case 2:OLED_DrawRectangle(66,54,4,4,OLED_FILLED);break;
			default:break;					
		}				
		if(state_0)		OLED_ShowChinese(48,16,"开");
		else			OLED_ShowChinese(48,16,"关");
		if(state_1)		OLED_ShowChinese(48,32,"开");
		else			OLED_ShowChinese(48,32,"关");
		if(state_2)		OLED_ShowChinese(48,48,"开");
		else			OLED_ShowChinese(48,48,"关");
	}
	else if(page == page_9)
	{
		OLED_ShowChinese(20,35,"锁定？");
		OLED_DrawLine(20,51,68,51);
	}
	else if(page == page_10)
	{
		OLED_ShowString(0,0,"NOW_ID:",OLED_8X16);
		OLED_ShowHexNum(0,16,cardID[0],2,OLED_8X16);
		OLED_ShowHexNum(24,16,cardID[1],2,OLED_8X16);
		OLED_ShowHexNum(48,16,cardID[2],2,OLED_8X16);
		OLED_ShowHexNum(72,16,cardID[3],2,OLED_8X16);
		
		OLED_ShowString(0,32,"CHECK:",OLED_8X16);
		OLED_ShowHexNum(0, 48,KeyLock[0],2,OLED_8X16);
		OLED_ShowHexNum(24,48,KeyLock[1],2,OLED_8X16);
		OLED_ShowHexNum(48,48,KeyLock[2],2,OLED_8X16);
		OLED_ShowHexNum(72,48,KeyLock[3],2,OLED_8X16);
		
		if(key_lock)	OLED_ShowChinese(64,0,"解锁成功");		
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
		sprintf((char*)buff,"cmd=2&uid=%s&topic=%s&msg=#%d#%d#%d#%d#%d#%d#%d#\r\n",
				PRODUCTID,"DATA",temp,wet,(uint16_t)mq2,(uint16_t)mq4,(uint16_t)lux,
		(uint8_t)person,(uint8_t)work_mode);
		WifiSendStr((char*)buff);
	}
}

void WIFI_RX(void)
{
	switch (rx_state)
	{
		case 1:temp_max = s_temp_max;break;
		case 2:temp_min = s_temp_min;break;
		case 3:wet_max = s_wet_max;break;
		case 4:wet_min = s_wet_min;break;
		case 5:mq2_max = s_mq2_max;break;		
		case 6:mq2_min = s_mq2_min;break;
		case 7:mq4_max = s_mq4_max;break;
		case 8:mq4_min = s_mq4_min;break;
		case 9:lux_min = s_lux_min;break;
		case 10:state_0 = s_state_0;break;
		case 11:state_1 = s_state_1;break;
		case 12:state_2 = s_state_2;break;		
		case 13:work_mode = (Work_enum)s_work_mode;break;		
		default:break;		
	}
	rx_state = 0;
}
