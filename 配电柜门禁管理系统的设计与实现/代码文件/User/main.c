#include "main.h"		// 枚举定义
#include "stm32f10x.h"
#include "Delay.h"
#include "math.h"
#include "oled.h"		// OLED
#include "key.h"		// KEY
#include "LED.h"		// LED
#include "Buzzer.h"		// 蜂鸣器
#include "Servo.h"		// SG90
#include "relay.h"		// 继电器
#include "esp8266.h"	// ESP8266

/*语音*/
#include "Serial.h"		// ASRPRO
#include "JQ8900.h"		// JQ8900

/*🔒*/
#include "AS608_use.h"	// 指纹
#include "button4_4.h"	// 矩阵键盘
#include "MFRC522.h"	// RFID

#include "Store.h"		//内部FLASH存储

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
uint8_t Line_page1 = 0;
Page_enum page = page_1;

u8 fingerprint_num1 = 0;
int fr_verify_result = 0;

uint16_t open_time;
uint8_t lock_state = 0;		//0上锁，1开锁
uint8_t warning = 0;
uint8_t err_warning = 0;
uint8_t err_count = 0;
uint8_t s_lock_state;
uint8_t s_warning;
uint32_t s_password;
uint8_t password_check;	//1代表格式错误，0代表格式正确

unsigned char cardID_Now[CARD_ID_LEN];

uint8_t save_state = 0;

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
	Buzzer_Init();			//蜂鸣器初始化	
	Servo_Init();			//舵机初始化
	relay_Init();			//继电器初始化
		
	AS608_Init();    		//指纹模块初始化
	MFRC522_Init();			//RFID模块初始化
	
	Keyboard_GPIO_Config();	//矩阵键盘初始化
	
	Init_One_line_Uart();	//JQ8900初始化
	Serial_Init();			//ASRPRO初始化	
	
	Store_Init();	
	for (int i = 0; i < MAX_CARDS; i++)			memcpy(&cardDB[i],&g_Params.cardDB[i],sizeof(CardDBItem));
	for (int i = 0; i < MAX_FINGERPRINTS; i++) 	memcpy(&fingerprintDB[i], &g_Params.fingerprintDB[i], sizeof(FingerprintInfo));   
	PasswordPanel_Init();

	
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
			static uint8_t voice_state = 1;
			if(lock_state)		//10s后自动关门并上锁
			{
				open_time++;Servo_SetAngle(170);	//开门
				relay_ON(RELAY_1);
								
				if(voice_state)		{voice_state = 0;Voice_001();}
				
				if(open_time > 150)
				{
					Servo_SetAngle(30);		//关门
					relay_OFF(RELAY_1);
					if(!voice_state){voice_state = 1;Voice_004();}
					open_time = 0;lock_state = 0;			
				}				
			}
			else	
			{
				open_time = 0;Servo_SetAngle(30);
				if(!voice_state){voice_state = 1;Voice_004();}
			}
					
			if(err_warning)
			{
				static uint8_t count = 0;
				count ++;
				if(count < 80)
				{
					Buzzer_ON();
					LED_Turn(LED1);LED_Turn(LED2);
				}
				else 
				{
					err_warning = 0;count = 0;
					Buzzer_OFF();
					LED_ON(LED1);LED_OFF(LED2);
				}
			}						
			WIFI_Send ++;
		}
		
		WIFI_USE();WIFI_RX();		
        Alarm();    						// 自动控制函数
        Key_Mune();                         // 页面显示
        OLED_Update();                      // 数据更新
    }
}

void GetData(void)      	//读取数据
{
	static uint16_t time_temppassword = 0;
	if(dynamic_password_valid)
	{
		time_temppassword ++;
		if(time_temppassword > 10)
		{time_temppassword = 0;dynamic_password_valid = 1;}		
	}
	else {dynamic_password_valid = 0;time_temppassword = 0;}
	
	if(Serial_GetRxFlag())
	{			
		if(Serial_RxPacket[4] == 'B') {lock_state = 0;Serial_SendString("1");}
		else if(Serial_RxPacket[4] == 'E')
		{
			Serial_SendString("3");
			uint8_t temp = 0;
			Delay_ms(1000);
			
			Serial_RxPacket[0] = Serial_RxPacket[0] - '0';
			Serial_RxPacket[1] = Serial_RxPacket[1] - '0';
			Serial_RxPacket[2] = Serial_RxPacket[2] - '0';
			Serial_RxPacket[3] = Serial_RxPacket[3] - '0';
			// 首先检查动态密码（优先级最高）			
			if(memcmp(Serial_RxPacket, ctx.passwords[0],PASSWORD_LENGTH) == 0)
			{lock_state = 1;page = page_6;temp = 1;}
			else if(memcmp(Serial_RxPacket, ctx.passwords[1],PASSWORD_LENGTH) == 0)
			{lock_state = 1;page = page_5;temp = 1;}
			else if(dynamic_password_valid) 
			{
				if(memcmp(Serial_RxPacket, dynamic_password,PASSWORD_LENGTH) == 0)
				{lock_state = 1;page = page_5;temp = 1;}
			}	

			if(temp)	Serial_SendString("2");
			else		Serial_SendString("4");
		}			
	}
	
	if(save_state)
	{	
		for (int i = 0; i < MAX_CARDS; i++)			memcpy(&g_Params.cardDB[i],&cardDB[i],sizeof(CardDBItem));
		for (int i = 0; i < MAX_FINGERPRINTS; i++) 	memcpy(&g_Params.fingerprintDB[i], &fingerprintDB[i], sizeof(FingerprintInfo));   
			
		// 管理员密码
		g_Params.passwords[0][0] = ctx.passwords[0][0];
		g_Params.passwords[0][1] = ctx.passwords[0][1];
		g_Params.passwords[0][2] = ctx.passwords[0][2];
		g_Params.passwords[0][3] = ctx.passwords[0][3];

		// 用户密码 
		g_Params.passwords[1][0] = ctx.passwords[1][0];
		g_Params.passwords[1][1] = ctx.passwords[1][1];
		g_Params.passwords[1][2] = ctx.passwords[1][2];
		g_Params.passwords[1][3] = ctx.passwords[1][3];		
				
		Store_Save();
		
		save_state = 0;
	}	
}

void Menu_key_set(void)     //按键扫描
{
	KeyNum = Get_Key();          //读取独立按键
    
	if(KeyNum == 1)
	{
		if(page == page_6)			page = page_7;
		else if(page == page_7)		page = page_8;
		else if(page == page_8)		page = page_9;
		else if(page == page_9)		page = page_6;		
	}
	else if(KeyNum == 2)	
	{
		if(page == page_1)	{Line_page1 ++;Line_page1 %= 4;}
		else				{Line_select ++; Line_select %= 3;}
	}
	else if(KeyNum == 3)
	{
		if(page == page_1)
		{
			if(Line_page1 == 0)		 warning = (warning) ? 0 : 1;
			else if(Line_page1 == 1) page = page_2;	//按键开锁
			else if(Line_page1 == 2) page = page_3;	//RFID开锁
			else if(Line_page1 == 3) page = page_4;	//指纹开锁
		}
		else if(page == page_5)	{lock_state = 0;page = page_1;save_state = 1;}			
		else if(page == page_6)
		{
			if(Line_select == 0)      page = page_10;	//修改用户密码
			else if(Line_select == 1) page = page_11;	//修改管理员密码
			else if(Line_select == 2) page = page_9;	//上锁					
		}
		else if(page == page_7)
		{
			if(Line_select == 0)      page = page_12;	//添加卡
			else if(Line_select == 1) page = page_13;	//删除卡
			else if(Line_select == 2) page = page_9;	//上锁		
		}
		else if(page == page_8)
		{
			if(Line_select == 0)      page = page_14;	//添加指纹
			else if(Line_select == 1) page = page_15;	//删除指纹
			else if(Line_select == 2) page = page_9;	//上锁			
		}		
		else if(page == page_9)	{lock_state = 0;page = page_1;save_state = 1;}
		else if(page == page_14)
		{
			if(Line_select == 0)      page = page_16;	//添加用户指纹
			else if(Line_select == 1) page = page_17;	//添加管理员指纹
			else if(Line_select == 2) page = page_8;	//返回管理用户指纹页面					
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
	if(lock_state)	{LED_OFF(LED1);LED_ON(LED2);}
	else			{LED_ON(LED1);LED_OFF(LED2);}
}

void ShowData(void)         //数据页面显示
{	
    // 页面1：锁定界面 + 选择开锁方式 + 3次错误报警
	if(page == page_1)
	{
		OLED_ShowString(0,0,"==LOCKED==",OLED_8X16);
		
		OLED_ShowChinese(0,16,"按键解锁");
		OLED_ShowChinese(0,32,"刷卡解锁");
		OLED_ShowChinese(0,48,"指纹解锁");
		
		// 光标跟随 Line_select
		(warning) ? OLED_ShowString(102,0,"ON",OLED_8X16) : OLED_ShowString(102,0,"OFF",OLED_8X16);
		if(Line_page1 == 0) 	 OLED_DrawRectangle(98,6,4,4,OLED_FILLED);
		else if(Line_page1 == 1) OLED_DrawRectangle(66,22,4,4,OLED_FILLED);
		else if(Line_page1 == 2) OLED_DrawRectangle(66,38,4,4,OLED_FILLED);
		else if(Line_page1 == 3) OLED_DrawRectangle(66,54,4,4,OLED_FILLED);   
		
		lock_state = 0;
		if(warning)
		{
			// 开锁失败3次蜂鸣器报警
			if(err_count >= 3)
			{
				err_count = 0;err_warning = 1;
			}		
		}
		else	{Buzzer_OFF();err_count = 0;}
	}
    
    // 页面2：矩阵键盘密码开锁
	else if(page == page_2)
	{
		bool is_admin = false;
		PasswordResult res;

		PasswordPanel_DrawInputUI();
		key_value = keyboard_scan();
		PasswordPanel_HandleKey(key_value);

		if(key_value == PASSWORD_CONFIRM_KEY)
		{
			res = PasswordPanel_VerifyPassword(&is_admin);

			if(res == PASSWORD_SUCCESS)
			{Voice_002();
				OLED_Clear();
				OLED_ShowChinese(0,20,"密码正确");
				OLED_Update();
				Delay_ms(800);
				
				if(is_admin == true)
				{
					lock_state = 1;
					page = page_6;
				}
				else
				{
					lock_state = 1;
					page = page_5;
				}
			}
			else
			{Voice_003();
				OLED_Clear();
				OLED_ShowChinese(0,20,"密码错误");
				OLED_Update();
				Delay_ms(800);
				page = page_1;err_count++;
				PasswordPanel_ClearInput();	
			}
		}

		if(key_value == 15)
		{
			page = page_1;
			PasswordPanel_ClearInput();
		}
	}
    
	// 页面3：RFID刷卡开锁
	else if(page == page_3)
	{		
		OLED_Clear();
		OLED_ShowChinese(0,20,"请刷卡");
		OLED_Update();	
		
		// 短延时，给用户放卡时间
		Delay_ms(500);
		
		uint8_t getid = GetCardID(cardID_Now);				//1、获取卡号

		// 读取卡号，0表示不存在
		if(CheckCardIDExists(cardID_Now))	
				lock_state = 1;
		else 	lock_state = 0;
		
		if(lock_state)	
		{Voice_002();
			OLED_ShowChinese(0,20,"识别成功");
			OLED_Update();
			Delay_ms(500);
			page = page_5;err_count++;
		}
		else			
		{Voice_003();
			OLED_ShowChinese(0,20,"识别失败");
			OLED_Update();
			Delay_ms(1000);
			page = page_1;err_count++;
		}
	}

    // 页面4：指纹开锁
	else if(page == page_4)
	{		
		OLED_ShowChinese(0,20,"请按指纹");
		OLED_Update();
		Delay_ms(600);
				
		if(identify())
		{Voice_002();		
			// 找到匹配的指纹
			if(fingerprint_flag == MAX_FINGERPRINTS)
			{
				lock_state = 1;	//管理员模式
				page = page_6;
			}
			else 
			{
				lock_state = 1;	//普通用户模式
				page = page_5;
			}			
		}
		else
		{Voice_003();
			OLED_Clear();
			OLED_ShowChinese(0,20,"识别失败");
			OLED_Update();
			Delay_ms(200);
			lock_state = 0;
			page = page_1;err_count++;
		}			
	}
    
    // 页面5：开锁成功 + 手动上锁 + 自动上锁
	else if(page == page_5)
	{
		OLED_Clear();
		OLED_ShowString(0,0,"OPEN",OLED_8X16);
		OLED_ShowChinese(0,20,"开锁成功");
        OLED_ShowChinese(0,40,"按键");
		OLED_ShowString(32,40,"3",OLED_8X16);
        OLED_ShowChinese(40,40,"上锁");
		OLED_Update();
	}
    
    // 页面6：管理员 - 密码管理
	else if(page == page_6)
	{
		OLED_Clear();
        OLED_ShowChinese(0,0,"管理员");
		OLED_ShowChinese(0,16,"修改用户密码");
		OLED_ShowChinese(0,32,"修改管理员密码");
		OLED_ShowChinese(0,48,"返回主页");
        
        // 光标
        if(Line_select == 0) OLED_DrawRectangle(98,22,4,4,OLED_FILLED);
        if(Line_select == 1) OLED_DrawRectangle(114,38,4,4,OLED_FILLED);
        if(Line_select == 2) OLED_DrawRectangle(66,54,4,4,OLED_FILLED);
		OLED_Update();
	}
    
    // 页面7：管理员 - 门禁卡管理
	else if(page == page_7)
	{
		OLED_Clear();
        OLED_ShowChinese(0,0,"门禁卡");
		OLED_ShowChinese(0,16,"添加卡");
		OLED_ShowChinese(0,32,"删除卡");
		OLED_ShowChinese(0,48,"返回");
        
        if(Line_select == 0) OLED_DrawRectangle(66,22,4,4,OLED_FILLED);
        if(Line_select == 1) OLED_DrawRectangle(66,38,4,4,OLED_FILLED);
        if(Line_select == 2) OLED_DrawRectangle(66,54,4,4,OLED_FILLED);
		OLED_Update();
	}
    
    // 页面8：管理员 - 指纹管理
	else if(page == page_8)
	{
		OLED_Clear();
        OLED_ShowChinese(0,0,"指纹");
		OLED_ShowChinese(0,16,"添加指纹");
		OLED_ShowChinese(0,32,"删除指纹");
		OLED_ShowChinese(0,48,"返回");
        
        if(Line_select == 0) OLED_DrawRectangle(66,22,4,4,OLED_FILLED);
        if(Line_select == 1) OLED_DrawRectangle(66,38,4,4,OLED_FILLED);
        if(Line_select == 2) OLED_DrawRectangle(66,54,4,4,OLED_FILLED);
		OLED_Update();
	}
    
    // 页面9：管理员 - 上锁
	else if(page == page_9)
	{
		OLED_ShowChinese(0,20,"上锁");
		OLED_DrawRectangle(34,26,4,4,OLED_FILLED);
	}		
	// 页面10：管理员 - 修改用户密码
	else if(page == page_10)	
	{
		PasswordResult res;

		// 绘制界面
		OLED_Clear();
		OLED_ShowChinese(0, 0, "修改用户密码");
		OLED_ShowString(0, 32, "NEW PW:", OLED_6X8);

		// 明文显示输入
		for(int i=0; i<4; i++) {
			if(i < ctx.input_index)
				OLED_ShowNum(40+i*8, 32, ctx.input_buffer[i], 1, OLED_6X8);
			else
				OLED_ShowChar(40+i*8, 32, '_', OLED_6X8);
		}

		key_value = keyboard_scan();
		PasswordPanel_HandleKey(key_value);

		// 确认修改
		if(key_value == PASSWORD_CONFIRM_KEY)
		{
			if(ctx.input_index == PASSWORD_LENGTH)
			{
				res = PasswordPanel_AddPassword(ctx.input_buffer, 1);  // 1=用户

				if(res == PASSWORD_SUCCESS)
				{
					OLED_Clear();
					OLED_ShowChinese(0,20,"用户密码修改成功");
					OLED_Update();
					Delay_ms(1000);
					page = page_6;
					PasswordPanel_ClearInput();
				}
				else
				{
					OLED_Clear();
					OLED_ShowChinese(0,20,"修改失败");
					OLED_Update();
					Delay_ms(800);
				}
			}
			else
			{
				OLED_Clear();
				OLED_ShowChinese(0,20,"请输入");
				OLED_ShowString(48,20,"4",OLED_8X16);
				OLED_ShowChinese(56,20,"位密码");
				
				OLED_Update();
				Delay_ms(800);
			}
		}

		// 返回
		if(key_value == 15)
		{
			page = page_6;
			PasswordPanel_ClearInput();
		}
	}
	// 页面11：管理员 - 修改管理员密码
	else if(page == page_11)
	{
		PasswordResult res;

		// 绘制界面：标题 + 输入框
		OLED_Clear();
		OLED_ShowChinese(0, 0, "修改管理员密码");
		OLED_ShowString(0, 32, "NEW PW:", OLED_6X8);

		// 显示输入内容：已输入显示数字，未输入显示下划线
		for(int i=0; i<4; i++) {
			if(i < ctx.input_index)
				OLED_ShowNum(40+i*8, 32, ctx.input_buffer[i], 1, OLED_6X8);
			else
				OLED_ShowChar(40+i*8, 32, '_', OLED_6X8);
		}

		// 扫描按键 + 处理输入
		key_value = keyboard_scan();
		PasswordPanel_HandleKey(key_value);

		// 确认键：保存新管理员密码
		if(key_value == PASSWORD_CONFIRM_KEY)
		{
			// 必须输入4位
			if(ctx.input_index == PASSWORD_LENGTH)
			{
				res = PasswordPanel_AddPassword(ctx.input_buffer, 0);  // 0=管理员

				if(res == PASSWORD_SUCCESS)
				{
					OLED_Clear();
					OLED_ShowChinese(0,20,"管理员");
					OLED_ShowChinese(32,36,"密码修改成功");
					
					OLED_Update();
					Delay_ms(1000);
					page = page_6;        // 返回管理员菜单
					PasswordPanel_ClearInput();
				}
				else
				{
					OLED_Clear();
					OLED_ShowChinese(0,20,"修改失败");
					OLED_Update();
					Delay_ms(800);
				}
			}
			else
			{
				// 不足4位提示
				OLED_Clear();
				OLED_ShowChinese(0,20,"请输入");
				OLED_ShowString(48,20,"4",OLED_8X16);
				OLED_ShowChinese(56,20,"位密码");
				OLED_Update();
				Delay_ms(800);
			}
		}

		// *键返回管理员菜单
		if(key_value == 15)
		{
			page = page_6;
			PasswordPanel_ClearInput();
		}	
	}
	// 页面12：管理员 - 添加卡
	else if(page == page_12)
	{
		OLED_Clear();
		OLED_ShowChinese(0,0,"添加卡");
		OLED_ShowChinese(0,20,"请放卡");
		OLED_Update();
		
		// 短延时，给用户放卡时间
		Delay_ms(500);
				
		uint8_t addResult = IC_CardAdd();		
		
		if(addResult == 1)
		{
			OLED_ClearArea(0,20,128,16);
			OLED_ShowChinese(0,20,"添加成功");
			OLED_Update();
			Delay_ms(700);
		}
		else if(addResult == 2)
		{
			OLED_ClearArea(0,20,128,16);
			OLED_ShowChinese(0,20,"已存在");
			OLED_Update();
			Delay_ms(700);
		}
		else
		{
			OLED_ClearArea(0,20,128,16);
			OLED_ShowChinese(0,20,"添加失败");
			OLED_Update();
			Delay_ms(700);
		}
		page = page_7;
	}	
	// 页面13：管理员 - 删除卡
	else if(page == page_13)
	{
		OLED_Clear();
		OLED_ShowChinese(0,0,"删除卡");
		OLED_ShowChinese(0,20,"请放卡");
		OLED_Update();
		
		// 短延时，给用户放卡时间
		Delay_ms(500);
				
		if(IC_CardDelete())		//1删除成功，0删除失败
		{
			OLED_ClearArea(0,20,128,16);
			OLED_ShowChinese(0,20,"删除成功");
			OLED_Update();
			Delay_ms(700);
		}
		else
		{
			OLED_ClearArea(0,20,128,16);
			OLED_ShowChinese(0,20,"无此卡");
			OLED_Update();
			Delay_ms(800);
		}
		page = page_7;
	}
	// 页面14：管理员 - 添加/更改指纹 - 用户/管理员
	else if(page == page_14)
	{	
		OLED_Clear();
        OLED_ShowChinese(0,0,"添加指纹");
		OLED_ShowChinese(0,16,"添加用户指纹");
		OLED_ShowChinese(0,32,"添加管理员指纹");
		OLED_ShowChinese(0,48,"返回");
        
        if(Line_select == 0) OLED_DrawRectangle(98,22,4,4,OLED_FILLED);
        if(Line_select == 1) OLED_DrawRectangle(114,38,4,4,OLED_FILLED);
        if(Line_select == 2) OLED_DrawRectangle(66,54,4,4,OLED_FILLED);
		OLED_Update();
	}
	// 页面15：管理员 - 删除用户指纹
	else if(page == page_15)
	{
		//删除指纹
		del_all_Fingerprints();
		OLED_Clear();
		OLED_ShowChinese(0, 32, "指纹已删除");
		OLED_Update();
		Delay_ms(1000);

		// 返回管理员菜单
		page = page_8;
	}
	// 页面16：管理员 - 添加用户指纹
	else if(page == page_16)
	{
		//录入指纹
		OLED_Clear();
		OLED_ShowChinese(0, 16, "添加指纹");
		OLED_ShowChinese(0, 32, "按指纹录入"); // 提示切换到指纹操作
		OLED_Update();
		Delay_ms(200);

		Delay_ms(50);
		record_fingerprints();          				// 采集指纹
		int fr_add_result = Add_FR(fingerprint_num1);  	// 添加指纹模板
	
		if(SaveToFingerprintDB(fingerprint_num1))
		{
			//1储存成功，0存储空间已满且不保存该位号指纹数据
			fingerprint_num1++;
			fingerprint_num1 %= (MAX_FINGERPRINTS - 1);				
			
			// 指纹录入结果反馈
			OLED_Clear();
			if(fr_add_result == 1)		OLED_ShowChinese(0, 32, "指纹录入成功");
			else if(fr_add_result == 0)	OLED_ShowChinese(0, 32, "指纹录入失败");
			
			OLED_Update();
			Delay_ms(1000);			
		}
		page = page_14;          // 自动返回菜单
	}

	// 页面17：管理员 - 添加管理员指纹
	else if(page == page_17)
	{
		//录入指纹
		OLED_Clear();
		OLED_ShowChinese(0, 16, "添加管理员指纹");
		OLED_ShowChinese(0, 32, "按指纹录入"); // 提示切换到指纹操作
		OLED_Update();
		Delay_ms(200);

		Delay_ms(50);
		record_fingerprints();          				// 采集指纹
		int fr_add_result = Add_FR(MAX_FINGERPRINTS);  	// 添加指纹模板
		
		SaveToFingerprintDB(0);
		fingerprintDB[0].fingerprint_id = 0;
        fingerprintDB[0].valid = 1;
		
		// 指纹录入结果反馈
		OLED_Clear();
		if(fr_add_result == 1)		OLED_ShowChinese(0, 32, "指纹录入成功");
		else if(fr_add_result == 0)	OLED_ShowChinese(0, 32, "指纹录入失败");
		
		OLED_Update();
		Delay_ms(1000);

		page = page_14;          // 自动返回菜单				
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
	if(WIFI_Send >= 2)
	{		
		WIFI_Send = 0;
		sprintf((char*)buff,"cmd=2&uid=%s&topic=%s&msg=#%d#%d#%d#%d#\r\n",
				PRODUCTID,"DATA",
		password_check,dynamic_password_valid,err_count,err_warning);	
		WifiSendStr((char*)buff);		//0无效，1有效
	}
}

void WIFI_RX(void)
{    
	DataAnylize(); // 调用数据解析函数
	switch (rx_state)
	{
		case 1:lock_state = s_lock_state;page = page_5;break;
		case 2:warning = s_warning;break;
		case 3:
		{
			if(s_password > 10000)
			{
				password_check = 1;			//格式错误
				dynamic_password_valid = 0;	//未设置动态密码
			}
			else 
			{
				password_check = 0;			//格式正确
				dynamic_password_valid = 1;	//已设置动态密码
				if(s_password > 1000)
				{
					dynamic_password[0] = s_password / 1000;
					dynamic_password[1] = s_password / 100 % 10;
					dynamic_password[2] = s_password / 10 % 10;
					dynamic_password[3] = s_password % 10;			
				}
				else if(s_password > 100)
				{
					dynamic_password[0] = 0;
					dynamic_password[1] = s_password / 100;
					dynamic_password[2] = s_password / 10 % 10;
					dynamic_password[3] = s_password % 10;					
				}
				else if(s_password > 10)
				{
					dynamic_password[0] = 0;
					dynamic_password[1] = 0;
					dynamic_password[2] = s_password / 10;
					dynamic_password[3] = s_password % 10;					
				}
				else
				{
					dynamic_password[0] = 0;
					dynamic_password[1] = 0;
					dynamic_password[2] = 0;
					dynamic_password[3] = s_password;					
				}				
			}	
			break;
		}			
		default:break;	
	}
	rx_state = 0;		
}
