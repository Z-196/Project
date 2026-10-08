#include "main.h"		// 枚举定义
#include "stm32f10x.h"
#include "Delay.h"
#include "math.h"
#include "oled.h"		// OLED
#include "key.h"		// KEY
#include "Buzzer.h"		// 蜂鸣器
#include "LED.h"		// LED
#include "esp8266.h"	// ESP8266
#include "W25Q64.h"		// W25Q64外部存储模块
#include "MFRC522.h"	// RFID

void ShowData(void);            // 数据页面
void GetData(void);             // 获取数据
void Alarm(void);               // 自动控制函数
void Menu_key_set(void);        // 按键扫描
void Key_Mune(void);            // 按键切换页面操作
void WIFI_USE(void);
uint8_t WiFi_State = 0;			// WiFi
uint8_t WIFI_Send = 0;
void WIFI_Init(void);

uint8_t buff[512] = {0};
uint8_t KeyNum = 0;
uint8_t Line_select = 0;
uint8_t card_select = 0;
Page_enum page = page_1;
Workmode_enum workmode = workmode_in;
Workresult_enum	in_result = fail;
Workresult_enum	out_result = fail;
Workresult_enum	clear_result = fail;

/*RFID*/
unsigned char cardID[CARD_ID_LEN];
CardStatus cardStatus;
uint16_t Wait_Count = 0;

uint8_t config[MAX_CARDS+1][9];
/*
0：卡号第0位
1：卡号第1位
2：卡号第3位
3：卡号第4位

4：扩展字节

5：状态记录1
6：状态记录2
7：状态记录3

8：状态记录索引

config[MAX_CARDS][0] = cardCount
*/

int main(void)
{   
		
    SystemInit();  
    Delay_Init();           // 滴答定时器初始化
	/*控制/通信*/
    OLED_Init();            // OLED屏初始化
	OLED_Clear();	
	OLED_Update();	           
    Key_Init();             // 按键初始化
	Buzzer_Init();			// 蜂鸣器初始化
	LED_Init();				// LED初始化	
	MFRC522_Init();			// RFID初始化	
	
	W25Q64_Init();			// W25Q64初始化
	W25Q64_ReadData(0x000000, config[0], (MAX_CARDS+1)*9*sizeof(uint8_t));	//读取外部存储的数据到指定数组中	
	memcpy(cardDB,config[0],MAX_CARDS*9*sizeof(uint8_t));
    cardCount = config[MAX_CARDS][0]; 
	if(cardCount == 0xFF)	cardCount = 0;
	
	WIFI_Init();			// ESP8266初始化
	
    while(1)
    {   
        if(timekey[0]>=timekey[1])          //滴答定时器10ms扫描一次按键
        {
            Menu_key_set();                 //获取按键数据
            timekey[0] = 0;                 //滴答定时器计数清零
        }       
        if(timedi[0]>=timedi[1])            //1S执行一次
        {		
			WIFI_USE();
			
            GetData();     					//读取数据
			timedi[0] = 0;
        }
        if(timedc[0]>=timedc[1])            //1S执行一次			
        {
			if(page == page_5 || page == page_6)
			{
				if(memcmp(cardDB,config[0],(MAX_CARDS+1)*9*sizeof(uint8_t)) != 0)			//如果不相等
				{
					memcpy(config[0],cardDB,(MAX_CARDS+1)*9*sizeof(uint8_t));
					config[MAX_CARDS][0] = cardCount;
					W25Q64_SectorErase(0x000000);
					W25Q64_PageProgram(0x000000, config[0], (MAX_CARDS+1)*9*sizeof(uint8_t));
					WIFI_Send = 1;
				}	
			}
			timedc[0] = 0;
        }		
        Alarm();    						// 自动控制函数
        Key_Mune();                         // 页面显示
        OLED_Update();                      // 数据更新
    }
}

uint32_t Buzzer_count = 0;
void GetData(void)      	//读取数据
{
	//识别成功，蜂鸣器短响+绿灯（LED1）亮
	if((page == page_5 && in_result == success)||(page == page_6 && out_result == success))			
	{	
		LED_ON(LED1);
		Buzzer_count ++;
		if(Buzzer_count < 2)	Buzzer_ON();
		else 					Buzzer_OFF();
	}
	//识别失败，蜂鸣器长响+红灯（LED2）闪烁
	else if((page == page_5 && in_result == fail)||(page == page_6 && out_result == fail))	
	{
		LED_Turn(LED2);
		Buzzer_count ++;
		if(Buzzer_count < 4)	Buzzer_ON();	
		else 					Buzzer_OFF();
	}
	else 	Buzzer_OFF();
		
}

void Menu_key_set(void)     //按键扫描
{
    KeyNum = Get_Key();
    
	if(KeyNum == 1)
	{
		if(page == page_1)							page = page_2;
		else if(page == page_2)						page = page_3;
		else if(page == page_3)						page = page_1;
		else if(page == page_5 || page == page_6)	
		{
			in_result  = fail;		//默认失败
			out_result = fail;		//默认失败
			LED_OFF(LED1);
			LED_OFF(LED2);
			Buzzer_count = 0;
			page = page_1;
		}
		else if(page == page_7)						page = page_2;
		else if(page == page_9)						
		{
			clear_result = fail;	//默认失败
			page = page_3;
		}
		else if(page == page_10)					page = page_11;
		else if(page == page_11)					page = page_12;
		else if(page == page_12)					page = page_13;
		else if(page == page_13)					page = page_2;
	}
	else if(KeyNum == 2)		
	{
		Line_select++;Line_select %= 6;
		if(page == page_1)
		{
			uint8_t temp = Line_select % 2;
			if(temp == 0)		workmode = workmode_in;
			else				workmode = workmode_out;
		}
	}
	else if(KeyNum == 3)
	{
		if(page == page_1)			page = page_4;
		else if(page == page_2)		page = page_10;
		else if(page == page_10)	{page = page_7;card_select = 0;}
		else if(page == page_11)	{page = page_7;card_select = 1;}
		else if(page == page_12)	{page = page_7;card_select = 2;}
		else if(page == page_13)	{page = page_7;card_select = 3;}
	}
	else if(KeyNum == 4 && page == page_3)	page = page_8;
}

void Key_Mune(void)         //按键页面切换
{
    OLED_Clear();
	ShowData();
}

void Alarm(void)            	//数据处理，判断数据是否异常，异常后执行的操作
{ 	
	if(page == page_4)
	{
		if(workmode == workmode_in)			// 入库模式
		{	
			uint8_t getid = GetCardID(cardID);				//1、获取卡号
			if(CheckCardIDExists(cardID) == 0 && IC_CardAdd() == 0)
			{
				Wait_Count ++;		
				if(Wait_Count > 300)		//超时退出机制
				{ 	
					Wait_Count = 0;
					page = page_5;
				}
			}
			else	
			{
				in_result = success;		// 入库成功
				Card_SetStatus(cardID, CARD_STATUS_IN);		// 设置该卡的状态为【已入库】
				page = page_5;				//已添加
				Wait_Count = 0;
			}
		}
		else if(workmode == workmode_out)	//出库模式
		{
			uint8_t getid = GetCardID(cardID);				//1、获取卡号
			if(CheckCardIDExists(cardID) == 0)
			{
				Wait_Count ++;		
				if(Wait_Count > 300)		//超时退出机制
				{ 	
					Wait_Count = 0;
					page = page_6;
				}
			}
			else	
			{
				out_result = success;		// 入库成功
				Card_SetStatus(cardID, CARD_STATUS_OUT);		// 设置该卡的状态为【已入库】
				page = page_6;				//已添加
				Wait_Count = 0;
			}
		}	
	}
	else if(page == page_8)		//清除W25Q64中存储的所有数据
	{
		Wait_Count ++;

		uint8_t temp;
		W25Q64_SectorErase(0x000000);					//扇区擦除
		W25Q64_ReadData(0x000000, &temp, 1);			//读取外部存储的数据到指定数组中
		if(temp == 0xFF)
		{
			memset(cardDB,0,MAX_CARDS*9*sizeof(uint8_t));	// 卡数据库数组清零
			cardCount = 0;        							// 当前存储的卡数量清零
			clear_result = success;
			page = page_9;
			Wait_Count = 0;
			WIFI_Send = 1;
		}		
		else 		//超时退出机制
		{ 	
			
			if(Wait_Count > 300)
			{
				Wait_Count = 0;
				page = page_9;
			}			
		}
	}
}

void ShowData(void)         //数据页面显示
{
    if(page == page_1)
	{
		OLED_ShowChinese(0,0,"操作模式选择：");
		OLED_ShowChinese(0,16,"入库模式");
		OLED_ShowChinese(0,32,"出库模式");				
		if(workmode == workmode_in)		OLED_DrawLine(0,32,64,32);
		else							OLED_DrawLine(0,48,64,48);
	}
	else if(page == page_2)
	{
		OLED_ShowChinese(0,0,"物料数量：");
		OLED_ShowNum(0,80,cardCount,2,OLED_8X16);
		OLED_ShowChinese(0,32,"查看物料信息？");
		OLED_DrawLine(0,48,112,48);
	}
	else if(page == page_3)
	{
		OLED_ShowChinese(0,0,"清除本地数据？");
		OLED_ShowChinese(0,32,"需按下按键4");
		OLED_DrawLine(0,16,112,16);
	}
	else if(page == page_4)		
	{
		OLED_ShowChinese(32,32,"识别中");
		OLED_ShowNum(0,0,Wait_Count,5,OLED_8X16);	
	}
	else if(page == page_5)
	{
		if(in_result == success)	OLED_ShowChinese(16,16,"入库成功");
		else						OLED_ShowChinese(16,16,"入库失败");
	}
	else if(page == page_6)
	{
		if(out_result == success)	OLED_ShowChinese(16,16,"出库成功");
		else						OLED_ShowChinese(16,16,"出库失败");
	}
	else if(page == page_7)
	{				
		uint16_t currentIdx = card_select*6 + Line_select;
		uint8_t xPos = 0;
		uint8_t yPos = 0;
		uint8_t fontSize = OLED_6X8;
		
		OLED_ShowString(xPos, yPos, "id ", fontSize);
		xPos += 3 * 6;  // 6X8字体，每个字符占6个像素宽度
		
		OLED_ShowNum(xPos, yPos, currentIdx + 1, 2, fontSize);
		xPos += 2 * 6;
		
		OLED_ShowString(xPos, yPos, ": ", fontSize);
		xPos += 2 * 6;
		
		OLED_ShowHexNum(xPos, yPos, cardDB[currentIdx].cardID[0], 2, fontSize);
		xPos += 2 * 6;
		OLED_ShowHexNum(xPos, yPos, cardDB[currentIdx].cardID[1], 2, fontSize);
		xPos += 2 * 6;
		OLED_ShowHexNum(xPos, yPos, cardDB[currentIdx].cardID[2], 2, fontSize);
		xPos += 2 * 6;
		OLED_ShowHexNum(xPos, yPos, cardDB[currentIdx].cardID[3], 2, fontSize);
		xPos += 2 * 6;
		
		OLED_ShowChar(xPos, yPos, ' ', fontSize);
		xPos += 1 * 6;
		OLED_ShowNum(xPos, yPos, currentIdx + 1, 2, fontSize);
		
		CardStatus temp[3];        //2对应最新存入状态    
		switch (cardDB[currentIdx].statusIndex)
		{
			case 0:
				temp[2] = cardDB[currentIdx].statusRecords[2];
				temp[1] = cardDB[currentIdx].statusRecords[1];
				temp[0] = cardDB[currentIdx].statusRecords[0];
				break;
			case 1:
				temp[2] = cardDB[currentIdx].statusRecords[0];
				temp[1] = cardDB[currentIdx].statusRecords[2];
				temp[0] = cardDB[currentIdx].statusRecords[1];
				break;
			case 2:
				temp[2] = cardDB[currentIdx].statusRecords[1];
				temp[1] = cardDB[currentIdx].statusRecords[0];
				temp[0] = cardDB[currentIdx].statusRecords[2];
				break;
			default:
				break;
		}
		OLED_ShowChinese(0,16,"当前状态：");
		if(temp[2] == CARD_STATUS_UNKNOWN)    OLED_ShowChinese(80,16,"未知");
		else if(temp[2] == CARD_STATUS_IN)    OLED_ShowChinese(80,16,"入库");
		else if(temp[2] == CARD_STATUS_OUT)    OLED_ShowChinese(80,16,"出库");
		
		OLED_ShowChinese(0,32,"上次状态：");
		if(temp[1] == CARD_STATUS_UNKNOWN)    OLED_ShowChinese(80,32,"未知");
		else if(temp[1] == CARD_STATUS_IN)    OLED_ShowChinese(80,32,"入库");
		else if(temp[1] == CARD_STATUS_OUT)    OLED_ShowChinese(80,32,"出库");
		
		OLED_ShowChinese(0,48,"上上次状态：");
		if(temp[0] == CARD_STATUS_UNKNOWN)    OLED_ShowChinese(96,48,"未知");
		else if(temp[0] == CARD_STATUS_IN)    OLED_ShowChinese(96,48,"入库");
		else if(temp[0] == CARD_STATUS_OUT)    OLED_ShowChinese(96,48,"出库");        
	}
	else if(page == page_8)        OLED_ShowChinese(32,32,"清除中");
	else if(page == page_9)
	{
		if(clear_result == success)    OLED_ShowChinese(16,16,"清除成功");
		else                        OLED_ShowChinese(16,16,"清除失败");
	}
	else if(page == page_10)
	{        
		OLED_ShowChinese(0,0,"存储卡号：");
		OLED_ShowString(80,0,"1~6",OLED_8X16);        
		if(cardCount == 0)     OLED_ShowString(0, 32, "无存储卡号", OLED_8X16);
		else
		{
			for(uint8_t i = 0; i < cardCount && i < 6; i++)  // 最多显示6张卡，避免超出屏幕
			{  
				uint8_t xPos = 0;
				uint8_t yPos = 16 + i*8;
				uint8_t fontSize = OLED_6X8;				
				
				OLED_ShowString(xPos, yPos, "id ", fontSize);	// 显示 "id "
				xPos += 3 * 6;
				OLED_ShowNum(xPos, yPos, i+1, 2, fontSize);		// 显示编号 i+1
				xPos += 2 * 6;
				OLED_ShowString(xPos, yPos, ": ", fontSize);	// 显示 ": "
				xPos += 2 * 6;
				
				// 显示4个十六进制的cardID
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[0], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[1], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[2], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[3], 2, fontSize);
				xPos += 2 * 6;			
			}        
		}
		OLED_DrawRectangle(112,(16+Line_select*8),6,6,OLED_FILLED);		
	}
	else if(page == page_11)
	{
		OLED_ShowChinese(0,0,"存储卡号：");
		OLED_ShowString(80,0,"7~12",OLED_8X16);        
		if(cardCount <= 6)     OLED_ShowString(0, 32, "无存储卡号", OLED_8X16);
		else
		{
			for(uint8_t i = 6; i < cardCount && i < 12; i++)  // 最多显示6张卡，避免超出屏幕
			{  
				uint8_t xPos = 0;
				uint8_t yPos = 16 + (i-6)*8;
				uint8_t fontSize = OLED_6X8;
				
				OLED_ShowString(xPos, yPos, "id ", fontSize);
				xPos += 3 * 6;
				OLED_ShowNum(xPos, yPos, i+1, 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowString(xPos, yPos, ": ", fontSize);
				xPos += 2 * 6;
				
				// 显示4个十六进制的cardID
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[0], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[1], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[2], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[3], 2, fontSize);
				xPos += 2 * 6;
			}        
		}
		OLED_DrawRectangle(112,(16+Line_select*8),6,6,OLED_FILLED);
	}
	else if(page == page_12)
	{
		OLED_ShowChinese(0,0,"存储卡号：");
		OLED_ShowString(80,0,"13~18",OLED_8X16);        
		if(cardCount <= 12)     OLED_ShowString(0, 32, "无存储卡号", OLED_8X16);
		else
		{
			for(uint8_t i = 12; i < cardCount && i < 18; i++)  // 最多显示6张卡，避免超出屏幕
			{  
				uint8_t xPos = 0;
				uint8_t yPos = 16 + (i-12)*8;
				uint8_t fontSize = OLED_6X8;
				
				OLED_ShowString(xPos, yPos, "id ", fontSize);
				xPos += 3 * 6;
				OLED_ShowNum(xPos, yPos, i+1, 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowString(xPos, yPos, ": ", fontSize);
				xPos += 2 * 6;
				
				// 显示4个十六进制的cardID
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[0], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[1], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[2], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[3], 2, fontSize);
				xPos += 2 * 6;
			}        
		}
		OLED_DrawRectangle(112,(16+Line_select*8),6,6,OLED_FILLED);
	}
	else if(page == page_13)
	{
		OLED_ShowChinese(0,0,"存储卡号：");
		OLED_ShowString(80,0,"19~24",OLED_8X16);        
		if(cardCount <= 18)     OLED_ShowString(0, 32, "无存储卡号", OLED_8X16);
		else
		{
			for(uint8_t i = 18; i < cardCount && i < 24; i++)  // 最多显示6张卡，避免超出屏幕
			{  
				uint8_t xPos = 0;
				uint8_t yPos = 16 + (i-18)*8;
				uint8_t fontSize = OLED_6X8;
				
				OLED_ShowString(xPos, yPos, "id ", fontSize);
				xPos += 3 * 6;
				OLED_ShowNum(xPos, yPos, i+1, 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowString(xPos, yPos, ": ", fontSize);
				xPos += 2 * 6;
				
				// 显示4个十六进制的cardID
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[0], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[1], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[2], 2, fontSize);
				xPos += 2 * 6;
				OLED_ShowHexNum(xPos, yPos, cardDB[i].cardID[3], 2, fontSize);
				xPos += 2 * 6;				
			}        
		}
		OLED_DrawRectangle(112,(16+Line_select*8),6,6,OLED_FILLED);
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
	Delay_ms(500);
	WiFi_State = ConnectToBaffServer();	//连接巴法云
			
	if(WiFi_State == 1)
	{
		OLED_ShowString(20,20,"connect OK",OLED_8X16);
		OLED_Update();
		// 订阅所有必要的主题，确保能接收所有控制命令
		sprintf((char*)buff,"cmd=1&uid=%s&topic=%s,%s,%s\r\n",PRODUCTID,"DATA","switch","warning");
		WifiSendStr((char*)buff);
		Delay_ms(500);
		WiFi_State = 2;
	}
	else
	{
		OLED_ShowString(40,28,"ERROR",OLED_8X16);
	}
	OLED_Clear();
	OLED_Update();	
	WIFI_Send = 1;
}

void WIFI_USE(void)
{
	if(WIFI_Send)
	{	
		CardStatus send_state[4][3];
		for(uint8_t i = 0; i < 4; i ++)
		{
			CardStatus temp[3];        //2对应最新存入状态    
			switch (cardDB[i].statusIndex)
			{
				case 0:
					temp[2] = cardDB[i].statusRecords[2];
					temp[1] = cardDB[i].statusRecords[1];
					temp[0] = cardDB[i].statusRecords[0];
					break;           
				case 1:              
					temp[2] = cardDB[i].statusRecords[0];
					temp[1] = cardDB[i].statusRecords[2];
					temp[0] = cardDB[i].statusRecords[1];
					break;           
				case 2:              
					temp[2] = cardDB[i].statusRecords[1];
					temp[1] = cardDB[i].statusRecords[0];
					temp[0] = cardDB[i].statusRecords[2];
					break;
				default:
					break;
			}
			memcpy(send_state[i],temp,3);			
		}
		
		WIFI_Send = 0;
		sprintf((char*)buff,"cmd=2&uid=%s&topic=%s&msg=#%d#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#%02X#\r\n",
				PRODUCTID,"DATA",cardCount,
			cardDB[0].cardID[0],cardDB[0].cardID[1],cardDB[0].cardID[2],cardDB[0].cardID[3],
			send_state[0][2],send_state[0][1],send_state[0][0],
			cardDB[1].cardID[0],cardDB[1].cardID[1],cardDB[1].cardID[2],cardDB[1].cardID[3],
			send_state[1][2],send_state[1][1],send_state[1][0],
			cardDB[2].cardID[0],cardDB[2].cardID[1],cardDB[2].cardID[2],cardDB[2].cardID[3],
			send_state[2][2],send_state[2][1],send_state[2][0],
			cardDB[3].cardID[0],cardDB[3].cardID[1],cardDB[3].cardID[2],cardDB[3].cardID[3],
			send_state[3][2],send_state[3][1],send_state[3][0]);
		WifiSendStr((char*)buff);
	}
}
