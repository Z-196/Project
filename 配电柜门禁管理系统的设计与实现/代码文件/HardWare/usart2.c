#include "delay.h"
#include "usart2.h"
#include "stdarg.h"	 
#include "stdio.h"	 
#include "string.h"	 
#include "timer.h"

u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 			
u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			

vu16 USART2_RX_STA=0;   	

void USART3_IRQHandler(void)
{
	u8 res;	      
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{	 
		res =USART_ReceiveData(USART3);		 
		if((USART2_RX_STA&(1<<15))==0)
		{ 
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	
			{
				TIM_SetCounter(TIM4,0);
				if(USART2_RX_STA==0) 			
				{
					TIM_Cmd(TIM4,ENABLE);
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	 
			}else 
			{
				USART2_RX_STA|=1<<15;			
			} 
		}
	}  				 											 
}   

void usart2_init(u32 bound)
{  
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);

 	USART_DeInit(USART3);  

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
	USART_Init(USART3, &USART_InitStructure);

	USART_Cmd(USART3, ENABLE);                    
  
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM4_Int_Init(99,7199);		
	USART2_RX_STA=0;		
	TIM_Cmd(TIM4,DISABLE);			
}

void u2_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART2_TX_BUF);		
	for(j=0;j<i;j++)						
	{
	  while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);   
		USART_SendData(USART3,USART2_TX_BUF[j]); 
	} 
}
