#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_SetAngle(float Angle)
{
	uint16_t CCR_Value;

	// 限幅：0~180度
	if(Angle > 180.0f)
		Angle = 180.0f;
	if (Angle < 0.0f)
		Angle = 0.0f;

	// 计算 CCR 值：0度→500, 90度→1500, 180度→2500
	// 明确用浮点除法，加 0.5f 实现四舍五入
	CCR_Value = (uint16_t)(Angle / 180.0f * 2000.0f + 500.0f + 0.5f);

	TIM_SetCompare2(TIM2, CCR_Value);
}
