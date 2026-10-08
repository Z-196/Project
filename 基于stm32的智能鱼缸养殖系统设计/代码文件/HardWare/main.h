#ifndef _MAIN_H_
#define _MAIN_H_

#include "../Start/stm32f10x.h"
#include "math.h"
#include "../HardWare/OLED_Data.h"
#include "../Library/stm32f10x_iwdg.h"

/**********变量结构体************/

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
}Page_enum;

typedef enum
{
	work_hand = 0,
	work_auto = 1,
	work_time = 2,
	work_cure = 3,
}Workmode_enum;

extern uint8_t rx_state;
 
extern uint8_t s_temp_min;
extern uint16_t s_lux_min;
extern uint8_t s_water_level_max;
extern uint8_t s_water_level_min;
extern uint16_t s_hzd_max;
extern uint16_t s_tds_max;
extern uint16_t s_led_light;
extern uint8_t s_time_max;
extern uint8_t s_time_min;
extern uint8_t s_feed_tick_set;
extern uint8_t s_o2_tick_set;

extern uint8_t s_state_servo;	
extern uint8_t s_state_relay1;	
extern uint8_t s_state_relay2;	
extern uint8_t s_state_relay3;	
extern uint8_t s_state_relay4;	
extern uint16_t s_state_led;
extern uint8_t s_workmode;

#endif
