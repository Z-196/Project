#ifndef __MYFLASH_H
#define __MYFLASH_H
#include "stm32f10x.h"                  // Device header

// 你的宏定义
#define MAX_CARDS          4        // 最大卡号数量
#define CARD_ID_LEN        4        // 卡号长度
#define MAX_FINGERPRINTS   4       	// 你自己的指纹最大数量（自行修改）
#define PASSWORD_COUNT     2        // 密码数量：0=管理员 1=用户
#define PASSWORD_LEN       4        // 密码长度

#include "MFRC522.h"
#include "AS608_use.h"

// ====================== 核心：统一存储结构体 ======================
typedef struct {
    // 密码：2组，每组4字节 → [PASSWORD_COUNT][PASSWORD_LEN]
    uint8_t passwords[PASSWORD_COUNT][PASSWORD_LEN];
    
    // 卡号：MAX_CARDS 张卡
    CardDBItem cardDB[MAX_CARDS];
    
    // 指纹：MAX_FINGERPRINTS 个
    FingerprintInfo fingerprintDB[MAX_FINGERPRINTS];
    
} StoreParams_t;

// 声明全局实例
extern StoreParams_t g_Params;

uint32_t MyFLASH_ReadWord(uint32_t Address);
uint16_t MyFLASH_ReadHalfWord(uint32_t Address);
uint8_t MyFLASH_ReadByte(uint32_t Address);

void MyFLASH_EraseAllPages(void);
void MyFLASH_ErasePage(uint32_t PageAddress);

void MyFLASH_ProgramWord(uint32_t Address, uint32_t Data);
void MyFLASH_ProgramHalfWord(uint32_t Address, uint16_t Data);

#endif
