#include "Int_SI24R1.h"

// 定义一个静态发送地址 => 发送地址与接收地址相同
static uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x0A, 0x01, 0x06, 0x0E, 0x01};
// SPI读写一个字节 => 写入的字节是传入的参数，读取的字节是返回值
static uint8_t SPI_RW(uint8_t byte)
{
    uint8_t rx_data = 0;
    HAL_SPI_TransmitReceive(&hspi2, &byte, &rx_data, 1, 1000);
    return rx_data;
}

/*****************************************************************************
函数功能：写寄存器的值（单字节）
入口参数：reg：寄存器映射地址（格式：SI24R1_WRITE_REG | reg）
         value：要写入寄存器的值
返回值：状态寄存器的值
******************************************************************************/
uint8_t Int_SI24R1_WRITE_REG(uint8_t reg, uint8_t value)
{
    uint8_t status;

    SI24R1_CS_LOW;
    status = SPI_RW(reg);
    SPI_RW(value);
    SI24R1_CS_HIGH;

    return status;
}

/*****************************************************************************
函数功能：写寄存器的值（多字节）
入口参数：reg：寄存器映射地址（格式：SI24R1_WRITE_REG | reg）
         pBuf：写数据首地址
         size：写数据字节数
返回值：状态寄存器的值
******************************************************************************/
uint8_t Int_SI24R1_WRITE_BUF(uint8_t reg, uint8_t *pBuf, uint8_t size)
{
    uint8_t status, byte_ctr;

    SI24R1_CS_LOW;
    status = SPI_RW(reg);
    for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
    {
        SPI_RW(*pBuf++);
    }
    SI24R1_CS_HIGH;

    return status;
}

/*****************************************************************************
函数功能：读寄存器的值（单字节）
入口参数：reg：寄存器映射地址（格式：SI24R1_READ_REG | reg）
返回值：寄存器的值
******************************************************************************/
uint8_t Int_SI24R1_READ_REG(uint8_t reg)
{
    uint8_t value;

    SI24R1_CS_LOW;
    SPI_RW(reg);
    value = SPI_RW(0x00);
    SI24R1_CS_HIGH;

    return value;
}

/*****************************************************************************
函数功能：读取寄存器的值（多字节）
入口参数：reg：寄存器映射地址（格式：SI24R1_READ_REG | reg）
         pBuf：接收缓冲区的首地址
         size：读取字节数
返回值：寄存器的值
******************************************************************************/
uint8_t Int_SI24R1_READ_BUF(uint8_t reg, uint8_t *pBuf, uint8_t size)
{
    uint8_t status, byte_ctr;

    SI24R1_CS_LOW;
    status = SPI_RW(reg);
    for (byte_ctr = 0; byte_ctr < size; byte_ctr++)
    {
        pBuf[byte_ctr] = SPI_RW(0x00); // 读取数据，低字节在前
    }
    SI24R1_CS_HIGH;

    return status;
}

/*****************************************************************************
函数功能：SI24R1接收模式初始化
入口参数：无
返回值：无
******************************************************************************/
void Int_SI24R1_RX_Mode(void)
{
    SI24R1_CE_LOW;
    Int_SI24R1_WRITE_BUF(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 接收设备接收通道0使用和发送设备相同的发送地址
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + EN_AA, 0x01);                          // 使能接收通道0的自动应答
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + EN_RXADDR, 0x01);                      // 使能接收通道0
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + RF_CH, RF_CHANNEL);                    // 选择射频通道40 => 2.4G~2.525G => 125个信道，40即 2.4G + 0.040G => 2.44G
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);             // 设置接收通道0选择和发送通道相同有效数据宽度
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + RF_SETUP, 0x06);                       // 数据传输率1Mbps，发射功率4dBm
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + CONFIG, 0x0F);                         // CRC使能，16位CRC校验，上电，接收模式
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + STATUS, 0xFF);                         // 清除所有的中断标志位
    SI24R1_CE_HIGH;                                                                // 拉高CE，启动接收设备
}

/*****************************************************************************
函数功能：SI24R1发送模式初始化
入口参数：无
返回值：无
******************************************************************************/
void Int_SI24R1_TX_Mode(void)
{
    SI24R1_CE_LOW;
    Int_SI24R1_WRITE_BUF(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);    // 写入发送地址
    Int_SI24R1_WRITE_BUF(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 为了应答接收设备，接收通道0地址和发送地址相同
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + EN_AA, 0x01);                          // 使能接收通道0的自动应答
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + EN_RXADDR, 0x01);                      // 使能接收通道0
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + SETUP_RETR, 0x0a);                     // 自动重发延时等待250us+86us，自动重发10次
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + RF_CH, RF_CHANNEL);                    // 选择射频通道40 => 2.4G~2.525G => 125个信道，40即 2.4G + 0.040G => 2.44G
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + RF_SETUP, 0x06);                       // 数据传输率1Mbps，发射功率4dBm
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + CONFIG, 0x0E);                         // CRC使能，16位CRC校验，上电，发送模式
    SI24R1_CE_HIGH;                                                                // 拉高CE，启动发送设备
}

/*****************************************************************************
函数功能：读取接收数据
硬件直接接收数据保存到FIFO队列中 => 通过状态标志位判断队列中是否有数据
入口参数：rxbuf：接收数据存放首地址
返回值：0：接收到数据
        1：没有接收到数据
******************************************************************************/
uint8_t Int_SI24R1_RxPacket(uint8_t *rxbuf)
{
    uint8_t status;
    // 将读取到的值，原封不动再写回状态寄存器 => 因为状态寄存器中的标志位设计为写1清除
    status = Int_SI24R1_READ_REG(STATUS);                    // 读取状态寄存器的值
    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + STATUS, status); // 清除RX_DS中断标志

    if (status & RX_DR) // 接收到数据
    {
        Int_SI24R1_READ_BUF(RD_RX_PLOAD, rxbuf, TX_PLOAD_WIDTH); // 读取数据
        Int_SI24R1_WRITE_REG(FLUSH_RX, 0xFF);                    // 清除RX_FIFO寄存器
        return 0;
    }

    return 1; // 没收到任何数据
}

/*****************************************************************************
函数功能：发送一个数据包
入口参数：txbuf：要发送的数据
返回值：0：发送成功 1：发送失败
******************************************************************************/
uint8_t Int_SI24R1_TxPacket(uint8_t *txbuf)
{
    uint8_t state;
    SI24R1_CE_LOW;                                            // CE拉低，使能SI24R1配置
    Int_SI24R1_WRITE_BUF(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH); // 写数据到TX_FIFO，32个字节
    SI24R1_CE_HIGH;                                           // CE拉高，使能发送

    // 没有使用中断判断是否发送完成 => 使用轮询读取状态标志位
    do
    {
        state = Int_SI24R1_READ_REG(STATUS); // 读取状态寄存器的值
        vTaskDelay(1);
    } while (((state & TX_DS) == 0) && ((state & MAX_RT) == 0));

    Int_SI24R1_WRITE_REG(SI24R1_WRITE_REG + STATUS, state); // 清除TX_DS或MAX_RT中断标志

    if (state & MAX_RT) // 达到最大重发次数
    {
        Int_SI24R1_WRITE_REG(FLUSH_TX, 0xFF); // 清除TX_FIFO寄存器
        return 1;
    }
    else if (state & TX_DS) // 发送完成
    {
        // 如果SI24R1发送数据完成，会自动清除TX_FIFO寄存器，如果是最大重试次数，则并不会自动清除
        return 0;
    }
    return 1; // 发送失败
}

/**
 * @brief SI24R1的初始化检测
 *
 * @return uint8_t 0: 检测成功 1: 检测失败
 */
uint8_t Int_SI24R1_Check(void)
{
    uint8_t si24r1_rx_buff[5] = {0};
    // 1. 测试SIP通信能够正常读写寄存器
    // 1.0 SI24R1芯片需要先读取一次，保证SPI正常之后再写入
    Int_SI24R1_READ_BUF(SI24R1_READ_REG + TX_ADDR, si24r1_rx_buff, TX_ADR_WIDTH);

    // 1.1 写入发送地址
    Int_SI24R1_WRITE_BUF(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);

    // 1.2 读取同样的数据
    Int_SI24R1_READ_BUF(SI24R1_READ_REG + TX_ADDR, si24r1_rx_buff, TX_ADR_WIDTH);

    for (uint8_t i = 0; i < TX_ADR_WIDTH; i++)
    {
        if (si24r1_rx_buff[i] != TX_ADDRESS[i])
        {
            return 1; // 检查失败
        }
    }

    return 0; // 检查通过
}

/**
 * @brief 硬件接口层SI24R1初始化
 *
 */
void Int_SI24R1_Init(void)
{

    HAL_Delay(200);                 // 上电之后的芯片延时 > 100ms
    while (Int_SI24R1_Check() == 1) // 校验检测
    {
        // 每两次检测间隔10ms
        HAL_Delay(10);
    }

    // 设置默认的状态为接收模式 => 每次发送数据的时候，切换到发送状态
    Int_SI24R1_RX_Mode();
    debug_printf("SI24R1 Init Success!\n");
}
