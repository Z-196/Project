#include "ds1302.h"

struct TIMEData TimeData;
uint8_t read_time[7];

void ds1302_gpio_init(void) // CE,SCLK端口初始化
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(CE_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = CE_GPIO_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
	GPIO_Init(CE_GPIO_PORT, &GPIO_InitStructure);	 // 初始化GPIOB.12
	GPIO_ResetBits(CE_GPIO_PORT, CE_GPIO_PIN);

	GPIO_InitStructure.GPIO_Pin = SCLK_GPIO_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
	GPIO_Init(SCLK_GPIO_PORT, &GPIO_InitStructure);	 // 初始化GPIOC.12
	GPIO_ResetBits(SCLK_GPIO_PORT, SCLK_GPIO_PIN);
}

void ds1032_DATAOUT_init(void) // 配置双向I/O端口为输出态
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(DATA_GPIO_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin = DATA_GPIO_PIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(DATA_GPIO_PORT, &GPIO_InitStructure); // 初始化GPIOB.14
	GPIO_ResetBits(DATA_GPIO_PORT, DATA_GPIO_PIN);
}

void ds1032_DATAINPUT_init() // 配置双向I/O端口为输入态
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(DATA_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = DATA_GPIO_PIN; // PB.14 DATA
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DATA_GPIO_PORT, &GPIO_InitStructure); // 初始化GPIOB.14
}

void ds1302_write_onebyte(u8 data) // 向DS1302发送一字节数据
{
	ds1032_DATAOUT_init();
	u8 count = 0;
	SCLK_L;
	for (count = 0; count < 8; count++)
	{
		SCLK_L;
		if (data & 0x01)
		{
			DATA_H;
		}
		else
		{
			DATA_L;
		}		// 先准备好数据再发送
		SCLK_H; // 拉高时钟线，发送数据
		data >>= 1;
	}
}

void ds1302_wirte_rig(u8 address, u8 data) // 向指定寄存器地址发送数据
{
	u8 temp1 = address;
	u8 temp2 = data;
	CE_L;
	SCLK_L;
	Delay_us(1);
	CE_H;
	Delay_us(2);
	ds1302_write_onebyte(temp1);
	ds1302_write_onebyte(temp2);
	CE_L;
	SCLK_L;
	Delay_us(2);
}

void ds1302_wirte(u8 address) // 向指定寄存器地址发送数据
{
	u8 temp1 = address;
	CE_L;
	SCLK_L;
	Delay_us(1);
	CE_H;
	Delay_us(2);
	ds1302_write_onebyte(temp1);
	CE_L;
	SCLK_L;
	Delay_us(2);
}

u8 ds1302_read_rig(u8 address) // 从指定地址读取一字节数据
{
	u8 temp3 = address;
	u8 count = 0;
	u8 return_data = 0x00;
	CE_L;
	SCLK_L;
	Delay_us(3);
	CE_H;
	Delay_us(3);
	ds1302_write_onebyte(temp3);
	ds1032_DATAINPUT_init(); // 配置I/O口为输入
	Delay_us(2);
	for (count = 0; count < 8; count++)
	{
		Delay_us(2); // 使电平持续一段时间
		return_data >>= 1;
		SCLK_H;
		Delay_us(4); // 使高电平持续一段时间
		SCLK_L;
		Delay_us(14); // 延时14us后再去读取电压，更加准确
		if (GPIO_ReadInputDataBit(DATA_GPIO_PORT, DATA_GPIO_PIN))
		{
			return_data = return_data | 0x80;
		}
	}
	Delay_us(2);
	CE_L;
	DATA_L;
	return return_data;
}

void ds1302_init(void)
{
	ds1302_wirte_rig(0x8e, 0x00);  // 解除写保护
	ds1302_wirte_rig(0x80, 0x00);  // 秒 00
	ds1302_wirte_rig(0x82, 0x31);  // 分 31
	ds1302_wirte_rig(0x84, 0x22);  // 时 22
	ds1302_wirte_rig(0x86, 0x29);  // 日 29
	ds1302_wirte_rig(0x88, 0x03);  // 月 03
	ds1302_wirte_rig(0x8a, 0x00);  // 星期 00（可自行改 1~7）
	ds1302_wirte_rig(0x8c, 0x26);  // 年 26
	ds1302_wirte_rig(0x8e, 0x80);  // 开启写保护
}

void time_set(uint8_t* time)
{
	ds1302_wirte_rig(0x8e, 0x00); 		// 关闭写保护
	ds1302_wirte_rig(0x80, time[5]); 	// seconds00秒
	ds1302_wirte_rig(0x82, time[4]); 	// minutes31分
	ds1302_wirte_rig(0x84, time[3]); 	// hours9时
	ds1302_wirte_rig(0x86, time[2]); 	// date18日
	ds1302_wirte_rig(0x88, time[1]); 	// months11月
	ds1302_wirte_rig(0x8c, time[0]); 	// year2023年
	ds1302_wirte_rig(0x8e, 0x80); 		// 关闭写保护
}
 
void ds1302_read_time(void)
{
	read_time[0] = ds1302_read_rig(0x81); // 读秒
	read_time[1] = ds1302_read_rig(0x83); // 读分
	read_time[2] = ds1302_read_rig(0x85); // 读时
	read_time[3] = ds1302_read_rig(0x87); // 读日
	read_time[4] = ds1302_read_rig(0x89); // 读月
	read_time[5] = ds1302_read_rig(0x8B); // 读星期
	read_time[6] = ds1302_read_rig(0x8D); // 读年
}

void ds1302_read_realTime(void)
{
	ds1302_read_time(); // BCD码转换为10进制
	TimeData.second = (read_time[0] >> 4) * 10 + (read_time[0] & 0x0f);
	TimeData.minute = ((read_time[1] >> 4) & (0x07)) * 10 + (read_time[1] & 0x0f);
	TimeData.hour = (read_time[2] >> 4) * 10 + (read_time[2] & 0x0f);
	TimeData.day = (read_time[3] >> 4) * 10 + (read_time[3] & 0x0f);
	TimeData.month = (read_time[4] >> 4) * 10 + (read_time[4] & 0x0f);
	TimeData.week = read_time[5];
	TimeData.year = (read_time[6] >> 4) * 10 + (read_time[6] & 0x0f) + 2000;
}

/**
 * @brief  两位十进制数转BCD码
 * @param  dec: 0~99 的十进制数
 * @retval 8位BCD码 (高4位=十位，低4位=个位)
 */
uint8_t DecToBCD(uint8_t dec)
{
    uint8_t bcd;
	
    // 限制范围 0~99，防止非法值
    if(dec > 99) dec = 99;
    
    // 十位 <<4 | 个位
    bcd = ((dec / 10) << 4) | (dec % 10);
    
    return bcd;
}
