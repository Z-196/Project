#include "esp8266.h"
#include "string.h"
#include "Delay.h"

// 串口选择宏定义：修改数字即可切换（1=USART1，2=USART2，3=USART3）
#define USE_ESP8266_USART 1

// 根据选择的串口，自动配置对应引脚、时钟、中断（无需手动改）
#if USE_ESP8266_USART == 1
#define USARTx                  USART1       // 串口外设
#define RCC_APBx_USART          RCC_APB2Periph_USART1  // 串口时钟
#define RCC_APBx_GPIO           RCC_APB2Periph_GPIOA   // GPIO时钟
#define GPIO_PORT               GPIOA        // 引脚端口
#define TX_PIN                  GPIO_Pin_9   // 发送引脚
#define RX_PIN                  GPIO_Pin_10  // 接收引脚
#define USARTx_IRQHandler       USART1_IRQHandler  // 中断函数名
#define USARTx_IRQn             USART1_IRQn        // 中断通道
#define RCC_USART_ClockCmd      RCC_APB2PeriphClockCmd  // 串口时钟使能函数
#define RCC_GPIO_ClockCmd       RCC_APB2PeriphClockCmd  // GPIO时钟使能函数
#elif USE_ESP8266_USART == 2
#define USARTx                  USART2
#define RCC_APBx_USART          RCC_APB1Periph_USART2
#define RCC_APBx_GPIO           RCC_APB2Periph_GPIOA
#define GPIO_PORT               GPIOA
#define TX_PIN                  GPIO_Pin_2
#define RX_PIN                  GPIO_Pin_3
#define USARTx_IRQHandler       USART2_IRQHandler
#define USARTx_IRQn             USART2_IRQn
#define RCC_USART_ClockCmd      RCC_APB1PeriphClockCmd
#define RCC_GPIO_ClockCmd       RCC_APB2PeriphClockCmd
#elif USE_ESP8266_USART == 3
#define USARTx                  USART3
#define RCC_APBx_USART          RCC_APB1Periph_USART3
#define RCC_APBx_GPIO           RCC_APB2Periph_GPIOB
#define GPIO_PORT               GPIOB
#define TX_PIN                  GPIO_Pin_10
#define RX_PIN                  GPIO_Pin_11
#define USARTx_IRQHandler       USART3_IRQHandler
#define USARTx_IRQn             USART3_IRQn
#define RCC_USART_ClockCmd      RCC_APB1PeriphClockCmd
#define RCC_GPIO_ClockCmd       RCC_APB2PeriphClockCmd
#endif

// 串口收发数据结构体（存储接收缓存、计数、完成标志）
__MESSAGE wifi={0};

// ESP8266串口初始化（宏定义适配，切换串口后自动适配配置）
void ESP8266_Config(void)
{
    // 使能串口和GPIO时钟
    RCC_USART_ClockCmd(RCC_APBx_USART, ENABLE);
    RCC_GPIO_ClockCmd(RCC_APBx_GPIO, ENABLE);
    
    // 配置TX引脚为复用推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIO_PORT, &GPIO_InitStructure);
    
    // 配置RX引脚为上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = RX_PIN;
    GPIO_Init(GPIO_PORT, &GPIO_InitStructure);
    
    // 配置串口参数（波特率115200，8位数据，1位停止，无校验）
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USARTx, &USART_InitStructure);
    
    // 使能接收中断（RXNE），禁用空闲中断（已改为\r\n判断）
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
    
    // 配置中断优先级（分组2，抢占1，响应1）
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    
    // 使能串口
    USART_Cmd(USARTx, ENABLE);    
    Delay_ms(1000); // 等待模块上电稳定
}

// 串口中断处理函数（接收数据+\r\n判断完成）
void USARTx_IRQHandler(void)
{
    uint8_t data = 0;
    // 接收数据中断（每收到1字节触发）
    if(USART_GetITStatus(USARTx, USART_IT_RXNE))
    {
        USART_ClearITPendingBit(USARTx, USART_IT_RXNE); // 清除中断标志
        data = USART_ReceiveData(USARTx); // 读取接收数据
        
        // 缓存未满时存储数据（留1字节防溢出）
        if(wifi.rxcount < RXMAX - 1)
        {
            wifi.rxbuff[wifi.rxcount++] = data;
            
            // 接收字节数≥2时，判断是否为\r\n（接收完成标志）
            if(wifi.rxcount >= 2)
            {
                if(wifi.rxbuff[wifi.rxcount-2] == '\r' && wifi.rxbuff[wifi.rxcount-1] == '\n')
                {
                    wifi.rxover = 1; // 标记接收完成，触发后续解析
                }
            }
        }
        else
        {
            wifi.rxover = 1; // 缓存满，强制标记完成（避免溢出）
        }
    }
}

// 串口发送单个字节
void USARTx_SendByte(uint8_t data)
{
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET); // 等待发送完成
    USART_SendData(USARTx, data); // 发送字节
}

// 发送字符串（逐字节调用发送函数）
void WifiSendStr(char *p)
{
    while(*p != '\0')
    {
        USARTx_SendByte(*p++);
    }
}

// 发送指定长度的字节数组
void WifiSendbuff(uint8_t *p, uint8_t lenth)
{
    for(uint8_t i=0; i<lenth; i++)
    {
        USARTx_SendByte(p[i]);
    }
}

// 清空接收缓存（数据解析后调用）
void Clear_BuffData(void)
{
    memset(wifi.rxbuff,0,RXMAX);
    wifi.rxcount = 0;
    wifi.rxover = 0;
}

/**
 * 功能：查找字符串中是否包含目标子串（带超时）
 * 参数：dest-待查找字符串，src-目标子串，outtime-超时时间(ms)
 * 返回值：找到返回子串地址，未找到返回NULL
 */
char *FindStr(char *dest,char *src,uint32_t outtime)
{
    while((outtime--) && (strstr(dest,src)==NULL))
    {
        Delay_ms(1);
    }
    return strstr(dest,src);
}

/**
 * 功能：发送AT指令并等待应答
 * 参数：cmd-AT指令，ack-期望应答，timeout-超时(ms)，check_cnt-重试次数
 * 返回值：0-成功，>0-失败
 */
uint8_t WifiSendRevAck(char *cmd,char *ack,uint32_t timeout,uint8_t check_cnt)
{
    uint16_t lenth = strlen((char *)cmd);
    wifi.rxcount = 0;
    memset(wifi.rxbuff,0,sizeof(wifi.rxbuff)); // 清空缓存
    do{
            if(*cmd)  // 指令非空则发送
            { 
                WifiSendbuff((u8 *)cmd,lenth);
            }
            if(*ack)  // 需应答则等待查找
            {
                if(FindStr((char *)wifi.rxbuff,(char *)ack,timeout)!=NULL)
                {
                    wifi.rxcount = 0;
                    return 0;
                }
            }
    }while(--check_cnt); // 重试次数减1，直到为0
    return 1;
}

// 打开透传模式（AT+CIPMODE=1）
uint8_t Wifi_OpenTransmission(void)
{
    return WifiSendRevAck("AT+CIPMODE=1\r\n","OK",1000,2);
}

// 关闭透传模式（发送+++，延时2000ms）
void Wifi_CloseTransmission(void)
{
    WifiSendStr("+++");
    Delay_ms(2000);
}

// 连接WiFi热点
uint8_t ConnectToHotspot(void)
{
    char buff[36]={0};
    Wifi_CloseTransmission(); // 先关闭透传
    if(WifiSendRevAck("AT\r\n","OK",100,2)!=0) return 1; // 测试模块应答
    if(WifiSendRevAck("AT+CWMODE=1\r\n","OK",1000,2)!=0) return 2; // 配置Station模式
    sprintf(buff,"AT+CWJAP=\"%s\",\"%s\"\r\n",ACCOUNT,PASSWORD); // 拼接WiFi账号密码
    if(WifiSendRevAck(buff,"OK",20000,2)!=0) return 3; // 连接热点，超时20s
    return 0;
}

// 连接TCP服务器
uint8_t Wifi_ConnectServer(char *mode,char *ip,uint16_t port)
{
    char buff[128]={0};
    sprintf(buff,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",mode,ip,port); // 拼接连接指令
    if(WifiSendRevAck(buff,"CONNECT",10000,2)==WIFI_ACK_OK) // 等待连接成功
    {
    }
    else
    {
        return WIFI_ACK_ERROR;
    }
    if(Wifi_OpenTransmission()==WIFI_ACK_OK) // 打开透传
    {
    }
    else
    {
        return WIFI_ACK_ERROR;
    }
    if(WifiSendRevAck("AT+CIPSEND\r\n",">",500,2)==WIFI_ACK_OK) // 等待发送提示符
    {
    }
    else
    {
        return WIFI_ACK_ERROR;
    }
    return WIFI_ACK_OK;
}

// 连接巴法服务器（调用连接服务器函数，固定TCP模式）
uint8_t ConnectToBaffServer(void)
{
    uint8_t Timers = 2; // 重试2次
    Wifi_CloseTransmission();
    Delay_ms(500);
    while(Timers--)
    {       
        if(Wifi_ConnectServer("TCP",SERVERIP,SERVERPORT) == WIFI_ACK_OK)
        {
            return 1;
        }
    }
    return 0;
}

// ASCII码转数字（仅处理0-9）
uint8_t ASCLL_TO_Oct(char c)
{
    uint8_t num;
    switch(c)
    {
        case 0x30:num = 0;break;
        case 0x31:num = 1;break;
        case 0x32:num = 2;break;
        case 0x33:num = 3;break;
        case 0x34:num = 4;break;
        case 0x35:num = 5;break;
        case 0x36:num = 6;break;
        case 0x37:num = 7;break;
        case 0x38:num = 8;break;
        case 0x39:num = 9;break;
    }
    return num;
}

#include "stdlib.h"
char* topic; // 主题字符串指针
#include "main.h"

void DataAnylize(void)
{
    if(wifi.rxover==1) // 接收完成标志位
    {
        wifi.rxover=0;
        if(strstr((char*)wifi.rxbuff,"topic") != NULL) // 查找是否包含"topic"字段
        {
            if((topic = strstr((char*)wifi.rxbuff,"switch")) != NULL) // 查找"switch"主题
            {
                // 解析switch1指令
                if((topic = strstr((char*)wifi.rxbuff,"s_lock_state"))!= NULL){
                    topic = strstr(topic,":"); // 找到":"分隔符
                    topic++; // 指向数值部分
                    s_lock_state=atof(topic); 	// 转换为数字，翻转LED状态
					rx_state = 1;
                }               
                // 解析switch2指令
                else if((topic = strstr((char*)wifi.rxbuff,"s_warning"))!= NULL){
                    topic = strstr(topic,":");
                    topic++;
                    s_warning=atoi(topic); 
					rx_state = 2;					
                }
                else if((topic = strstr((char*)wifi.rxbuff,"s_password"))!= NULL){
                    topic = strstr(topic,":");
                    topic++;
                    s_password=atof(topic); 
					rx_state = 3;					
                }
            }       
            topic = NULL; 		// 清空指针
            Clear_BuffData(); 	// 清空缓存，准备下一次接收
        }
    }
}
