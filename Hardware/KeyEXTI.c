#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include "Delay.h"
#include "OLED.h"
#include "Encoder.h"


u16 count=0;

/**
  * @brief 外部中断初始化（PB14 按键，低电平触发下降沿）
  * @param None
  * @retval None
  */

void KeyEXTI_Init(void)
{

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef GPIO_Initstructure;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_Initstructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);


	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line14;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);


	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;

	NVIC_Init(&NVIC_InitStruct);

}
u16 return_count(void)
{
		return count;
}

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line14)==SET)
	{
		/* 进入中断立即屏蔽 EXTI14 中断，防止抖动期间反复重入 */
		EXTI->IMR &= ~EXTI_Line14;

		/* 等待20ms，跳过机械抖动期（典型抖动5~20ms） */
		Delay_ms(20);

		/* 抖动期过后再读回引脚：仍为低电平才算有效按下 */
		if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == RESET)
		{
//			ResetEncoderCount();
			
			count++;
			LED_On();
//			OLED_ShowNum(1,7,count,5);	//计数后立即刷新OLED（等待释放期间主循环被阻塞，必须在此刷新）

			/* 等待按键释放（高电平）后再重新使能，防止一次按压触发多次 */
			while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == RESET)
			{
			}

			/* 释放后再延时20ms，跳过释放抖动 */
			Delay_ms(20);
			LED_Off();
		}
		
		/* 清除挂起位，恢复 EXTI14 中断（仅操作中断屏蔽寄存器，不动边沿配置） */
		EXTI_ClearITPendingBit(EXTI_Line14);
		EXTI->IMR |= EXTI_Line14;
	}

}
