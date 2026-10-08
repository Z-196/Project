//#ifndef __DELAY_H
//#define __DELAY_H

////包含头文件
//#include "stm32f10x.h"

////宏定义

////变量声明

////函数声明
//void Delay_Init(void);
//void delay_us(uint32_t nus);
//void delay_ms(uint16_t nms);

//#endif

#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"

#define Delay_1us()	{\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();\
}

extern vu32 timeled[2];
extern vu32 timekey[2];
extern vu32 timedc[2];
extern vu32 timedi[2];

void Delay_Init(void);
void Delay_us(uint64_t time);//函数声明
void Delay_ms(uint64_t time);
void Delay_usnop(u32 time);
void Delay_msnop(u32 time);
#define Delay_mus_nop Delay_msnop

#endif
