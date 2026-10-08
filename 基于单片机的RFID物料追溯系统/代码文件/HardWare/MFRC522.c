#include "MFRC522.h"
#include "oled.h"
#include <string.h>

#define MAXRLEN 18
#define RC522_DELAY()  Delay_us(2)

// 全局变量定义（仅存于内存，掉电丢失）
unsigned char cardCount = 0;          // 当前存储卡号数量
CardDBItem cardDB[MAX_CARDS] = {0};   // 卡数据库数组

// 从MFRC522寄存器读一个字节
unsigned char Read_MFRC522(unsigned char Address)
{
    unsigned char i, ucAddr;
    unsigned char ucResult=0;

    MFRC522_SCK_L; RC522_DELAY();
    MFRC522_SDA_L; RC522_DELAY();
    ucAddr = ((Address<<1)&0x7E)|0x80;

    // 修复：确保MFRC522_MOSI_H/L宏正确展开
    for(i=8;i>0;i--)
    {
        if(ucAddr & 0x80) {
            MFRC522_MOSI_H; // 显式if-else，避免三元运算符可能的编译问题
        } else {
            MFRC522_MOSI_L;
        }
        RC522_DELAY();
        MFRC522_SCK_H; RC522_DELAY();
        ucAddr <<= 1;
        MFRC522_SCK_L; RC522_DELAY();
    }

    // 修复：确保MFRC522_MISO_READ宏正确展开
    for(i=8;i>0;i--)
    {
        MFRC522_SCK_H; RC522_DELAY();
        ucResult <<= 1;
        ucResult |= MFRC522_MISO_READ; // 读取MISO引脚值
        MFRC522_SCK_L; RC522_DELAY();
    }

    MFRC522_SCK_H; RC522_DELAY();
    MFRC522_SDA_H; RC522_DELAY();
    return ucResult;
}

// 向MFRC522寄存器写一个字节
void Write_MFRC522(unsigned char Address, unsigned char value)
{  
    unsigned char i, ucAddr;

    MFRC522_SCK_L;
    MFRC522_SDA_L;
    ucAddr = ((Address<<1)&0x7E);

    // 修复：替换三元运算符为if-else，避免编译识别问题
    for(i=8;i>0;i--)
    {
        if(ucAddr & 0x80) {
            MFRC522_MOSI_H;
        } else {
            MFRC522_MOSI_L;
        }
        RC522_DELAY();
        MFRC522_SCK_H; RC522_DELAY();
        ucAddr <<= 1;
        MFRC522_SCK_L; RC522_DELAY();
    }

    for(i=8;i>0;i--)
    {
        if(value & 0x80) {
            MFRC522_MOSI_H;
        } else {
            MFRC522_MOSI_L;
        }
        RC522_DELAY();
        MFRC522_SCK_H; RC522_DELAY();
        value <<= 1;
        MFRC522_SCK_L; RC522_DELAY();
    }

    MFRC522_SCK_H; RC522_DELAY();
    MFRC522_SDA_H; RC522_DELAY();
}

// 置RC522寄存器位
void SetBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp | mask);
}

// 清RC522寄存器位
void ClearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & ~mask);
}

// RC522与卡片通讯
char MFRC522_ToCard(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    
    switch (Command)
    {
       case PCD_AUTHENT:
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
   
    Write_MFRC522(0x02, irqEn|0x80);
    ClearBitMask(0x04, 0x80);
    Write_MFRC522(0x01, PCD_IDLE);
    SetBitMask(0x0A, 0x80);
    
    for (i=0; i<InLenByte; i++)
    {   
        Write_MFRC522(0x09, pInData[i]);
    }
    Write_MFRC522(0x01, Command);
   
    if (Command == PCD_TRANSCEIVE)
    {    
        SetBitMask(0x0D, 0x80);
    }
    
    n = Read_MFRC522(0x04);
    i = 1500;
    do 
    {
        n = Read_MFRC522(0x04);
        i--;
    } while ((i!=0) && !(n&0x01) && !(n&waitFor));
    
    ClearBitMask(0x0D, 0x80);
      
    if (i!=0)
    {    
        if(!(Read_MFRC522(0x06)&0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   status = MI_NOTAGERR;   }
            
            if (Command == PCD_TRANSCEIVE)
            {
                n = Read_MFRC522(0x0A);
                lastBits = Read_MFRC522(0x0C) & 0x07;
                *pOutLenBit = lastBits ? (n-1)*8 + lastBits : n*8;
                
                n = n > MAXRLEN ? MAXRLEN : n;
                for (i=0; i<n; i++)
                {   pOutData[i] = Read_MFRC522(0x09);    }
            }
        }
        else
        {   status = MI_ERR;   }
    }
   
    SetBitMask(0x0C, 0x80);
    Write_MFRC522(0x01, PCD_IDLE); 
    return status;
}

// RC522初始化
void MFRC522_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    // 配置SDA/SCK/MOSI为推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    
    GPIO_InitStructure.GPIO_Pin = MFRC522_GPIO_SDA_PIN;
    GPIO_Init(MFRC522_GPIO_SDA_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = MFRC522_GPIO_SCK_PIN;
    GPIO_Init(MFRC522_GPIO_SCK_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = MFRC522_GPIO_MOSI_PIN;
    GPIO_Init(MFRC522_GPIO_MOSI_PORT, &GPIO_InitStructure);
    
    // 配置MISO为浮空输入
    GPIO_InitStructure.GPIO_Pin = MFRC522_GPIO_MISO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(MFRC522_GPIO_MISO_PORT, &GPIO_InitStructure);
    
    // 配置RST为推挽输出
    GPIO_InitStructure.GPIO_Pin = MFRC522_GPIO_RST_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(MFRC522_GPIO_RST_PORT, &GPIO_InitStructure);
    
    // 硬件复位RC522
    MFRC522_Reset();
    
    // 开启天线
    MFRC522_AntennaOn();
}

// RC522复位
char MFRC522_Reset(void) 
{
    MFRC522_RST_H; Delay_us(1);             
    MFRC522_RST_L; Delay_us(1);                         
    MFRC522_RST_H; Delay_us(1);        	

    Write_MFRC522(0x01, 0x0F);
    while(Read_MFRC522(0x01) & 0x10);

    Write_MFRC522(0x11, 0x3D);
    Write_MFRC522(0x2D, 30);
    Write_MFRC522(0x2C, 0);
    Write_MFRC522(0x2A, 0x8D);
    Write_MFRC522(0x2B, 0x3E);
    Write_MFRC522(0x15, 0x40);
    
    return MI_OK;
}

// 开启天线
void MFRC522_AntennaOn(void)
{
    unsigned char i = Read_MFRC522(0x14);
    if (!(i & 0x03))
    {
        SetBitMask(0x14, 0x03);
    }
}

// 关闭天线
void MFRC522_AntennaOff(void)
{
    ClearBitMask(0x14, 0x03);
}

// 寻卡
char MFRC522_Request(unsigned char req_code, unsigned char *pTagType)
{
    char status;  
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ClearBitMask(0x08, 0x08);
    Write_MFRC522(0x0D, 0x07);
    SetBitMask(0x14, 0x03);
 
    ucComMF522Buf[0] = req_code;

    status = MFRC522_ToCard(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
   
    if ((status == MI_OK) && (unLen == 0x10))
    {    
        *pTagType     = ucComMF522Buf[0];
        *(pTagType+1) = ucComMF522Buf[1];
    }
    else
    {   status = MI_ERR;  }
   
    return status;
}

// 防冲突检测（获取卡号）
char MFRC522_Anticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i, snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ClearBitMask(0x08, 0x08);
    Write_MFRC522(0x0D, 0x00);
    ClearBitMask(0x0E, 0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = MFRC522_ToCard(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        for (i=0; i<4; i++)
        {   
            *(pSnr+i)  = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {   status = MI_ERR;    }
    }
    
    SetBitMask(0x0E, 0x80);
    return status;
}

// 获取当前识别到的卡号
unsigned char GetCardID(unsigned char *cardID)
{
    char status;
    unsigned char TagType[2];  // 必须单独用数组存卡类型！！！

    // 清空卡号
    memset(cardID, 0, 4);

    // 1. 寻卡 → 输出到 TagType，不破坏 cardID
    status = MFRC522_Request(PICC_REQALL, TagType);
    if(status != MI_OK)
    {
        return 0;
    }

    // 2. 防冲突 → 读取 4 字节真实卡号
    status = MFRC522_Anticoll(cardID);
    if(status != MI_OK)
    {
        return 0;
    }

    return 1;
}



//// 获取当前识别到的卡号
//unsigned char GetCardID(unsigned char *cardID) {
//    char status;

//    // 请求寻卡
//    status = MFRC522_Request(PICC_REQALL, cardID);
//    if (status != MI_OK) {
//        MFRC522_Reset();
//        MFRC522_AntennaOff(); 
//        MFRC522_AntennaOn(); 
//        return 0;
//    }

//    // 防冲突（获取卡号）
//    status = MFRC522_Anticoll(cardID);
//    if (status != MI_OK) {    
//        return 0;    
//    }

//    return 1;
//}

// 检查卡号是否存在
unsigned char CheckCardIDExists(unsigned char *cardID) {
    for (unsigned char i = 0; i < cardCount; i++) {
        if (memcmp(cardDB[i].cardID, cardID, CARD_ID_LEN) == 0) {
            return i+1;  // 返回索引+1（0表示不存在）
        }
    }
    return 0;
}

// 添加卡到数据库
unsigned char IC_CardAdd(void) {
    unsigned char cardID[CARD_ID_LEN];
    unsigned char existIndex;

    // 获取卡号
    if (!GetCardID(cardID)) {
        return 0; // 未识别到卡
    }

    // 检查数据库是否已满
    if (cardCount >= MAX_CARDS) {
        return 0; // 数据库满
    }

    // 检查卡号是否已存在
    existIndex = CheckCardIDExists(cardID);
    if (existIndex) {
        cardDB[existIndex-1].x++;
        return 2; // 已存在
    }

    // 添加新卡到数据库
    memcpy(cardDB[cardCount].cardID, cardID, CARD_ID_LEN);
    cardDB[cardCount].x = 1;
    
    // 初始化状态记录
    for(uint8_t j=1; j<STATUS_RECORD_CNT; j++){
        cardDB[cardCount].statusRecords[j] = CARD_STATUS_UNKNOWN;
    }
    cardDB[cardCount].statusIndex = 0;
    
    cardCount++;
    return 1; // 添加成功
}

// 从数据库删除卡
unsigned char IC_CardDelete(void) {
    unsigned char cardID[CARD_ID_LEN];
    unsigned char i, j;

    // 获取卡号
    if (!GetCardID(cardID)) {
        return 0; // 未识别到卡
    }

    // 查找并删除
    for (i = 0; i < cardCount; i++) {
        if (memcmp(cardDB[i].cardID, cardID, CARD_ID_LEN) == 0) {
            // 后续卡前移
            for (j = i; j < cardCount-1; j++) {
                memcpy(&cardDB[j], &cardDB[j+1], sizeof(CardDBItem));
            }
            cardCount--;
            return 1; // 删除成功
        }
    }

    return 0; // 卡号不存在
}

// 识别卡（返回索引）
unsigned char IC_CardIdentify(unsigned char *cardID) {
    for (unsigned char i = 0; i < cardCount; i++) {
        if (memcmp(cardDB[i].cardID, cardID, CARD_ID_LEN) == 0) {
            return i; // 返回索引
        }
    }
    return MAX_CARDS; // 未找到
}

// 删除所有卡
void DelAllCards(void) {
    // 清空数据库
    memset(cardDB, 0, sizeof(cardDB));
    cardCount = 0;
}

// 设置卡状态（入库/出库）
uint8_t Card_SetStatus(unsigned char *cardID, CardStatus status) {
    // 查找卡索引
    uint8_t cardIndex = IC_CardIdentify(cardID);
    if(cardIndex >= MAX_CARDS) {
        return 0; // 卡号不存在
    }
    
    // 写入状态（循环覆盖）
    cardDB[cardIndex].statusRecords[cardDB[cardIndex].statusIndex] = status;
    // 更新索引（0→1→2→0）
    cardDB[cardIndex].statusIndex = (cardDB[cardIndex].statusIndex + 1) % STATUS_RECORD_CNT;
    
    return 1; // 设置成功
}

// 获取卡最后一次状态
CardStatus Card_GetLastStatus(unsigned char *cardID) {
    uint8_t cardIndex = IC_CardIdentify(cardID);
    if(cardIndex >= MAX_CARDS) {
        return CARD_STATUS_UNKNOWN;
    }
    
    // 计算最后一次状态索引
    uint8_t lastIndex = (cardDB[cardIndex].statusIndex == 0) ? (STATUS_RECORD_CNT - 1) : (cardDB[cardIndex].statusIndex - 1);
    return cardDB[cardIndex].statusRecords[lastIndex];
}

// 清空卡所有状态记录
void Card_ClearAllStatus(unsigned char *cardID) {
    uint8_t cardIndex = IC_CardIdentify(cardID);
    if(cardIndex >= MAX_CARDS) {
        return;
    }
    
    for(uint8_t j=0; j<STATUS_RECORD_CNT; j++){
        cardDB[cardIndex].statusRecords[j] = CARD_STATUS_UNKNOWN;
    }
    cardDB[cardIndex].statusIndex = 0;
}
