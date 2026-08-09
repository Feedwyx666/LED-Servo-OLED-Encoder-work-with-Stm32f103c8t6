#include "stm32f10x.h"
#include "ClockInit.h"

/**
  * @brief  初始化LED（PC13推挽输出，低电平点亮）
  * @param  无
  * @retval 无
  */
void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);			//使能GPIO时钟

	GPIO_InitTypeDef GPIO_Initstructure;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_Initstructure);
	
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

/**
  * @brief  点亮LED
  * @param  无
  * @retval 无
  */
void LED_On(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}

/**
  * @brief  熄灭LED
  * @param  无
  * @retval 无
  */
void LED_Off(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

/**
  * @brief  翻转LED状态
  * @param  无
  * @retval 无
  */
void LED_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == 0)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	}
	else
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	}
}
