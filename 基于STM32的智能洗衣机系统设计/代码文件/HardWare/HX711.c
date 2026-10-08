#include "HX711.h"

uint32_t HX711_Buffer;      // 存储AD原始值（24位）
uint32_t Weight_Maopi;      // 毛重AD值
uint32_t Weight_Shiwu;      // 实物重量，单位：g
uint8_t Flag_Error = 0;     // 错误标志


// 微秒级延时
void Delay__hx711_us(void)
{
    Delay_us(2);
}

// 引脚初始化
void HX711_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(GPIO_CLK_HX711, ENABLE);
	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = HX711_DOUT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// DOUT - 浮空输入
    GPIO_Init(GPIO_HX711, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = HX711_SCK;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		// SCK  - 推挽输出
    GPIO_Init(GPIO_HX711, &GPIO_InitStructure);   
	
	Get_Maopi();
}

//读取HX711
uint32_t HX711_Read(void)	//增益128
{
	uint32_t count = 0;
    
	uint32_t timeout = 0;  // 超时计数器
    Delay__hx711_us();
    GPIO_ResetBits(GPIO_HX711, HX711_SCK);
	
	while(GPIO_ReadInputDataBit(GPIO_HX711, HX711_DOUT))	// 等待DOUT拉低（数据就绪）
	{
		timeout++;
        Delay__hx711_us();  // 每次循环延时2μs，累计超时判断
        // 超时判断：timeout * 2μs ≥ HX711_TIMEOUT_US 时退出
        if(timeout > (HX711_TIMEOUT_US / 2))
        {
            return 0;  // 超时返回0，上层可判断读取失败
        }
	}
	
  	for(uint8_t i = 0; i < 24; i++)				
	{
		GPIO_SetBits(GPIO_HX711, HX711_SCK); 
	  	count = count<<1; 
		GPIO_ResetBits(GPIO_HX711, HX711_SCK);
	  	
		if(GPIO_ReadInputDataBit(GPIO_HX711, HX711_DOUT))	count++; 
	} 
	
 	GPIO_SetBits(GPIO_HX711, HX711_SCK);
    count = count^0x800000;					//第25个脉冲下降沿时，转换数据
	Delay__hx711_us();
	GPIO_ResetBits(GPIO_HX711, HX711_SCK);
	
	return count;
}

//获取毛重
void Get_Maopi(void)
{
	Weight_Maopi = HX711_Read();	
} 

//称重
void Get_Weight(void)
{
	HX711_Buffer = HX711_Read();
	if(HX711_Buffer > Weight_Maopi)			
	{
		Weight_Shiwu = HX711_Buffer;
		Weight_Shiwu = Weight_Shiwu - Weight_Maopi;					//获取实物的AD采样数值。
	
		Weight_Shiwu = (uint32_t)((float)Weight_Shiwu/GapValue) - 21030; 	//计算实物的实际重量																
	}
	else Weight_Shiwu = 0; 		// 无重物时重量为0    
}
