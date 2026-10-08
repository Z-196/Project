#include "stm32f10x.h"                  // Device header
#include "delay.h" 
#include "JQ8900.h" 
void Init_One_line_Uart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}
//用于控制jq8900指令
void SendData(u8 addr)
{
	u8 i;
	sda(1);//拉高
	Delay_us( 700);
	sda(0);//拉低
	
	Delay_us (3200);
	 for ( i = 0; i < 8; i++ ) 
    {
        sda(1);//拉高
        if ( addr & 0x01 )//高电平
        {
            Delay_us( 600 );
            sda(0);//拉低
            Delay_us( 210 );
        }
        else //低电平
        {
            Delay_us( 210 );
            sda(0);//拉低
            Delay_us( 600 );
        }
      addr >>= 1;//右移一位
    }
	sda(1);//拉高
}
//播放下一首
void Next(void)
{
	SendData(0x15);	
}
//播放上一首
void Previous(void)
{
	SendData(0x14);	
}
//设置音量
void Set_volume(u8 vol)
{
	SendData(0x0a);//清零数字
	SendData(vol);
	SendData(0x00);
	SendData(0x0c);//设置音量
}
void Voive_Weather(uint16_t num)
{
		SendData(0x0a);
		SendData(0x01); 
		SendData(0x01*num); 
		SendData(0x0b);
}
void Voive_Play(uint16_t num)
{
		SendData(0x0a);
		SendData(0x02); 
		SendData(0x01*num); 
		SendData(0x0b);
}
void Voice_Num(uint16_t num)
{
	if(num == 0)
	{
		SendData(0x0a);
		SendData(0x01); 
		SendData(0x05); 
		SendData(0x0b);
		Delay_ms(800);
	}
	else if(num<10 && num >0)
	{
		SendData(0x0a);
		SendData(0x01*num);
		SendData(0x0b);
		Delay_ms(500);
	}
	else if(num==10)
	{
		SendData(0x0a);
		SendData(0x01); 
		SendData(0x00); 
		SendData(0x0b);
		Delay_ms(500);
	}
	else if(num>10 && num <20)
	{
		int i=0;
		// int j=0;  // 未使用，注释掉
		i = num%10;//计算个位     
		// j = num/10;//计算十位 87/10 -- 8  // 未使用，注释掉
//		SendData(0x0a);
//		SendData(0x01*j); 
//		SendData(0x0b);
//		Delay_ms(500);
		SendData(0x0a);
		SendData(0x01); 
		SendData(0x00); 
		SendData(0x0b);
		Delay_ms(500);
		SendData(0x0a);
		SendData(0x01*i); 
		SendData(0x0b);
		Delay_ms(500);
	}
	else if (num >= 20 && num <100)
	{
		int i=0;
		int j=0;
		i = num%10;//计算个位     
		j = num/10;//计算十位 87/10 -- 8
		SendData(0x0a);
		SendData(0x01*j); 
		SendData(0x0b);
		Delay_ms(500);
		SendData(0x0a);
		SendData(0x01); 
		SendData(0x00); 
		SendData(0x0b);
		Delay_ms(500);
		SendData(0x0a);
		SendData(0x01*i); 
		SendData(0x0b);
		Delay_ms(500);
	}
	else if (num >=100)
	{
		int i=0;
		int j =0;
		int n = 0;
		i = num/100;//计算百位
		j = num/100%10;	//计算十位
		n = num%100;	//计算个位
		SendData(0x0a);
		SendData(0x01*i); 
		SendData(0x0b);
		Delay_ms(500);
		SendData(0x0a);
		SendData(0x01); 
		SendData(0x01);
		SendData(0x0b);
		Delay_ms(500);
		SendData(0x0a);
		SendData(0x01*j); 
		SendData(0x0b);
		Delay_ms(500);
		SendData(0x0a);
		SendData(0x01); 
		SendData(0x00); 
		SendData(0x0b);
		Delay_ms(500);
		SendData(0x0a);
		SendData(0x01*n); 
		SendData(0x0b);
		Delay_ms(500);
	}	
}

void Voice_001(void)
{
	SendData(0x11);
	SendData(0x0a);
	SendData(0x01);       // 播放编号001的语音文件           
	SendData(0x0b);  
	Delay_ms(1500);    // 等待播放完成
}

void Voice_002(void)
{
		//播报地址1
	SendData(0x11);
	SendData(0x0a);
	SendData(0x02);      // 播放编号002的语音文件           
	SendData(0x0b); 
	Delay_ms(1500);    // 等待播放完成
}

void Voice_003(void)
{
		//播报地址1
	SendData(0x11);
	SendData(0x0a);
	SendData(0x03);      // 播放编号002的语音文件            
	SendData(0x0b); 
	Delay_ms(1500);    // 等待播放完成
}

void Voice_004(void)
{
		//播报地址1
	SendData(0x11);
	SendData(0x0a);
	SendData(0x04);      // 播放编号002的语音文件            
	SendData(0x0b); 
	Delay_ms(1500);    // 等待播放完成
}

/**
  * @brief  播放舒缓音乐
  * @note   假设语音文件编号为002（需要根据实际JQ8900中存储的语音文件编号调整）
  */
void Voice_music(void)
{
		//播报地址2
	SendData(0x11);
	SendData(0x0a);
	SendData(0x03);      // 播放编号003的语音文件          
	SendData(0x0b); 
	Delay_ms(1500);    // 等待播放完成
}
