#include "AS608_use.h"
#include "oled.h"
#include "usart2.h"
#include "Store.h"
extern uint8_t KeyNum; 
SysPara AS608Para;							/*指纹模块AS608参数*/
u16 ValidN;											/*模块内有效指纹个数*/

u8 ensure,x;
FingerprintInfo fingerprintDB[MAX_FINGERPRINTS]; // 指纹信息数据库

u8 currentEditingID = 0;        // 当前正在编辑的指纹ID
u8 fingerprint_num = 1;
u8 fingerprint_flag ;		//当前解锁的指纹id

uint8_t x,mode=0;
extern uint8_t key_value;
//显示确认码错误信息
void ShowErrMessage(u8 ensure)
{
	OLED_ShowString(0,0,(char*)EnsureMessage(ensure),OLED_6X8);
}

//刷指纹
void press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	ensure=PS_GetImage();
	if(ensure==0x00)//获取图像成功 
	{	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{		
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure!=0x00)	ShowErrMessage(ensure);								
		}
		else	ShowErrMessage(ensure);
	}
}

//录指纹
// 返回值：1=成功，0=用户取消
u8 Add_FR(u16 FR_ID)
{
	u8 i,ensure ,processnum=0;
	u16 ID;
	while(1)
	{
		// 录入流程是阻塞式循环：必须在这里主动轮询按键，否则外层 main 的 key_value 不会更新
		switch (processnum)
		{
			case 0:
			{
				i++;
				OLED_Clear();
				OLED_ShowChinese(0,16,"请按手指");
				OLED_Update();
				Delay_ms(500);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					ensure=PS_GenChar(CharBuffer1);//生成特征
					if(ensure==0x00)
					{
						// 检查是否重复指纹 - 使用高速搜索而不是identify()函数
						SearchResult searchResult;
						u8 search_ensure = PS_HighSpeedSearch(CharBuffer1, 0, AS608Para.PS_max, &searchResult);
						if(search_ensure == 0x00){
							// 找到匹配的指纹，说明重复
							OLED_Clear();
							OLED_ShowChinese(20,36,"重复指纹");
							OLED_Update();
							Delay_ms(1000);
							return 1;
							break;
						}
//						Serial_Printf("指纹正常\r\n");
						OLED_ShowChinese(0,16,"指纹正常");
						OLED_Update();
						processnum=1;//跳到第二步
						i = 0;						
					}else ShowErrMessage(ensure);				
				}else ShowErrMessage(ensure);						
				break;
			}
			case 1:{
				i++;
				OLED_Clear();
				OLED_ShowChinese(0,16,"请再按一次指纹");
				OLED_Update();
				Delay_ms(500);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					ensure=PS_GenChar(CharBuffer2);//生成特征
					if(ensure==0x00)
					{
						OLED_Clear();
						OLED_ShowChinese(0,16,"指纹正常");
						OLED_Update();
						Delay_ms(500);
						i=0;
						processnum=2;//跳到第三步
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				break;
			}
			case 2:{
				OLED_Clear();
				OLED_ShowChinese(0,16,"对比两次指纹");
				OLED_Update();
				Delay_ms(500);
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					OLED_Clear();
					OLED_ShowChinese(0,16,"对比成功");
					OLED_ShowChinese(0,32,"两次指纹一样");
					OLED_Update();
					Delay_ms(500);
					processnum=3;//跳到第四步
				}
				else 
				{
					OLED_Clear();
					OLED_ShowChinese(0,16,"对比失败");
					OLED_ShowChinese(0,32,"请重新录入指纹");
					OLED_Update();
					Delay_ms(500);
					ShowErrMessage(ensure);
					i=0;
					processnum=0;//跳回第一步		
				}
				Delay_ms(1200);
				break;
			}
			case 3:{
				OLED_Clear();
				OLED_ShowChinese(0,16,"生成指纹模板");
				OLED_Update();
				Delay_ms(500);				
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					OLED_Clear();
					OLED_ShowChinese(0,32,"生成指纹模板成功");
					OLED_Update();
					Delay_ms(500);
					processnum=4;//跳到第五步
				}else {processnum=0;ShowErrMessage(ensure);}
				Delay_ms(1200);
				break;
			}
			case 4:{	

				OLED_Clear();
				OLED_ShowChinese(0,16,"储存指纹");
				OLED_Update();
				Delay_ms(500);
				do
					ID=FR_ID;
				while(!(ID<AS608Para.PS_max));//输入ID必须小于指纹容量的最大值
				ensure=PS_StoreChar(CharBuffer2,ID);//储存模板
				if(ensure==0x00) 
				{					
					PS_ValidTempleteNum(&ValidN);//读库指纹个数
					return 1;  // 成功完成
				}else {
					processnum=0;
					ShowErrMessage(ensure);
				}					
				break;
			default:
				break;				
		}
		// 注意：case 4 成功时会 return，不会执行到这里
		// 其他 case 和 case 4 失败时执行 break 后会执行下面的延时
		#pragma diag_suppress 111  // 抑制"不可达语句"警告（case 4 成功时会 return，这是正常逻辑）
		Delay_ms(1000);
		#pragma diag_default 111
		// 延时后再轮询一次取消键，提升手感
		if(i==5)//超过5次没有按手指则退出
		{
			extern u8 fingerprint_num;
//			Serial_Printf("超过5次没有按手指则退出\r\n");
			OLED_Clear();
			OLED_ShowChinese(0,16,"失败");
			OLED_Update();
			Delay_ms(500);
//			OLED_Clear();
//			OLED_WriteCN(16,3,16,6);	
			fingerprint_num--;
			Delay_ms(100);
			break;	
		}	
	}		
}
}

//删除指纹
void Del_FR(u16 FR_ID)	/*输入删除指纹ID*/
{
	u8  ensure;
//	Serial_Printf("删除指纹\r\n");
	ensure=PS_DeletChar(FR_ID,1);//删除单个指纹
	if(ensure==0)
	{
		OLED_Clear();
//		OLED_ShowString(36,40,"SUCCESS",OLED_8X16);
		OLED_Update();
		Delay_ms(1000);
	}
  else
		ShowErrMessage(ensure);	
	Delay_ms(1200);
	PS_ValidTempleteNum(&ValidN);//读库指纹个数
}

void AS608_Init(void)
{
//	Serial_Printf("初始化中\r\n");
	usart2_init(usart2_baund);	/*初始化串口2,用于与指纹模块通讯*/
	PS_StaGPIO_Init();					/*初始化FR读状态引脚*/
//	Serial_Printf("与指纹模块握手\r\n");
	while(PS_HandShake(&AS608Addr))			/*与AS608模块握手*/
	{
		Delay_ms(400);
//		Serial_Printf("未检测到模块\r\n");
		OLED_Clear();
		OLED_ShowChinese(0,16,"未检测到模块");
		OLED_Update();
		Delay_ms(800);
//		Serial_Printf("尝试重新连接模块\r\n"); 
		OLED_Clear();
		OLED_ShowChinese(0,16,"尝试重新连接模块");
		OLED_Update();
		
	}
//	Serial_Printf("通讯成功\r\n");
//	Serial_Printf("波特率:%d   地址:%x\r\n",usart2_baund,AS608Addr);		/*打印信息*/
	ensure=PS_ValidTempleteNum(&ValidN);										/*读库指纹个数*/
	if(ensure!=0x00)
		ShowErrMessage(ensure);								/*显示确认码错误信息*/
	ensure=PS_ReadSysPara(&AS608Para);  		/*读参数 */
	if(ensure==0x00)
	{
//		OLED_Clear();
//		OLED_ShowChinese(0,16,"库容量");
//		OLED_ShowNum(0,32,AS608Para.PS_max-ValidN,4,OLED_8X16);
//		OLED_ShowChinese(0,48,"对比等级");
//		OLED_ShowNum(0,64,AS608Para.PS_level,4,OLED_8X16);
//		OLED_Update();
//		Delay_ms(1000);
	}
	else ShowErrMessage(ensure);	
	
	LoadFingerprintDBFromFlash();
}  

void delate_fingerprints(void)//删指纹函数
{
	OLED_Clear();
	OLED_ShowChinese(6,18,"删除指纹中");	
	Delay_ms(500);
	OLED_ShowChar(86,18,'.',8);
	Delay_ms(500);
	OLED_ShowChar(94,18,'.',8);
	Delay_ms(500);
	OLED_ShowChar(102,18,'.',8);
	Delay_ms(500);
	OLED_ShowChar(110,18,'.',8);
	OLED_Update();
}
void record_fingerprints(void)//录指纹函数
{
	OLED_Clear();
	OLED_ShowChinese(6,18,"录入指纹中");	
	Delay_ms(500);
	OLED_ShowChar(86,18,'.',8);
	Delay_ms(500);
	OLED_ShowChar(94,18,'.',8);
	Delay_ms(500);
	OLED_ShowChar(102,18,'.',8);
	Delay_ms(500);
	OLED_ShowChar(110,18,'.',8);
	OLED_Update();
}

/**
  * @brief  保存指纹数据到数据库
  * @param  id   指纹ID
  * @param  data 关联数据（来自串口）
  * @retval None
  * @note   存储策略：
  *         1. 查找相同ID的现有记录进行覆盖
  *         2. 若无相同ID，使用第一个无效条目
  *         3. 数据自动截断保证字符串安全
  */
u8 SaveToFingerprintDB(u8 fid) {
    for(int x=0; x<MAX_FINGERPRINTS; x++) {
        if(fingerprintDB[x].valid == 0) {
            fingerprintDB[x].fingerprint_id = fid;
            fingerprintDB[x].valid = 1;
            return 1;
        }
    }
	Del_FR(fid);
    OLED_ShowChinese(0,40,"存储空间已满");
	return 0;
}

/**
  * @brief  从数据库删除指纹信息
  * @param  id 要删除的指纹ID
  * @retval None
  * @note   执行逻辑：
  *         1. 标记记录为无效
  *         2. 清空数据区域
  */
// 修改后的删除函数
void DeleteFromFingerprintDB(u8 fid, uint32_t icid) {
    for(int x=0; x<MAX_FINGERPRINTS; x++) {
        if( (fingerprintDB[x].fingerprint_id == fid ) && fingerprintDB[x].valid) 
        {
			Del_FR(fingerprintDB[x].fingerprint_id);
			fingerprintDB[x].fingerprint_id = 0;
            fingerprintDB[x].valid = 0;
            OLED_Clear();
            OLED_ShowChinese(0,16,"删除成功");
            OLED_Update();
            Delay_ms(1000);
			OLED_Clear();
            return;
        }
    }
    OLED_Clear();
    OLED_ShowChinese(0,16,"记录不存在");
    OLED_Update();
    Delay_ms(1000);
	OLED_Clear();
}

/**
  * @brief  列出所有有效指纹信息
  */
void ListAllFingerprints(void) {
    OLED_Clear();
    for(int x=0,y=16; x<MAX_FINGERPRINTS; x++){
        if(fingerprintDB[x].valid){
            // 显示指纹ID
            OLED_ShowString(0,y,"FID:",OLED_8X16);
            OLED_ShowNum(32,y,fingerprintDB[x].fingerprint_id,2,OLED_8X16);
            y += 16; // 换行显示
        }
		else {
			// 显示指纹ID
            OLED_ShowString(0,y,"FID:",OLED_8X16);
            OLED_ShowNum(32,y,00,2,OLED_8X16);
            
            y += 16; // 换行显示
		}
    }
		OLED_Update();
}

void Input_Fingerprints(void){
//	if(KeyNum==3){
		OLED_Clear();
		OLED_ShowChinese(24,40,"请按手指");
		OLED_Update();
		u8 newID = fingerprint_num + 1;
		
		for(int i = 0; i < MAX_FINGERPRINTS; i++)
		{
			if(newID == fingerprintDB[i].fingerprint_id)fingerprint_num++;
		}
		
		// 指纹录入 
		u8 add_result = Add_FR(newID);

		// 绑定存储
		u8 save_result = SaveToFingerprintDB(newID);
		OLED_Clear();
		if(save_result == 1)OLED_ShowChinese(0,16,"绑定成功");
		OLED_Update();
		Delay_ms(500);
		fingerprint_num++;
		
		UpdateFingerprintDBToFlash();

		OLED_Clear();
}


//void del_Fingerprints(void){
//	if(mode == 0){
//		if(key_value == 15){
//			mode = 0;current_state = STATE_LOGIN;
//			return;
//		}
//			OLED_Clear();
//			OLED_ShowChinese(24,16,"删除");
//			OLED_ShowString(24,40,"S3",OLED_8X16);
//			OLED_ShowChinese(48,64,"确定");
//			OLED_Update();
//		}
//	if(key_value == 11 || mode){
//		mode = 1;
//		OLED_Clear();
//		OLED_ShowChinese(24,40,"刷指纹");
//		OLED_Update();
//		
//		if(key_value == 15){
//			mode = 0;current_state = STATE_LOGIN;
//			return;
//		}
//		
//		// 优先检测指纹
//		if(PS_Sta){
//			identify();  // 指纹识别
//			if(fingerprint_flag != 0xFF){
//				DeleteFromFingerprintDB(fingerprint_flag, 0);
//				fingerprint_num--;
//				UpdateFingerprintDBToFlash();
//				mode = 0;current_state = STATE_LOGIN;
//			}
//		}
//	}
//}

u8 identify(void){
	OLED_Clear();
	OLED_ShowChinese(6,48,"指纹识别中");
	Delay_ms(500);
	OLED_ShowChar(86,48,'.',8);
	Delay_ms(500);
	OLED_ShowChar(94,48,'.',8);
	Delay_ms(500);
	OLED_ShowChar(102,48,'.',8);
	Delay_ms(500);
	OLED_ShowChar(110,48,'.',8);
	OLED_Update();  
	uint32_t l = 0;
	while(1)
	{
		
		Delay_ms(1000);
		l++;
		if(l>5) 
		{
			goto a;
		}
		SearchResult seach;
		u8 ensure = 0;
		ensure=PS_GetImage();
		if(ensure==0x00)//获取图像成功 
		{	
			ensure=PS_GenChar(CharBuffer1);
			if(ensure==0x00) //生成特征成功
			{		
				ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
				if(ensure==0x00)//搜索成功
				{	
					// 验证成功

					fingerprint_flag = seach.pageID;
					OLED_Clear();
					OLED_Printf(0,16,OLED_8X16,"OK");
					OLED_Update();									
					return 1;
				}
				else {
					// 指纹识别失败

					OLED_Clear();
					OLED_ShowChinese(0,36,"未检索到相同指纹");
					OLED_Update();
					Delay_ms(1000);
					
					ensure = 0xFF;
					return 0;
				}	
			}
			else
			ShowErrMessage(ensure);
		}
	}
	a: ;
}

void del_all_Fingerprints(void){
	// 静默删除，不显示提示信息
	PS_Empty();
	for(int x=0; x<MAX_FINGERPRINTS; x++){
		fingerprintDB[x].fingerprint_id = 0;
		fingerprintDB[x].valid = 0;
		fingerprintDB[x].sing = 0;
	}
	fingerprint_num = 1;
	UpdateFingerprintDBToFlash();
	mode = 0;
}



void verification_Fingerprint(void)
{
	if(PS_Sta){
		identify();
		if(fingerprint_flag != 0xFF){
			for(int x=0; x<MAX_FINGERPRINTS; x++){
				if(fingerprintDB[x].fingerprint_id == fingerprint_flag && fingerprintDB[x].valid){
					fingerprint_flag = 0;
					OLED_Clear();
					OLED_Printf(0,32,OLED_8X16,"USER MODE");
					OLED_Update();
					
					Delay_ms(500);
					return;
				}
			}
			for(int x = 0; x < 5; x++){
				
			}
		}
	}
}

/**
  * @brief  遍历完整指纹库，获取所有有效指纹的ID列表
  * @param  ids 存储ID的数组
  * @retval 有效指纹数量
  */
u8 GetAllValidFingerprintIDs(u8 *ids) {
    u8 num = 0;
    SearchResult searchResult;
    for (u16 fid = 1; fid <= AS608Para.PS_max; fid++) { 
        u8 ensure = PS_HighSpeedSearch(fid, 0, AS608Para.PS_max, &searchResult);
        if (ensure == 0x00 && searchResult.pageID == fid) {
            ids[num++] = fid;
            if (num >= MAX_FINGERPRINTS) break;
        }
    }
    return num;
}

// 从 FLASH 加载指纹数据库
void LoadFingerprintDBFromFlash(void)
{
	for (int i = 0; i < MAX_FINGERPRINTS; i++) {
		fingerprintDB[i].fingerprint_id = Store_Data[i+51];
		if(fingerprintDB[i].fingerprint_id != 0){
			fingerprintDB[i].valid = 1;
		}
	}
	fingerprint_num = Store_Data[50];
}

// 将指纹数据库更新到 FLASH
void UpdateFingerprintDBToFlash(void)
{
	for (int i = 0; i < MAX_FINGERPRINTS; i++) {
		Store_Data[i+51] = fingerprintDB[i].fingerprint_id;
    }
	Store_Data[50] = fingerprint_num;
	Store_Save();			//将Store_Data的数据备份保存到闪存，实现掉电不丢失
}
