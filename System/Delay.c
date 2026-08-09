#include "stm32f10x.h"

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us_jkd(uint32_t xus)
{
	SysTick->LOAD = 72 * xus;				//设置定时器重装值
	SysTick->VAL = 0x00;					//清空当前计数值
	SysTick->CTRL = 0x00000005;				//设置时钟源为HCLK，启动定时器
	while(!(SysTick->CTRL & 0x00010000));	//等待计数到0
	SysTick->CTRL = 0x00000004;				//关闭定时器
}


void Delay_us(u32 xus)
{
    SysTick->CTRL &= ~(1 << 2);          // 选择内部时钟HCLK/8（9MHz）
    SysTick->LOAD = 9 * xus - 1;         // 设定重装载值，计数9*xus个tick
    SysTick->VAL = 0;                    // 清空当前计数值
    SysTick->CTRL &= ~(1 << 1);          // 禁止SysTick中断
    SysTick->CTRL |= (1 << 0);           // 启动SysTick计数器
    while (!(SysTick->CTRL & (1 << 16))); // 等待计数器计数到0，自动清标志
    SysTick->CTRL &= ~(1 << 0);          // 关闭计数器
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 
