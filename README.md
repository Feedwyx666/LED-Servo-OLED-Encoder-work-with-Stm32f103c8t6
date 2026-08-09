# 旋转编码器调节 PWM —— LED 亮度与舵机角度联动

STM32F103C8T6 标准外设库工程。用旋转编码器输出一个 0~100 的计数值，同时驱动两路 PWM：一路调 LED 亮度，一路调舵机角度，当前值实时显示在 OLED 上。

江科大课程的自学练习，在 PWM 与输出比较的基础上把编码器、外部中断、OLED 显示串起来。

## 硬件连线

| 功能 | 引脚 | 配置 |
|------|------|------|
| 板载 LED | PC13 | 推挽输出，**低电平点亮** |
| PWM 通道 1（LED） | PA0 | 复用推挽，TIM2_CH1 |
| PWM 通道 2（舵机） | PA1 | 复用推挽，TIM2_CH2 |
| 编码器 A 相 | PB0 | 上拉输入，EXTI0 下降沿 |
| 编码器 B 相 | PB1 | 上拉输入，EXTI1 下降沿 |
| 编码器自带按键 | PB3 | 上拉输入，EXTI3 下降沿，按下清零计数 |
| 独立按键 | PB14 | 上拉输入，EXTI15_10 下降沿，按下计数加一 |
| OLED SCL | PB12 | 开漏输出，软件 I2C |
| OLED SDA | PB13 | 开漏输出，软件 I2C |

**PB3 需要关闭 JTAG 才能用**。`Encoder.c` 里调了 `GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE)` 释放 PA15/PB3/PB4，保留 SWD 所以 CMSIS-DAP 仍可正常下载。

## 时基与 PWM 参数

TIM2 内部时钟 72 MHz，`PSC = 72-1`，`ARR = 20000-1`：

```
计数时钟 = 72 MHz / 72 = 1 MHz     →  1 个计数 = 1 μs
PWM 周期 = 20000 / 1 MHz = 20 ms   →  频率 50 Hz
```

50 Hz、20 ms 周期是**为舵机选的**。SG90 这类舵机要求 20 ms 周期、0.5~2.5 ms 高电平对应 0~180°，所以：

```c
// Servo.c
TIM_SetCompare2(TIM2, Angel / 180 * 2000 + 500);
// 角度 0   → CCR = 500   → 0.5 ms
// 角度 180 → CCR = 2500  → 2.5 ms
```

CCR 的单位刚好是 μs，这是 `PSC` 选 72 的好处，换算不用再乘系数。

LED 复用了同一个定时器，所以也是 50 Hz：

```c
// main.c
TIM_SetCompare1(TIM2, GetEncoder_Count() * 200);
// 计数 0   → CCR = 0      → 占空比 0%
// 计数 100 → CCR = 20000  → 占空比 100%
```

50 Hz 对 LED 调光偏低（人眼可能感知到闪烁），但两路共用一个定时器的时基，舵机的 20 ms 是硬要求，所以让 LED 跟着走。想单独提高 LED 频率得换到另一个定时器。

## 编码器解码

正交编码器 A、B 两相相位差 90°，旋转方向决定谁先跳变。这里用最省事的做法：两相都触发下降沿中断，在中断里读**另一相**的电平判断方向。

```c
// A 相（PB0）下降沿时，若 B 相为低 → 反向
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) == SET) {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
            Encoder_Count--;
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

// B 相（PB1）下降沿时，若 A 相为低 → 正向
void EXTI1_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) == SET) {
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0)
            Encoder_Count++;
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}
```

计数在 `GetEncoder_Count()` 里限幅到 0~100，正好对应占空比百分数；乘 1.8 得到 0~180 的舵机角度。

## 按键消抖

`KeyEXTI.c` 和 `Encoder.c` 的按键中断用的是同一套写法，关键是**只操作 `EXTI->IMR`，不重新调 `EXTI_Init()`**：

```c
EXTI->IMR &= ~EXTI_Line14;   // 进中断先屏蔽本线，防抖动期重入
Delay_ms(20);                // 跳过机械抖动（典型 5~20 ms）
if (读回引脚仍为低) {         // 确认是真按下
    count++;
    while (引脚仍为低) {}     // 等释放，防一次按压触发多次
    Delay_ms(20);
}
EXTI_ClearITPendingBit(EXTI_Line14);
EXTI->IMR |= EXTI_Line14;    // 恢复
```

用 `EXTI_Init()` 重新使能会连边沿配置一起重写，容易把触发方式改掉；直接改中断屏蔽寄存器只动该动的位。

代价是**在中断里阻塞了几十毫秒**，等待按键释放期间主循环停摆，OLED 不刷新。练习阶段可以接受，正规做法是主循环里定时扫描按键状态机，中断只置标志位。

## 目录结构

```
User/main.c              主循环：读编码器 → 刷 OLED → 更新两路 PWM
User/stm32f10x_it.c      中断向量（实际处理函数写在各模块内）
Hardware/PWM.c           TIM2 时基 + CH1/CH2 输出比较初始化
Hardware/Servo.c         角度到 CCR 的换算
Hardware/Encoder.c       编码器解码 + PB3 按键清零
Hardware/KeyEXTI.c       PB14 独立按键计数
Hardware/LED.c           PC13 板载 LED
Hardware/OLED.c          软件 I2C 驱动 SSD1306
System/Delay.c           SysTick 延时
System/ClockInit.c       GPIO 时钟统一使能
Library/                 ST 标准外设库，只读不改
Start/                   启动文件与 CMSIS
```

## 编译与烧录

Keil MDK 打开 `Project.uvprojx` 直接构建。命令行（Git Bash）：

```bash
# 编译，成功标志 0 Error(s), 0 Warning(s)
"/c/Keil_v5/UV4/UV4.exe" -b "Project.uvprojx" -j0 -o "build_log.txt"

# 烧录，成功标志 Verify OK. Application running
"/c/Keil_v5/UV4/UV4.exe" -f "Project.uvprojx" -o "flash_log.txt"
```

Keil 路径按自己机器改。调试器配的是 **CMSIS-DAP**，烧录算法 `STM32F10x_128.FLM`；用 ST-Link 或 J-Link 需要在 Options for Target → Debug 里换。

`-j0` 会用 V5.06 ARMCC 而非工程配置的 ARM Compiler 6，产物能正常烧录；要严格按工程配置编译就用界面 Rebuild。

## OLED 显示

```
PWM   00000     ← PB14 按键计数
LED   050%      ← 编码器计数，即占空比
Servo 090Deg    ← 计数 × 1.8，舵机角度
```

## 已知可改进的地方

- 主循环无条件全量刷 OLED，软件 I2C 比较慢，是当前的主要耗时。加个「值变了才刷」的判断能明显减负
- 按键消抖在中断里阻塞，应改成主循环状态机扫描
- LED 与舵机共用 TIM2 时基，LED 被迫工作在 50 Hz。想提高得换定时器
- `Encoder_Count` 是 `int16_t` 全局变量，中断写主循环读。本例中 M3 的 16 位访问是单指令，实际安全，但严格来说应加 `volatile`
