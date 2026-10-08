#include "ds18b20.h"
#include "stm32f10x.h"
 
// 配置DS18B20用到的I/O口
static void DS18B20_GPIO_Config(void)
{		
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(DS18B20_CLK, ENABLE); 
															   
	GPIO_InitStructure.GPIO_Pin = DS18B20_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
	GPIO_SetBits(DS18B20_PORT, DS18B20_PIN);	
}
 
// 配置为上拉输入模式（补充Speed参数）
static void DS18B20_Mode_IPU(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = DS18B20_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 新增：补全配置
  GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
}
 
// 配置为推挽输出模式（无修改，仅对齐格式）
static void DS18B20_Mode_Out_PP(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = DS18B20_PIN;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
}
 
// 主机发送复位脉冲（优化释放总线后的延时）
static void DS18B20_Rst(void)
{
	DS18B20_Mode_Out_PP();     
	DS18B20_DATA_OUT(LOW);     
	Delay_us(750); // 保持：复位低电平≥480us
	DS18B20_DATA_OUT(HIGH);   
	Delay_us(60);  // 修改：从机响应窗口15~60us，延长至60us确保检测到存在脉冲
}
 
// 检测存在脉冲（优化超时阈值）
static u8 DS18B20_Presence(void)
{
	u8 pulse_time = 0;
	DS18B20_Mode_IPU();    
	
	// 等待存在脉冲（低电平），超时阈值从100us→200us
	while( DS18B20_DATA_IN() && pulse_time<200 )
	{
		pulse_time++;
		Delay_us(1);
	}	
	if( pulse_time >=200 )
		return 1;             
	else
		pulse_time = 0;    
	
	// 检测存在脉冲持续时间，超时阈值保持240us
	while( !DS18B20_DATA_IN() && pulse_time<240 )
	{
		pulse_time++;
		Delay_us(1);
	}	
	if( pulse_time >=240 )
		return 1;        
	else
		return 0;
}
 
// 读取一个bit（无修改）
static u8 DS18B20_Read_Bit(void)
{
	u8 dat;
	DS18B20_Mode_Out_PP();
	DS18B20_DATA_OUT(LOW);
	Delay_us(10);
	DS18B20_Mode_IPU();
	
	if( DS18B20_DATA_IN() == SET )
		dat = 1;
	else
		dat = 0;
	
	Delay_us(45);
	return dat;
}
 
// 读取一个字节（无修改）
u8 DS18B20_Read_Byte(void)
{
	u8 i, j, dat = 0;	
	
	for(i=0; i<8; i++) 
	{
		j = DS18B20_Read_Bit();
		dat = (dat) | (j<<i);
	}
	
	return dat;																																																																																
}
 
// 写入一个字节（无修改）
void DS18B20_Write_Byte(u8 dat)
{
	u8 i, testb;
	DS18B20_Mode_Out_PP();
	
	for( i=0; i<8; i++ )
	{
		testb = dat&0x01;
		dat = dat>>1;		
		if (testb)
		{			
			DS18B20_DATA_OUT(LOW);
			Delay_us(8);
			DS18B20_DATA_OUT(HIGH);
			Delay_us(58);
		}		
		else
		{			
			DS18B20_DATA_OUT(LOW);  
			Delay_us(70);
			DS18B20_DATA_OUT(HIGH);			
			Delay_us(2);
		}
	}
}
 
// 启动温度转换（无修改）
void DS18B20_Start(void)
{
	DS18B20_Rst();	          
	DS18B20_Presence();	       
	DS18B20_Write_Byte(0XCC);			
	DS18B20_Write_Byte(0X44);			
}
 
// 初始化函数（无修改）
uint8_t DS18B20_Init(void)
{
	DS18B20_GPIO_Config();   
	DS18B20_Rst();
	return DS18B20_Presence();
}
 
// 读取温度值（核心修改：增加转换延时）
float DS18B20_Get_Temp(void)
{
	u8 tpmsb, tplsb;
	short s_tem;
	float f_tem;
	
	DS18B20_Rst();	   
	if(DS18B20_Presence() != 0) // 新增：检测传感器是否在线
		return 85.0; // 传感器未响应，返回默认值
	
	DS18B20_Write_Byte(0XCC);				
	DS18B20_Write_Byte(0X44);				
	Delay_ms(750); // 核心修改：等待温度转换完成（12位分辨率需750ms）
	
	DS18B20_Rst();
  if(DS18B20_Presence() != 0)
		return 85.0;
	DS18B20_Write_Byte(0XCC);				
	DS18B20_Write_Byte(0XBE);				
	
	tplsb = DS18B20_Read_Byte();		 
	tpmsb = DS18B20_Read_Byte(); 
	
	s_tem = (short)(tpmsb << 8) | tplsb; // 修正：显式转换为short，避免符号位错误
	
	if( s_tem < 0 )		
		f_tem = (float)((~s_tem + 1) * 0.0625);	
	else
		f_tem = (float)(s_tem * 0.0625);
	
	return f_tem; 	
}
