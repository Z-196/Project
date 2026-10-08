#ifndef _JQ8900_H
#define _JQ8900_H
#include "stm32f10x.h"
void Init_One_line_Uart(void);

#define sda(EN) EN ?  GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_SET):GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_RESET)
//#define ITConfig(EN) EN ? USART_ITConfig(USART1,USART_IT_RXNE, ENABLE):USART_ITConfig(USART1,USART_IT_RXNE,DISABLE)
void SendData(u8 addr);
void Next(void);
void Previous(void);
void Set_volume(u8 vol);
void Voice_Num(uint16_t num);
void Voive_Play(uint16_t num);
void Voive_Weather(uint16_t num);
void voice_time(uint16_t Hour,uint16_t Min);
void Voice_001(void);	//门已打开
void Voice_002(void);	//验证成功
void Voice_003(void);	//验证失败
void Voice_004(void);	//门已关闭
void Voice_music(void);

#endif
