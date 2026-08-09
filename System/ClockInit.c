#include "stm32f10x.h"

/**
  * @brief  使能GPIOA、GPIOB、GPIOC的时钟
  * @param  无
  * @retval 无
  */
void ClockInit(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
}
