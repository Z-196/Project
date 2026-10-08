#include "main.h"		// 枚举定义
#include "stm32f10x.h"
#include "MY_HSE.h"
#include "Delay.h"
#include "math.h"
#include "oled.h"		// OLED
#include "key.h"		// KEY
#include "LED.h"		// LED
#include "esp8266.h"	// ESP8266
#include "MYRTC.h"
#include "W25Q64.h"
#include "ecg.h"
#include "max30102.h"
#include "Serial.h" 

void ShowData(void);    // 数据页面
void GetData(void);     // 获取数据
void Alarm(void);       // 自动控制函数
void Menu_key_set(void);// 按键扫描
void Key_Mune(void);    // 按键切换页面操作
void WIFI_USE(void);
uint8_t WiFi_State = 0;			// WiFi
void WIFI_Init(void);
uint8_t WIFI_SEND = 0;

/*通信*/
uint8_t buff[512] = {0};
uint8_t KeyNum = 0;
uint8_t Line_select = 0;
Page_enum page = page_1;

/*状态存储*/
uint16_t LED1_count = 0;
uint16_t LED2_count = 0;

// 全局配置变量
/*
二维数组中遵循行优先存储
[行][列]
now_config[3][0];
now_config[3][1];
now_config[3][2];
now_config[3][3];
...
在存储空间中是连续的
*/

uint8_t data[3][14];
uint8_t data_state = 0;
uint8_t send_state = 0;
/*
行代表存储数据的个数
列代表单个数据的存储长度

0 ：hr0
1 ：spo2
2 ：MyRTC_Time[0]的[0~7]
3 ：MyRTC_Time[0]的[8~15]
4 ：MyRTC_Time[1]的[0~7]
5 ：MyRTC_Time[1]的[8~15]
6 ：MyRTC_Time[2]的[0~7]
7 ：MyRTC_Time[2]的[8~15]
8 ：MyRTC_Time[3]的[0~7]
9 ：MyRTC_Time[3]的[8~15]
10：MyRTC_Time[4]的[0~7]
11：MyRTC_Time[4]的[8~15]
12：MyRTC_Time[5]的[0~7]
13：MyRTC_Time[5]的[8~15]

*/

int main(void)
{   
		
    SystemInit();  
	MY_HSE_Init();
    Delay_Init();           //滴答定时器初始化
	/*控制/通信*/
    OLED_Init();            //OLED屏初始化
	OLED_Clear();	
    OLED_Update();
	
	Delay_ms(3000);
	
	Serial_Init();
	Delay_ms(500);
	WIFI_Init();			//ESP8266初始化
	Delay_ms(500);
	MyRTC_Init();			//内部RTC初始化
	Delay_ms(500);
	
	Key_Init();             //按键初始化
	LED_Init();				//LED初始化	
	max30102_init();        //max30102初始化
	max30102_blood();       //max30102数据样本处理
	W25Q64_Init();
	//读取外部存储的数据到指定数组中
	//传递首元素的地址
	W25Q64_ReadData(0x000000,*data, 14*3*sizeof(uint8_t));				

    while(1)
    {   
		if(Serial_GetRxFlag())	//接收到数据
		{
			uint8_t data_temp = Serial_GetRxData();
			if(data_temp == 1)
			{
				uint8_t temp_heart_rate = data[0][0];
				uint8_t temp_sp02 = data[0][1];		
				
				uint16_t temp_time[6]={0};
				
				temp_time[0] = (data[0][3]  << 8) | data[0][2];
				temp_time[1] = (data[0][5] << 8)  | data[0][4];
				temp_time[2] = (data[0][7] << 8)  | data[0][6];
				temp_time[3] = (data[0][9] << 8)  | data[0][8];
				temp_time[4] = (data[0][11] << 8) | data[0][10];
				temp_time[5] = (data[0][13] << 8) | data[0][12];
					
				printf("data1 心率%d血氧%d_%d年%d月%d日%d时%d分%d秒\r\n",
				temp_sp02,temp_heart_rate,
				temp_time[0],temp_time[1],temp_time[2],
				temp_time[2],temp_time[3],temp_time[4]);
			}
			else if(data_temp == 2)
			{
				uint8_t temp_heart_rate = data[1][0];
				uint8_t temp_sp02 = data[1][1];		
				
				uint16_t temp_time[6]={0};
				
				temp_time[0] = (data[1][3]  << 8) | data[1][2];
				temp_time[1] = (data[1][5] << 8)  | data[1][4];
				temp_time[2] = (data[1][7] << 8)  | data[1][6];
				temp_time[3] = (data[1][9] << 8)  | data[1][8];
				temp_time[4] = (data[1][11] << 8) | data[1][10];
				temp_time[5] = (data[1][13] << 8) | data[1][12];
				
				printf("data2 心率%d血氧%d_%d年%d月%d日%d时%d分%d秒\r\n",
				temp_sp02,temp_heart_rate,
				temp_time[0],temp_time[1],temp_time[2],
				temp_time[2],temp_time[3],temp_time[4]);
			}
			else if(data_temp == 3)
			{
				uint8_t temp_heart_rate = data[2][0];
				uint8_t temp_sp02 = data[2][1];		
				
				uint16_t temp_time[6]={0};
				
				temp_time[0] = (data[2][3]  << 8) | data[2][2];
				temp_time[1] = (data[2][5] << 8)  | data[2][4];
				temp_time[2] = (data[2][7] << 8)  | data[2][6];
				temp_time[3] = (data[2][9] << 8)  | data[2][8];
				temp_time[4] = (data[2][11] << 8) | data[2][10];
				temp_time[5] = (data[2][13] << 8) | data[2][12];
				
				printf("data3 心率%d血氧%d_%d年%d月%d日%d时%d分%d秒\r\n",
				temp_sp02,temp_heart_rate,
				temp_time[0],temp_time[1],temp_time[2],
				temp_time[2],temp_time[3],temp_time[4]);
			}
			else if(data_temp == 6)		//1+2+3
			{
				for(uint8_t i = 0; i < 3; i++)
				{
					uint8_t temp_heart_rate = data[i][0];
					uint8_t temp_sp02 = data[i][1];		
					
					uint16_t temp_time[6]={0};
					
					temp_time[0] = (data[i][3]  << 8) | data[i][2];
					temp_time[1] = (data[i][5] << 8)  | data[i][4];
					temp_time[2] = (data[i][7] << 8)  | data[i][6];
					temp_time[3] = (data[i][9] << 8)  | data[i][8];
					temp_time[4] = (data[i][11] << 8) | data[i][10];
					temp_time[5] = (data[i][13] << 8) | data[i][12];
					
					printf("data%d 心率%d血氧%d_%d年%d月%d日%d时%d分%d秒\r\n",																
					i,temp_sp02,temp_heart_rate,
					temp_time[0],temp_time[1],temp_time[2],
					temp_time[2],temp_time[3],temp_time[4]);			
				}			
			}
		}
		
        if(timekey[0]>=timekey[1])          //滴答定时器10ms扫描一次按键
        {
            Menu_key_set();                 //获取按键数据
            timekey[0] = 0;                 //滴答定时器计数清零
        }
        
        if(timedi[0]>=timedi[1])            //1S执行一次
        {		
            GetData();     					//读取数据
			
			WIFI_USE();			
			timedi[0] = 0;
        }
        if(timedc[0]>=timedc[1])            //1S执行一次
        {
				
			timedc[0] = 0;
        }
		
		MyRTC_ReadTime();			//时间存于MyRTC_Time数组中
        Alarm();    				// 自动控制函数
        Key_Mune();                 // 页面显示
        OLED_Update();              // 数据更新
    }
}

void GetData(void)      	//读取数据
{
	if(page == page_1)	heart_SpO2();	//心率血氧数据解算
	if(send_state == 1)
	{			
		send_state = 0;
		for(uint8_t i = 0; i < 3; i++)
		{
			uint8_t temp_heart_rate = data[i][0];
			uint8_t temp_sp02 = data[i][1];		
			
			uint16_t temp_time[6]={0};
			
			temp_time[0] = (data[i][3]  << 8) | data[i][2];
			temp_time[1] = (data[i][5] << 8)  | data[i][4];
			temp_time[2] = (data[i][7] << 8)  | data[i][6];
			temp_time[3] = (data[i][9] << 8)  | data[i][8];
			temp_time[4] = (data[i][11] << 8) | data[i][10];
			temp_time[5] = (data[i][13] << 8) | data[i][12];
														
			printf("data%d:%d#%d#%d#%d#%d#%d#%d#%d#\r\n",
			i,temp_heart_rate,temp_sp02,
			temp_time[0],temp_time[1],temp_time[2],
			temp_time[2],temp_time[3],temp_time[4]);		
		}	
	}	
}

void Menu_key_set(void)     //按键扫描
{
    KeyNum = Get_Key();
	if(KeyNum == 1)
	{
		if(page == page_1)		page = page_2;
		else if(page == page_2)	page = page_1;
		else if(page == page_5)	page = page_1;
		else if(page == page_6)	page = page_1;
		else if(page == page_6)	page = page_1;
		else if(page == page_7)	page = page_8;
		else if(page == page_8)	page = page_9;
		else if(page == page_9)	page = page_2;	
	}
	else if(KeyNum == 2)
	{
		if(Line_select)	Line_select = 0;
		else			Line_select = 1;
	}
	else if(KeyNum == 3)
	{
		if(page == page_1)
		{
			if(Line_select)	page = page_3;
			else			page = page_4;
		}
		else if(page == page_2)	page = page_7;
	}
}

void Key_Mune(void)         //按键页面切换
{
    OLED_Clear();
	ShowData();
}

void Alarm(void)            //数据处理，判断数据是否异常，异常后执行的操作
{	
	if(hr0 > 100)	
	{
		LED1_count ++;
		if(LED1_count > 50)		{LED1_count = 0;LED_Turn(LED1);}	
	}
	else	LED_OFF(LED1);
	
	if(hr0 < 60)	
	{
		LED2_count ++;
		if(LED2_count > 100)	{LED2_count = 0;LED_Turn(LED2);}
	}
	else	LED_OFF(LED2);
	
	if(page == page_3)
	{
		memcpy(data[2],data[1],20*sizeof(uint8_t));		//数据：1——>2
		memcpy(data[1],data[0],20*sizeof(uint8_t));		//数据：0——>1
		
		data[0][0]  = hr0;				//新数据 ——>0
		data[0][1]  = spo2;
		data[0][2]  = (uint8_t)MyRTC_Time[0];
		data[0][3]  = (uint8_t)(MyRTC_Time[0] >> 8);
		data[0][4]  = (uint8_t)MyRTC_Time[1];
		data[0][5]  = (uint8_t)(MyRTC_Time[1] >> 8);
		data[0][6]  = (uint8_t)MyRTC_Time[2];
		data[0][7]  = (uint8_t)(MyRTC_Time[2] >> 8);
		data[0][8]  = (uint8_t)MyRTC_Time[3];
		data[0][9]  = (uint8_t)(MyRTC_Time[3] >> 8);
		data[0][10] = (uint8_t)MyRTC_Time[4];
		data[0][11] = (uint8_t)(MyRTC_Time[4] >> 8);
		data[0][12] = (uint8_t)MyRTC_Time[5];
		data[0][13] = (uint8_t)(MyRTC_Time[5] >> 8);
		
		/*W25Q64功能函数*/
		W25Q64_SectorErase(0x000000);
		if(W25Q64_PageProgram(0x000000,*data,14*3*sizeof(uint8_t)))	data_state = 1;
		else	data_state = 0;
		page = page_5;		
	}
	else if(page == page_4)
	{
		memset(data,0,20*3*sizeof(uint8_t));
		W25Q64_SectorErase(0x000000);
		if(W25Q64_PageProgram(0x000000,*data,14*3*sizeof(uint8_t)))	data_state = 1;	
		else	data_state = 0;
		page = page_6;
	}
}

void ShowData(void)         //数据页面显示
{
	if(page == page_1)
	{
		/*第一行*/				
		if(WiFi_State)	OLED_ShowString(0,0,"WiFi",OLED_8X16);		//WiFi
		else			OLED_ShowString(0,0,"    ",OLED_8X16);
		OLED_ShowChinese(48,0,"存储");
		OLED_ShowChinese(96,0,"清除");
		if(Line_select)	OLED_DrawRectangle(34,2,4,4,OLED_FILLED);
		else			OLED_DrawRectangle(86,2,4,4,OLED_FILLED);		
		
		/*第二行*/
		OLED_ShowString(0,16,"S:",OLED_8X16);							//血氧
		OLED_ShowNum(16,16,spo2,3,OLED_8X16); 
		
		OLED_ShowString(64,16,"H:",OLED_8X16);							//心率
		OLED_ShowNum(80,16,hr0,3,OLED_8X16);
		
		/*第三、四行*/	
		OLED_ShowNum(8, 32,MyRTC_Time[0],4,OLED_8X16);
		OLED_ShowNum(56,32,MyRTC_Time[1],2,OLED_8X16);
		OLED_ShowNum(88,32,MyRTC_Time[2],2,OLED_8X16);
		OLED_ShowNum(30,48,MyRTC_Time[3],2,OLED_8X16);
		OLED_ShowNum(54,48,MyRTC_Time[4],2,OLED_8X16);
		OLED_ShowNum(78,48,MyRTC_Time[5],2,OLED_8X16);				
	}
	else if(page == page_2)
	{
		OLED_ShowChinese(0,24,"查看存储数据？");
		OLED_DrawLine(0,40,112,40);
	}
	else if(page == page_3)
	{
		OLED_ShowChinese(32,24,"存储中");
		OLED_ShowString(80,24,"~",OLED_8X16);
	}
	else if(page == page_4)
	{
		OLED_ShowChinese(32,24,"清除中");
		OLED_ShowString(80,24,"~",OLED_8X16);	
	}
	else if(page == page_5)
	{
		send_state = 1;
		if(data_state)	
		{
			OLED_ShowChinese(16,24,"存储成功");
			OLED_ShowString(80,24,"~",OLED_8X16);
		}
		else	OLED_ShowChinese(16,24,"存储失败！");	
	}
	else if(page == page_6)
	{
		if(data_state)	
		{
			OLED_ShowChinese(16,24,"清除成功");
			OLED_ShowString(80,24,"~",OLED_8X16);
		}
		else	OLED_ShowChinese(16,24,"清除失败！");		
	}
	else if(page == page_7)
	{
		uint8_t temp_heart_rate = data[0][0];
		uint8_t temp_sp02 = data[0][1];		
		
		uint16_t temp_time[6]={0};
		
		temp_time[0] = (data[0][3]  << 8) | data[0][2];
		temp_time[1] = (data[0][5] << 8)  | data[0][4];
		temp_time[2] = (data[0][7] << 8)  | data[0][6];
		temp_time[3] = (data[0][9] << 8)  | data[0][8];
		temp_time[4] = (data[0][11] << 8) | data[0][10];
		temp_time[5] = (data[0][13] << 8) | data[0][12];
		
		/*第一行*/
		OLED_ShowChinese(0,0,"存储数据—");
		OLED_ShowString(80,0,"1",OLED_8X16);	
	
		/*第二行*/
		OLED_ShowString(0,16,"S:",OLED_8X16);							//血氧
		OLED_ShowNum(16,16,temp_sp02,5,OLED_8X16); 
		
		OLED_ShowString(64,16,"H:",OLED_8X16);							//心率
		OLED_ShowNum(80,16,temp_heart_rate,5,OLED_8X16);
		
		/*第三、四行*/	
		OLED_ShowNum(8, 32,temp_time[0],4,OLED_8X16);
		OLED_ShowNum(56,32,temp_time[1],2,OLED_8X16);
		OLED_ShowNum(88,32,temp_time[2],2,OLED_8X16);
		OLED_ShowNum(30,48,temp_time[3],2,OLED_8X16);
		OLED_ShowNum(54,48,temp_time[4],2,OLED_8X16);
		OLED_ShowNum(78,48,temp_time[5],2,OLED_8X16);							
	}
	else if(page == page_8)
	{
		uint8_t temp_heart_rate = data[1][0];
		uint8_t temp_sp02 = data[1][1];		
		
		uint16_t temp_time[6]={0};
		
		temp_time[0] = (data[1][3]  << 8) | data[1][2];
		temp_time[1] = (data[1][5] << 8)  | data[1][4];
		temp_time[2] = (data[1][7] << 8)  | data[1][6];
		temp_time[3] = (data[1][9] << 8)  | data[1][8];
		temp_time[4] = (data[1][11] << 8) | data[1][10];
		temp_time[5] = (data[1][13] << 8) | data[1][12];
		
		/*第一行*/
		OLED_ShowChinese(0,0,"存储数据—");
		OLED_ShowString(80,0,"2",OLED_8X16);	
	
		/*第二行*/
		OLED_ShowString(0,16,"S:",OLED_8X16);							//血氧
		OLED_ShowNum(16,16,temp_sp02,5,OLED_8X16); 
		
		OLED_ShowString(64,16,"H:",OLED_8X16);							//心率
		OLED_ShowNum(80,16,temp_heart_rate,5,OLED_8X16);
		
		/*第三、四行*/	
		OLED_ShowNum(8, 32,temp_time[0],4,OLED_8X16);
		OLED_ShowNum(56,32,temp_time[1],2,OLED_8X16);
		OLED_ShowNum(88,32,temp_time[2],2,OLED_8X16);
		OLED_ShowNum(30,48,temp_time[3],2,OLED_8X16);
		OLED_ShowNum(54,48,temp_time[4],2,OLED_8X16);
		OLED_ShowNum(78,48,temp_time[5],2,OLED_8X16);							
	}
	else if(page == page_9)
	{
		uint8_t temp_heart_rate = data[2][0];
		uint8_t temp_sp02 = data[2][1];		
		
		uint16_t temp_time[6]={0};
		
		temp_time[0] = (data[2][3]  << 8) | data[2][2];
		temp_time[1] = (data[2][5] << 8)  | data[2][4];
		temp_time[2] = (data[2][7] << 8)  | data[2][6];
		temp_time[3] = (data[2][9] << 8)  | data[2][8];
		temp_time[4] = (data[2][11] << 8) | data[2][10];
		temp_time[5] = (data[2][13] << 8) | data[2][12];
		
		/*第一行*/
		OLED_ShowChinese(0,0,"存储数据—");
		OLED_ShowString(80,0,"3",OLED_8X16);	
	
		/*第二行*/
		OLED_ShowString(0,16,"S:",OLED_8X16);							//血氧
		OLED_ShowNum(16,16,temp_sp02,5,OLED_8X16); 
		
		OLED_ShowString(64,16,"H:",OLED_8X16);							//心率
		OLED_ShowNum(80,16,temp_heart_rate,5,OLED_8X16);
		
		/*第三、四行*/	
		OLED_ShowNum(8, 32,temp_time[0],4,OLED_8X16);
		OLED_ShowNum(56,32,temp_time[1],2,OLED_8X16);
		OLED_ShowNum(88,32,temp_time[2],2,OLED_8X16);
		OLED_ShowNum(30,48,temp_time[3],2,OLED_8X16);
		OLED_ShowNum(54,48,temp_time[4],2,OLED_8X16);
		OLED_ShowNum(78,48,temp_time[5],2,OLED_8X16);							
	}
}

void WIFI_Init(void)
{
	OLED_ShowChinese(0,0,"联网中");
	Delay_ms(200);
	OLED_Update();
    ESP8266_Config();
    Delay_ms(300);
	ConnectToHotspot();
    Delay_ms(2000);
	uint8_t time_ret = ESP8266_GetNTPTime();
	if(time_ret == 0)
    {
        // 把网络时间拷贝到 RTC 数组
        memcpy(MyRTC_Time, Timer, 6*sizeof(uint16_t));
		OLED_ShowString(0,30,"Time Get Success",OLED_8X16);
    }
	else
	{
		OLED_ShowString(0,30,"ERROR",OLED_8X16);
	}	
	OLED_Update();
	
//	WiFi_State = ConnectToBaffServer();	//连接巴法云			
//	if(WiFi_State == 1)
//	{
//		OLED_ShowString(20,60,"connect OK",OLED_8X16);
//		// 订阅所有必要的主题，确保能接收所有控制命令
//		sprintf((char*)buff,"cmd=1&uid=%s&topic=%s,%s,%s\r\n",PRODUCTID,"DATA","switch","warning");
//		WifiSendStr((char*)buff);
//		WiFi_State = 2;
//	}
//	else OLED_ShowString(40,60,"ERROR",OLED_8X16);
//	OLED_Update();
	
	Delay_ms(2000);
	OLED_Clear();
	OLED_Update();		
}

void WIFI_USE(void)
{
	if(WIFI_SEND == 1)
	{
		uint8_t temp_heart_rate = data[0][0];
		uint8_t temp_sp02 = data[0][1];		
		
		uint16_t temp_time[6]={0};
		
		temp_time[0] = (data[0][3]  << 8) | data[0][2];
		temp_time[1] = (data[0][5] << 8)  | data[0][4];
		temp_time[2] = (data[0][7] << 8)  | data[0][6];
		temp_time[3] = (data[0][9] << 8)  | data[0][8];
		temp_time[4] = (data[0][11] << 8) | data[0][10];
		temp_time[5] = (data[0][13] << 8) | data[0][12];
				
		sprintf((char*)buff,"cmd=2&uid=%s&topic=%s&msg=#%d#%d#%d#%d#%d#%d#%d#%d#\r\n",
					PRODUCTID,"DATA",temp_sp02,temp_heart_rate,
		temp_time[0],temp_time[1],temp_time[2],
		temp_time[2],temp_time[3],temp_time[4]);
		WifiSendStr((char*)buff);		
		WIFI_SEND = 0;
	}
	else if(WIFI_SEND == 2)
	{
		uint8_t temp_heart_rate = data[1][0];
		uint8_t temp_sp02 = data[1][1];		
		
		uint16_t temp_time[6]={0};
		
		temp_time[0] = (data[1][3]  << 8) | data[1][2];
		temp_time[1] = (data[1][5] << 8)  | data[1][4];
		temp_time[2] = (data[1][7] << 8)  | data[1][6];
		temp_time[3] = (data[1][9] << 8)  | data[1][8];
		temp_time[4] = (data[1][11] << 8) | data[1][10];
		temp_time[5] = (data[1][13] << 8) | data[1][12];
		
		sprintf((char*)buff,"cmd=2&uid=%s&topic=%s&msg=#%d#%d#%d#%d#%d#%d#%d#%d#\r\n",
					PRODUCTID,"DATA",temp_sp02,temp_heart_rate,
		temp_time[0],temp_time[1],temp_time[2],
		temp_time[2],temp_time[3],temp_time[4]);
		WifiSendStr((char*)buff);		
		WIFI_SEND = 0;
	}
	else if(WIFI_SEND == 3)
	{
		uint8_t temp_heart_rate = data[2][0];
		uint8_t temp_sp02 = data[2][1];		
		
		uint16_t temp_time[6]={0};
		
		temp_time[0] = (data[2][3]  << 8) | data[2][2];
		temp_time[1] = (data[2][5] << 8)  | data[2][4];
		temp_time[2] = (data[2][7] << 8)  | data[2][6];
		temp_time[3] = (data[2][9] << 8)  | data[2][8];
		temp_time[4] = (data[2][11] << 8) | data[2][10];
		temp_time[5] = (data[2][13] << 8) | data[2][12];
		
		sprintf((char*)buff,"cmd=2&uid=%s&topic=%s&msg=#%d#%d#%d#%d#%d#%d#%d#%d#\r\n",
					PRODUCTID,"DATA",temp_sp02,temp_heart_rate,
		temp_time[0],temp_time[1],temp_time[2],
		temp_time[2],temp_time[3],temp_time[4]);
		WifiSendStr((char*)buff);		
		WIFI_SEND = 0;
	}
}

