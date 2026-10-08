#include "Store.h"

#define STORE_START_ADDRESS		0x0800FC00		//存储的起始地址
#define STORE_COUNT				512				//存储数据的个数

uint16_t Store_Data[STORE_COUNT];				//定义SRAM数组

extern StoreParams_t g_Params;                  //全局参数结构体

/**
  * 函    数：参数存储模块初始化
  * 参    数：无
  * 返 回 值：无
  */
void Store_Init(void)
{
	/*判断是不是第一次使用*/
	if (MyFLASH_ReadHalfWord(STORE_START_ADDRESS) != 0xA5A5)	//读取第一个半字的标志位，if成立，则执行第一次使用的初始化
	{
		MyFLASH_ErasePage(STORE_START_ADDRESS);					//擦除指定页
		MyFLASH_ProgramHalfWord(STORE_START_ADDRESS, 0xA5A5);	//在第一个半字写入自己规定的标志位，用于判断是不是第一次使用
		for (uint16_t i = 1; i < STORE_COUNT; i ++)				//循环STORE_COUNT次，除了第一个标志位
		{
			MyFLASH_ProgramHalfWord(STORE_START_ADDRESS + i * 2, 0x0000);		//除了标志位的有效数据全部清0
		}
		
		// 第一次使用：设置默认密码
        g_Params.passwords[0][0] = 0;
        g_Params.passwords[0][1] = 0;
        g_Params.passwords[0][2] = 0;
        g_Params.passwords[0][3] = 0;
		
        g_Params.passwords[1][0] = 1;
        g_Params.passwords[1][1] = 2;
        g_Params.passwords[1][2] = 3;
        g_Params.passwords[1][3] = 4;
	
		// 卡号和指纹默认清空
		for (int i = 0; i < MAX_CARDS; i++) 		memset(&g_Params.cardDB[i], 0, sizeof(CardDBItem));
		for (int i = 0; i < MAX_FINGERPRINTS; i++) 	memset(&g_Params.fingerprintDB[i], 0, sizeof(FingerprintInfo));   
		
		Store_SaveParamsToCache(); 	// 将默认参数存入SRAM缓存
		Store_Save();				// 写入FLASH
	}
	else
	{
		/*上电时，将闪存数据加载回SRAM数组，实现SRAM数组的掉电不丢失*/
		for (uint16_t i = 0; i < STORE_COUNT; i ++)					//循环STORE_COUNT次，包括第一个标志位
		{
			Store_Data[i] = MyFLASH_ReadHalfWord(STORE_START_ADDRESS + i * 2);		//将闪存的数据加载回SRAM数组
		}
		
		Store_LoadParamsFromCache();
	}
}

/**
  * 函    数：将全局结构体 g_Params 复制到 SRAM缓存 Store_Data
  * 参    数：无
  * 返 回 值：无
  */
void Store_SaveParamsToCache(void)
{
    // 直接内存拷贝：结构体 → 缓存数组
    // 第一个位置是标志位 0xA5A5，数据从索引 1 开始存放
    memcpy(&Store_Data[1], &g_Params, sizeof(StoreParams_t));
}

/**
  * 函    数：从 SRAM缓存 Store_Data 恢复到 全局结构体 g_Params
  * 参    数：无
  * 返 回 值：无
  */
void Store_LoadParamsFromCache(void)
{
    // 直接内存拷贝：缓存数组 → 结构体
    memcpy(&g_Params, &Store_Data[1], sizeof(StoreParams_t));
}

/**
  * 函    数：参数存储模块保存数据到闪存
  * 参    数：无
  * 返 回 值：无
  */
void Store_Save(void)
{
    // 先把当前最新的结构体 同步到缓存
    Store_SaveParamsToCache();
    
	MyFLASH_ErasePage(STORE_START_ADDRESS);				
	for (uint16_t i = 0; i < STORE_COUNT; i ++)			
	{
		MyFLASH_ProgramHalfWord(STORE_START_ADDRESS + i * 2, Store_Data[i]);
	}
}

/**
  * 函    数：参数存储模块将所有有效数据清0
  * 参    数：无
  * 返 回 值：无
  */
void Store_Clear(void)
{
    // 全局结构体清0
    memset(&g_Params, 0, sizeof(StoreParams_t));
    
    // 同步到缓存
    Store_SaveParamsToCache();
    
    // 写入FLASH
    Store_Save();
}
