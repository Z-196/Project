#ifndef _MAIN_H_
#define _MAIN_H_

#include "../Start/stm32f10x.h"
#include "math.h"
#include "../HardWare/OLED_Data.h"
#include "../Library/stm32f10x_iwdg.h"

/**********变量结构体************/


typedef enum
{
	mode_rfid = 0,
	mode_as608 = 1,
	mode_button = 2,
}Openmode_enum;

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
	page_13 = 12,	
	page_14 = 13,	
	page_15 = 14,	
	page_16 = 15,	
	page_17 = 16,	
	page_18 = 17,	
}Page_enum;

extern uint8_t rx_state;

extern uint8_t s_lock_state;
extern uint8_t s_warning;
extern uint32_t s_password;

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
