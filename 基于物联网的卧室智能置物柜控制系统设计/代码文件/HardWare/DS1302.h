#ifndef __DS1302_H
#define __DS1302_H

#include "stm32f10x.h"                  // Device header
#include "Delay.h"

#define SCLK_GPIO_PORT GPIOA
#define SCLK_GPIO_PIN GPIO_Pin_7
#define SCLK_GPIO_CLK RCC_APB2Periph_GPIOA

#define DATA_GPIO_PORT GPIOB
#define DATA_GPIO_PIN GPIO_Pin_0
#define DATA_GPIO_CLK RCC_APB2Periph_GPIOB

#define CE_GPIO_PORT GPIOB
#define CE_GPIO_PIN GPIO_Pin_1
#define CE_GPIO_CLK RCC_APB2Periph_GPIOB

#define CE_L GPIO_ResetBits(CE_GPIO_PORT, CE_GPIO_PIN)		 // 拉低使能位
#define CE_H GPIO_SetBits(CE_GPIO_PORT, CE_GPIO_PIN)		 // 拉高使能位
#define SCLK_L GPIO_ResetBits(SCLK_GPIO_PORT, SCLK_GPIO_PIN) // 拉低时钟线
#define SCLK_H GPIO_SetBits(SCLK_GPIO_PORT, SCLK_GPIO_PIN)	 // 拉高时钟线
#define DATA_L GPIO_ResetBits(DATA_GPIO_PORT, DATA_GPIO_PIN) // 拉低数据线
#define DATA_H GPIO_SetBits(DATA_GPIO_PORT, DATA_GPIO_PIN)	 // 拉高数据线

struct TIMEData
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t week;
};											// 创建TIMEData结构体方便存储时间日期数据
extern struct TIMEData TimeData;			// 全局变量

void ds1302_gpio_init(void);					// ds1302端口初始化
void ds1302_write_onebyte(u8 data);			// 向ds1302发送一字节数据
void ds1302_wirte_rig(u8 address, u8 data); // 向指定寄存器写一字节数据
u8 ds1302_read_rig(u8 address);				// 从指定寄存器读一字节数据
void ds1302_init(void);							// ds1302初始化函数
void ds1302_DATAOUT_init(void);					// IO端口配置为输出
void ds1302_DATAINPUT_init(void);				// IO端口配置为输入
void ds1302_read_time(void);					// 从ds1302读取实时时间（BCD码）
void ds1302_read_realTime(void);				// 将BCD码转化为十进制数据
void time_set(uint8_t* time);
uint8_t DecToBCD(uint8_t dec);					// 将十进制数据转化为BCD码

#endif
