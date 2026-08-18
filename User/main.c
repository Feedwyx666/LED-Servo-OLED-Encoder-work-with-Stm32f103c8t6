#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "KeyEXTI.h"
#include "Encoder.h"
#include "PWM.h"
#include "Servo.h"
#include "Motor.h"

/*
实现旋转编码器调整PWM 驱动LED缓慢熄灭
舵机调节
直流电机调节


名称	全称											作用
CK_PSC	Clock Prescaler（定时器输入时钟）				分频器之前的原始时钟信号，也就是定时器时钟源（通常来自内部时钟，比如 72 MHz）
PSC		Prescaler Register（预分频寄存器）				对 CK_PSC 进行分频，产生计数时钟 CK_CNT。实际分频系数是 PSC+1
ARR		Auto-Reload Register（自动重装载寄存器）		计数器计数的上限。计数器从 0 加到 ARR 后溢出归零，并产生更新事件，中断（如 TIM_Update）就是在这里触发的
CCR		Capture/Compare Register（捕获/比较寄存器）		用于 PWM 输出或输入捕获：当计数器 CNT 的值等于 CCR 时，产生比较匹配事件（如输出翻转、触发比较中断等）

PWM频率 Freq=CK_PSC/(PSC+1)(ARR+1)
PWM占空比 Duty=CCR/(ARR+1)
PWM分辨率 Reso=1/(ARR+1)

引脚定义：	PA0--LED
			PA1--Servo舵机

*/


int main(void)
{
	LED_Init();
	OLED_Init();
	KeyEXTI_Init();
	EncoderEXTI_Init();
	PWM_Init();
	Motor_Init();
	LED_Off();
	
	
	
	OLED_ShowString(1,1,"PWM");
	OLED_ShowNum(1,7,0,5);
	while(1)
	{
//		LED_Off();
		int16_t NowEncoder=GetEncoder_Count();
		OLED_ShowNum(1,7,return_count(),5);
		
		OLED_ShowString(2,1,"LED");
		OLED_ShowNum(2,4,NowEncoder,3);
		OLED_ShowChar(2,7,'%');
		
		OLED_ShowString(3,1,"Servo");
		
		Motor_SetSpeed(NowEncoder);
		
		// 先计算角度（强制用 float），再传给舵机和OLED
		float ServoAngle = (float)NowEncoder * 1.8f;

		OLED_ShowNum(3,6,ServoAngle,3);
		OLED_ShowString(3,9,"Deg");



		TIM_SetCompare1(TIM2, NowEncoder*200);
		Servo_SetAngle(ServoAngle);
	}
}
