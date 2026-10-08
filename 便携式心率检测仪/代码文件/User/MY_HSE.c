#include "stm32f10x.h"                  // Device header
#include "MY_HSE.h"

void MY_HSE_Init(void)
{
		/*切换外部高速时钟晶振*/
	RCC_DeInit();					//复位RCC配置
	RCC_HSEConfig(RCC_HSE_ON);		//开启HSE
	
	ErrorStatus HSEStatus = RCC_WaitForHSEStartUp();
	if (HSEStatus == SUCCESS) 
	{
		/*HSE启动成功，进行PLLCLK后续配置*/		
		// 使能预取指缓冲区，设置FLASH延迟，因为时钟变快了
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		FLASH_SetLatency(FLASH_Latency_2); // 对于72MHz系统时钟需要2个等待状态
		
		// 配置HCLK, PCLK1, PCLK2的分频
		RCC_HCLKConfig(RCC_SYSCLK_Div1);   //SYSCLK(72) -> AHB  Prescaler(1)  -> HCLK(72)
		RCC_PCLK1Config(RCC_HCLK_Div2);    //HCLK(72)   -> APB1 Prescaler(2)  -> PCKL1(36)
		RCC_PCLK2Config(RCC_HCLK_Div1);    //HCLK(72)   -> APB2 Prescaler(1)  -> PCKL2(72)
		
		// 配置PLL：源为HSE, 9倍频
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		
		// 使能PLL并等待就绪
		RCC_PLLCmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

		// 切换系统时钟源到PLL，并等待切换成功
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while (RCC_GetSYSCLKSource() != 0x08); 	// 0x08表示当前时钟源是PLL
		
		/*HSE稳定运行*/
		
		RCC_ClockSecuritySystemCmd(ENABLE);		//启用CSS
		//时钟安全系统(Clock Security System) -> CSS
		//必须编写NMI中断服务函数		
	} 
	else 
	{	
		/*HSE启动失败，处理错误*/	
		
		RCC_DeInit(); // 复位RCC配置
		RCC_HSICmd(ENABLE);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
	
		LED_Init();	
		while(1)
		{
			LED_ON(LED1);
			Delay_ms(300);
			LED_OFF(LED1);
			Delay_ms(300);
		}	
	}	
}

/*需注释stm32f10x_it.c与stm32f10x_it.h中的对应函数，防止重定义*/
void NMI_Handler(void) 		
{
    if (RCC_GetITStatus(RCC_IT_CSS) != RESET)    // 检查是否是CSS事件触发的NMI
	{
        // CSS检测到HSE错误！进行错误处理
		for (uint16_t IT_CSS_STATE = 0; IT_CSS_STATE < 10; IT_CSS_STATE++)
		{
			LED_ON(LED1);
			Delay_ms(100);
			LED_OFF(LED1);
			Delay_ms(100);
		}
		
        RCC_ClearITPendingBit(RCC_IT_CSS);		// 清除中断标志
    }
}

