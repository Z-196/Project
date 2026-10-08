#ifndef _MAIN_H_
#define _MAIN_H_

#include "../Start/stm32f10x.h"
#include "math.h"
#include "../HardWare/OLED_Data.h"
#include "../Library/stm32f10x_iwdg.h"

/**********变量结构体************/


typedef enum
{
	work_auto = 0,
	work_hand = 1
}Work_enum;

typedef enum
{
	none = 0,
	exist = 1
}Person_enum;

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
	page_10 = 9
}Page_enum;

extern uint8_t s_temp_max;
extern uint8_t s_temp_min;
extern uint8_t s_wet_max;
extern uint8_t s_wet_min;
extern uint16_t s_mq2_max;
extern uint16_t s_mq2_min;
extern uint16_t s_mq4_max;
extern uint16_t s_mq4_min;
extern uint16_t s_lux_min;

extern uint8_t s_state_0;
extern uint8_t s_state_1;
extern uint8_t s_state_2;

extern uint8_t s_work_mode;

extern uint8_t rx_state;


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
