#include "stm32f10x.h"                  // Device header

/*

void TIM_OC1Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct); 初始化

void TIM_SetCompare1(TIM_TypeDef* TIMx, uint16_t Compare1); 更改占空比

void TIM_CtrlPWMOutputs(TIM_TypeDef* TIMx, FunctionalState NewState); 仅高级定时器使用 使能主输出


名称	全称											作用
CK_PSC	Clock Prescaler（定时器输入时钟）				分频器之前的原始时钟信号，也就是定时器时钟源（通常来自内部时钟，比如 72 MHz）
PSC		Prescaler Register（预分频寄存器）				对 CK_PSC 进行分频，产生计数时钟 CK_CNT。实际分频系数是 PSC+1
ARR		Auto-Reload Register（自动重装载寄存器）		计数器计数的上限。计数器从 0 加到 ARR 后溢出归零，并产生更新事件，中断（如 TIM_Update）就是在这里触发的
CCR		Capture/Compare Register（捕获/比较寄存器）		用于 PWM 输出或输入捕获：当计数器 CNT 的值等于 CCR 时，产生比较匹配事件（如输出翻转、触发比较中断等）

PWM频率 Freq=CK_PSC/(PSC+1)(ARR+1)
PWM占空比 Duty=CCR/(ARR+1)
PWM分辨率 Reso=1/(ARR+1)


*/


/**
  * @brief PWM初始化
  * @param None
  * @retval None
  */

void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//使能GPIO时钟
	
	GPIO_InitTypeDef GPIO_Initstructure;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_Initstructure);
	
	
	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 20000 - 1; 		//	ARR
	TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;    //	PSC
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	
	//------------输出比较 PWM ---------------
	
	TIM_OCInitTypeDef TIM_OCInitStruct;
	
	TIM_OCStructInit(&TIM_OCInitStruct);
	
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = 0;  //CCR
	
	/*
	PWM频率 Freq=CK_PSC/(PSC+1)(ARR+1)
	PWM占空比 Duty=CCR/(ARR+1)
	PWM分辨率 Reso=1/(ARR+1)
	1kHz Duty=50% Reso=1%  ==>  ARR=100 PSC=720 CCR=50
	*/
	
	TIM_OC1Init(TIM2, &TIM_OCInitStruct);
	
	TIM_OCInitStruct.TIM_Pulse = 500;  //CCR
	TIM_OC2Init(TIM2, &TIM_OCInitStruct);
	
	
	
	TIM_Cmd(TIM2, ENABLE);
	
}

void PWM1_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//使能GPIO时钟
	
	GPIO_InitTypeDef GPIO_Initstructure;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_Initstructure);
	
	
	TIM_InternalClockConfig(TIM3);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 100 - 1; 		//	ARR
	TIM_TimeBaseInitStruct.TIM_Prescaler = 36 - 1;    //	PSC
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	
	//------------输出比较 PWM ---------------
	
	TIM_OCInitTypeDef TIM_OCInitStruct;
	
	TIM_OCStructInit(&TIM_OCInitStruct);
	
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = 0;  //CCR
	
	/*
	PWM频率 Freq=CK_PSC/(PSC+1)(ARR+1)
	PWM占空比 Duty=CCR/(ARR+1)
	PWM分辨率 Reso=1/(ARR+1)
	1kHz Duty=50% Reso=1%  ==>  ARR=100 PSC=720 CCR=50
	*/
	
	TIM_OC1Init(TIM3, &TIM_OCInitStruct);
	
	TIM_OCInitStruct.TIM_Pulse = 500;  //CCR
	
	
	
	TIM_Cmd(TIM3, ENABLE);
	
}
