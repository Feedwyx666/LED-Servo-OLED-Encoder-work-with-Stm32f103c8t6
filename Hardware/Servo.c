#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_SetAngel(float Angel)
{
	
	TIM_SetCompare2(TIM2, Angel / 180 * 2000 + 500);
}
