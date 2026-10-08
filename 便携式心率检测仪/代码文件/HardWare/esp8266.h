#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"
#include "OLED.h"

//wifi连接参数
#define ACCOUNT  "zzz"    	//wifi名称: test2	密码: zz123456
#define PASSWORD "12345678"  
#define PRODUCTID   "9f25335827064bf48d31fb57fc8e214a"  
#define SERVERIP    "bemfa.com"
#define SERVERPORT  8344

enum{
WIFI_ACK_OK,WIFI_ACK_ERROR
};

#define RXMAX  1024

typedef struct{
	uint8_t rxbuff[RXMAX];
	uint16_t rxcount;  
	uint8_t rxover;   
	uint8_t txbuff[RXMAX];
}__MESSAGE;

void ESP8266_Config(void);
void Usart2_SendByte(uint8_t data);
void WifiSendStr(char *p);
void WifiSendbuff(uint8_t *p,uint8_t lenth);
void Clear_BuffData(void);
char *FindStr(char *dest,char *src,uint32_t outtime);
uint8_t WifiSendRevAck(char *cmd,char *ack,uint32_t timeout,uint8_t check_cnt);
uint8_t Wifi_OpenTransmission(void);
void Wifi_CloseTransmission(void);
uint8_t ConnectToHotspot(void);
uint8_t Wifi_ConnectServer(char *mode,char *ip,uint16_t port);
uint8_t ConnectToBaffServer(void);

void DataAnylize(void);

// NTP服务器配置（阿里云公共NTP，稳定且国内访问快）
#define NTP_SERVER "ntp.aliyun.com"
#define TIME_ZONE 8 // 东八区，北京时间

// 时间存储数组：Timer[0]=年, Timer[1]=月, Timer[2]=日, Timer[3]=时, Timer[4]=分, Timer[5]=秒
extern uint16_t Timer[6];

// 新增函数声明
uint8_t ESP8266_GetNTPTime(void);


#endif

