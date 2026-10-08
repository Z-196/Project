#include "stm32f10x.h"                  // Device header
#include <stdio.h>		//printf函数重定向
#include <stdarg.h>		//封装sprint函数

uint8_t	Serial_RxData;
uint8_t	Serial_RxFlag;
uint8_t Serial_RxPacket[5];

void Serial_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_2;	//发送端口
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_3;	//接收端口
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate= 9600;			//波特率
	USART_InitStructure.USART_HardwareFlowControl= USART_HardwareFlowControl_None;	//硬件流控制
	/*	选择是否使用RTS、CTS，
	RTS	请求发送（主机→从机），表示主机准备好接收数据（低电平有效）。
	CTS	允许发送（从机→主机），表示从机准备好接收数据（低电平有效）。
	*/
	USART_InitStructure.USART_Mode= USART_Mode_Tx | USART_Mode_Rx;	//发送|接收（都使用）
	USART_InitStructure.USART_Parity= USART_Parity_No;	//校验位，NO，无校验；Odd奇校验；Even偶校验
	USART_InitStructure.USART_StopBits= USART_StopBits_1;	//停止位，设置其长度
	USART_InitStructure.USART_WordLength= USART_WordLength_8b;	//字长，不校验8位，校验的话选9位
	USART_Init(USART2,&USART_InitStructure);
		
	//使用中断方式
	USART_ITConfig(USART2,USART_IT_RXNE, ENABLE);	//开启RXNE标志位到NVIC的输出
	//设置NVIC
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//选择分组
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel= USART2_IRQn;	//选择通道为USART1
	NVIC_InitStructure.NVIC_IRQChannelCmd= ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0;	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority= 0;			//从占优先级
	NVIC_Init(&NVIC_InitStructure);
	
	//若使用查询方式，则不需再设置中断参数
	USART_Cmd(USART2,ENABLE);	
}

//接收部分
uint8_t Serial_GetRxFlag(void)
{
	if (Serial_RxFlag == 1)
	{
		Serial_RxFlag = 0;
		return 1;
	}
	return 0;
}

uint8_t Serial_GetRxData(void)
{
	return Serial_RxData;
}

//中断函数
void USART2_IRQHandler(void)
	
{	static uint8_t RxState = 0;		//用来指示到达哪个状态
	static uint8_t pRxPacket = 0;	//用来指示接收到哪个数据

	if (USART_GetITStatus(USART2,USART_IT_RXNE) == SET)
	{
		uint8_t RxData = USART_ReceiveData(USART2);	//该函数读取DR寄存器后，硬件会自动将标志位置0
		
		if (RxState ==0)
		{
			if (RxData == 'E' && Serial_RxFlag == 0)	//说明已经接收到包头且已经接收完之前的数据
			{
				RxState = 1;
				Serial_RxPacket[4] = 'E';
				pRxPacket = 0;
			}
			else if(RxData == 'B' && Serial_RxFlag == 0)
			{
				Serial_RxPacket[4] = 'B';
				Serial_RxFlag = 1;	//接收完成
			}
		}
		else if (RxState ==1)
		{			
			Serial_RxPacket[pRxPacket] = RxData;
			pRxPacket ++;
			if(pRxPacket >= 4)	
			{
				Serial_RxFlag = 1;	//接收完成
				RxState = 0;
			}
		}		
		/*	读取DR寄存器的值后，硬件会自动将标志位清0，该处选择手动清0	*/
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);	//手动清除标志位
	}	
}

//发送单个字节
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART2,Byte);	//写入数据至TDR寄存器
	//等待TDR的数据全部转移到移位寄存器，即等待TXE标志位置1
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
	/*	TXE标志位不需手动清零，当下次调用函数向TDR寄存器写入数据时，硬件会自动将TXE标志位清0	*/	
}

//发送数组
void Serial_SendArray(uint8_t *Array,uint16_t Length)
{
	uint16_t i;
	for ( i=0; i <Length; i++)
	{
		Serial_SendByte(Array[i]);
	}	
}

//发送字符串
void Serial_SendString(char *String)	//字符串自带一个结束标志位，故不再需要传长度参数
{
	uint8_t i;
	for( i=0; String[i] != '\0';i++)	//空字符的转义字符表示形式为 '\0'
	{
		Serial_SendByte(String[i]);
	}
	
}

//次方函数，用于取出每位上的数字
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while(Y--)
	{
		Result *= X;
	}
	return Result;
}

//发送数字，使得最终能在电脑显示字符串形式的数字
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t	i;
	for(i=0; i < Length; i++)
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i -1) %10 + 0x30);
		//Serial_SendByte( Number / Serial_Pow(10, Length - i -1) %10 + '0');//两种写法均可
		//由于最终结果要以字符形式显示，所以需加上0x30的偏移
	}	
}

//由于printf函数默认是输出到屏幕，所以需进行重定向，将printf打印的内容输出到串口
//fputc函数即printf函数的底层
//通过将fputc函数重定向到串口，以使printf输出到串口
int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;	
}

void Serial_Printf(char *format, ...)	// "..."用来接收后面的可变参数列表
{
	char String[100];
	va_list arg;	//定义一个参数列表变量
	va_start(arg, format);	//从format位置开始接收参数表，放在arg里面
	vsprintf(String,format,arg);
	//打印位置是String，格式化字符是format，参数表是arg
	//由于sprintf函数只能接收直接写的参数，所以需改为vsprintf函数
	va_end(arg);	//释放参数表
	
	Serial_SendString(String);
}
