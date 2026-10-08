#ifndef _MAIN_H_
#define _MAIN_H_

#include "../Start/stm32f10x.h"
#include "math.h"
#include "../HardWare/OLED_Data.h"
#include "../Library/stm32f10x_iwdg.h"

/**********变量结构体************/

typedef enum
{
	workmode_in = 0,
	workmode_out = 1
}Workmode_enum;

typedef enum
{
	success = 0,
	fail = 1,
	error = 2		
}Workresult_enum;


typedef enum
{
	page_1 = 0,
	page_2 = 1,
	page_3 = 2,
	page_4 = 3,
	page_5 = 4,
	page_6 = 5,
	page_7 = 6,
	page_8 = 7,
	page_9 = 8,
	page_10 = 9,
	page_11 = 10,
	page_12 = 11,
	page_13 = 12
}Page_enum;


/*******************************/

/**********阈值结构体************/

//typedef struct 
//{
//	uint16_t Temp;//温度高阈值
//	float Temp_Low;//温度低阈值
//	uint16_t Hum;
//	float Humi_Soil;
//	int Light;//光照高阈值
//	int Light_Low;//光照低阈值
//	u8 adc;
//	u16 co2;
//	int Time;
//} thresh;
/*******************************/

/**********标志位结构体**********/

//typedef struct 
//{
//	u8 i;
//	u8 n;
//	u8 mode;
//	u8 key;
//	int x;
//	int y;
//	int z;
//	u8 LED;
//	u8 Servo;
//	u8 Pump;
//	u8 Fan;
//	u8 Heater;
//	u8 Extra;
//	u8 LED1;
//	u8 Servo1;
//	u8 Pump1;
//	u8 Fan1;
//	u8 Heater1;
//	u8 Extra1;
//} flg;
/*******************************/

//typedef struct 
//{
//	var Variable;       // Variable 类型的成员变量，类型为 var
//	thresh Threshold;   // Threshold 类型的成员变量，类型为 thresh
//	flg flag;           // flag 类型的成员变量，类型为 flg
//} dt;

//extern dt data;

void Task_Initialization(void);

/**********模块数据结构体**********/
// 在这里定义模块可能需要使用的数据结构
// 例如：
// typedef struct {
//     float temperature;
//     float humidity;
// } Module_DHT11_Data;
/**********************************/

// 声明模块数据，如果需要的话
// 例如：extern Module_DHT11_Data dht11_data;

#endif
