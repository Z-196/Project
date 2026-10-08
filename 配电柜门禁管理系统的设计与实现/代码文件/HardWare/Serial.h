#ifndef __SERIAL_H
#define __SERIAL_H
#include "stm32f10x.h"                  // Device header
#include <stdio.h>		//printf函数重定向
#include <stdarg.h>		//封装sprint函数


extern uint8_t Serial_RxPacket[4];
extern uint8_t	Serial_RxData;


void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array,uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxData(void);


#endif
