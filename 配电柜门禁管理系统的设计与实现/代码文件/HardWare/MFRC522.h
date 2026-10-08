#ifndef __MFRC522_H
#define	__MFRC522_H
#include "stm32f10x.h"
#include "Delay.h"
#include <string.h>

/*************** 核心配置 ****************/
#define MAX_CARDS 4                // 最大存储卡号数量
#define CARD_ID_LEN 4              // 卡号长度（4字节）

// 卡数据库结构体（仅保留卡号）
typedef struct {
    uint8_t cardID[CARD_ID_LEN];  	// 卡号（4字节）
}CardDBItem;

// 全局变量声明
extern CardDBItem cardDB[MAX_CARDS];   // 卡数据库数组
extern unsigned char cardCount;        // 当前存储的卡数量

/*************** RC522 硬件定义（必须放在GPIO操作宏之前） ****************/
// 引脚定义（全局生效）
#define MFRC522_GPIO_SDA_PORT    GPIOB		   
#define MFRC522_GPIO_SDA_PIN     GPIO_Pin_15
											
#define MFRC522_GPIO_SCK_PORT    GPIOB		   
#define MFRC522_GPIO_SCK_PIN     GPIO_Pin_14
											
#define MFRC522_GPIO_MOSI_PORT   GPIOB	   
#define MFRC522_GPIO_MOSI_PIN    GPIO_Pin_13
											
#define MFRC522_GPIO_MISO_PORT   GPIOB		   
#define MFRC522_GPIO_MISO_PIN    GPIO_Pin_12
											
#define MFRC522_GPIO_RST_PORT    GPIOA		   
#define MFRC522_GPIO_RST_PIN     GPIO_Pin_8

// GPIO操作宏（基于上面的引脚定义）
#define MFRC522_SDA_L   GPIO_ResetBits(MFRC522_GPIO_SDA_PORT, MFRC522_GPIO_SDA_PIN)
#define MFRC522_SDA_H   GPIO_SetBits(MFRC522_GPIO_SDA_PORT, MFRC522_GPIO_SDA_PIN)
#define MFRC522_RST_L   GPIO_ResetBits(MFRC522_GPIO_RST_PORT, MFRC522_GPIO_RST_PIN)
#define MFRC522_RST_H   GPIO_SetBits(MFRC522_GPIO_RST_PORT, MFRC522_GPIO_RST_PIN)
#define MFRC522_SCK_L   GPIO_ResetBits(MFRC522_GPIO_SCK_PORT, MFRC522_GPIO_SCK_PIN)
#define MFRC522_SCK_H   GPIO_SetBits(MFRC522_GPIO_SCK_PORT, MFRC522_GPIO_SCK_PIN)
#define MFRC522_MOSI_L  GPIO_ResetBits(MFRC522_GPIO_MOSI_PORT, MFRC522_GPIO_MOSI_PIN)
#define MFRC522_MOSI_H  GPIO_SetBits(MFRC522_GPIO_MOSI_PORT, MFRC522_GPIO_MOSI_PIN)
#define MFRC522_MISO_READ  GPIO_ReadInputDataBit(MFRC522_GPIO_MISO_PORT, MFRC522_GPIO_MISO_PIN)

/*************** RC522 命令定义 ****************/
#define PCD_IDLE              0x00
#define PCD_AUTHENT           0x0E
#define PCD_TRANSCEIVE        0x0C
#define PCD_RESETPHASE        0x0F
#define PCD_CALCCRC           0x03

#define PICC_REQIDL           0x26
#define PICC_REQALL           0x52
#define PICC_ANTICOLL1        0x93
#define PICC_AUTHENT1A        0x60
#define PICC_AUTHENT1B        0x61
#define PICC_READ             0x30
#define PICC_WRITE            0xA0
#define PICC_HALT             0x50

/*************** 错误码定义 ****************/
#define MI_OK                 0x26
#define MI_NOTAGERR           0xCC
#define MI_ERR                0xBB

/*************** 函数声明 ****************/
// 基础驱动函数
void MFRC522_Init(void);
char MFRC522_Reset(void);
void MFRC522_AntennaOn(void);
void MFRC522_AntennaOff(void);
char MFRC522_Request(unsigned char req_code, unsigned char *pTagType);
char MFRC522_Anticoll(unsigned char *pSnr);
unsigned char GetCardID(unsigned char *cardID);

// 卡管理函数（仅卡号操作）
unsigned char CheckCardIDExists(unsigned char *cardID);
unsigned char IC_CardAdd(void);                
unsigned char IC_CardDelete(void);             
unsigned char IC_CardIdentify(unsigned char *cardID);
void DelAllCards(void);                        
uint8_t IC_CheckCard(void);

#endif
