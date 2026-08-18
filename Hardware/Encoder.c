#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include "Delay.h"

/**
  * @brief 外部中断初始化（PB0 PB1 Encoder，低电平触发下降沿）
  * @param None
  * @retval None
  */

void EncoderEXTI_Init(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef GPIO_Initstructure;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_Initstructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);


	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line0 | EXTI_Line1;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);


	// NVIC_PriorityGroupConfig 已在 KeyEXTI_Init 中调用，此处删除避免重复配置

	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;

	NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;

	NVIC_Init(&NVIC_InitStruct);
	
	
	//-----------------------Encoder Key -------------------------------------
	
	//	GPIO_InitTypeDef GPIO_Initstructure;
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);   //关闭JTAG 释放PA15 PB3 PB4 PA13 PA14
	
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_Initstructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);


//	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line3;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);


	// NVIC_PriorityGroupConfig 已在 KeyEXTI_Init 中调用，此处删除避免重复配置

//	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;

	NVIC_Init(&NVIC_InitStruct);
	
	
	
	
}

int16_t Encoder_Count = 0;

int16_t GetEncoder_Count(void)
{
	return Encoder_Count;
}


void CheckEncoderCount()
{
	if (Encoder_Count>100)
		Encoder_Count = 100;
	if (Encoder_Count<0)
		Encoder_Count = 0;
}

	

void ResetEncoderCount(void)
{
	Encoder_Count=0;
}

void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0)==SET)
	{
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0)
			Encoder_Count--;
		CheckEncoderCount();
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}



void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line1)==SET)
	{
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)==0)
			Encoder_Count++;
		CheckEncoderCount();
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}


void EXTI3_IRQHandler(void)
{
//	if(EXTI_GetITStatus(EXTI_Line3)==SET)
//	{
//		LED_On();
//		Delay_us(200);
//		LED_Off();
//		ResetEncoderCount();

//		EXTI_ClearITPendingBit(EXTI_Line3);
//	}
	
	if(EXTI_GetITStatus(EXTI_Line3)==SET)
	{
		/* 进入中断立即屏蔽 EXTI14 中断，防止抖动期间反复重入 */
		EXTI->IMR &= ~EXTI_Line3;

		/* 等待20ms，跳过机械抖动期（典型抖动5~20ms） */
		Delay_ms(20);

		/* 抖动期过后再读回引脚：仍为低电平才算有效按下 */
		if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == RESET)
		{
			ResetEncoderCount();
			
//			count++;
			LED_On();
//			OLED_ShowNum(1,7,count,5);	//计数后立即刷新OLED（等待释放期间主循环被阻塞，必须在此刷新）

			/* 等待按键释放（高电平）后再重新使能，防止一次按压触发多次 */
			while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3) == RESET)
			{
			}

			/* 释放后再延时20ms，跳过释放抖动 */
			Delay_ms(20);
			LED_Off();
		}
		
		/* 清除挂起位，恢复 EXTI14 中断（仅操作中断屏蔽寄存器，不动边沿配置） */
		EXTI_ClearITPendingBit(EXTI_Line3);
		EXTI->IMR |= EXTI_Line3;
	}
}

