/*********************************************************************************************************************
* 本文件是STM32F10X 开源库的一部分
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
* 欢迎各位使用并传播本程序 但修改内容时必须保留版权声明（即本声明）
*
*
* 修改记录
* 日期              作者           备注
* 2026-01-01        Lihua      first version
********************************************************************************************************************/
#include "driver_pwmi.h"
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PWM输入捕获初始化
// 参数说明     pwmch       PWM通道枚举号
// 参数说明     AFIO_flag   重映射标志：1 开启，0 关闭
// 返回参数     void
// 使用示例     pwmi_init(ATOM1_CH2_PA9, 0);   // 初始化 ATOM1_CH2_PA9 为 PWM 输入捕获
//-------------------------------------------------------------------------------------------------------------------
void pwmi_init(pwm_channel_enum pwmch, uint8 AFIO_flag)
{
		GPIO_InitTypeDef GPIO_InitStructure; // 定义结构体变量
    /* 局部变量：定时器、GPIO、通道、 remap 等信息 */
    TIM_TypeDef          *timx;
    GPIO_TypeDef         *gpiox;
    uint16              gpio_pin;
    uint32              tim_rcc;
    uint32              remap = 0;
    uint8               need_jtag_off = 0;
    TIM_ICInitTypeDef     icinit;
    uint16              tim_channel;

    /* 1. 根据通道枚举，一次性查出所有硬件信息 */
    switch (pwmch)
    {
        /*------------- TIM1 --------------*/
        case ATOM1_CH2_PA9:  timx = TIM1;  gpiox = GPIOA; gpio_pin = GPIO_Pin_9;  tim_rcc = RCC_APB2Periph_TIM1; tim_channel = TIM_Channel_2; break;
        case ATOM1_CH3_PA10: timx = TIM1;  gpiox = GPIOA; gpio_pin = GPIO_Pin_10; tim_rcc = RCC_APB2Periph_TIM1; tim_channel = TIM_Channel_3; break;
        case ATOM1_CH4_PA11: timx = TIM1;  gpiox = GPIOA; gpio_pin = GPIO_Pin_11; tim_rcc = RCC_APB2Periph_TIM1; tim_channel = TIM_Channel_4; break;

        /*------------- TIM2 默认映射 ------*/
        case ATOM2_CH1_PA0:  timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_0;  tim_rcc = RCC_APB1Periph_TIM2; tim_channel = TIM_Channel_1; break;
        case ATOM2_CH2_PA1:  timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_1;  tim_rcc = RCC_APB1Periph_TIM2; tim_channel = TIM_Channel_2; break;
        case ATOM2_CH3_PA2:  timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_2;  tim_rcc = RCC_APB1Periph_TIM2; tim_channel = TIM_Channel_3; break;
        case ATOM2_CH4_PA3:  timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_3;  tim_rcc = RCC_APB1Periph_TIM2; tim_channel = TIM_Channel_4; break;

        /*------------- TIM2 完全重映射 ---*/
        case ATOM2_CH1_PA15: timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_15; tim_rcc = RCC_APB1Periph_TIM2; remap = GPIO_FullRemap_TIM2; need_jtag_off = 1; tim_channel = TIM_Channel_1; break;
        case ATOM2_CH2_PB3:  timx = TIM2;  gpiox = GPIOB; gpio_pin = GPIO_Pin_3;  tim_rcc = RCC_APB1Periph_TIM2; remap = GPIO_FullRemap_TIM2; need_jtag_off = 1; tim_channel = TIM_Channel_2; break;
        case ATOM2_CH3_PB10: timx = TIM2;  gpiox = GPIOB; gpio_pin = GPIO_Pin_10; tim_rcc = RCC_APB1Periph_TIM2; remap = GPIO_FullRemap_TIM2; tim_channel = TIM_Channel_3; break;
        case ATOM2_CH4_PB11: timx = TIM2;  gpiox = GPIOB; gpio_pin = GPIO_Pin_11; tim_rcc = RCC_APB1Periph_TIM2; remap = GPIO_FullRemap_TIM2; tim_channel = TIM_Channel_4; break;

        /*------------- TIM3 默认映射 ------*/
        case ATOM3_CH1_PA6:  timx = TIM3;  gpiox = GPIOA; gpio_pin = GPIO_Pin_6;  tim_rcc = RCC_APB1Periph_TIM3; tim_channel = TIM_Channel_1; break;
        case ATOM3_CH2_PA7:  timx = TIM3;  gpiox = GPIOA; gpio_pin = GPIO_Pin_7;  tim_rcc = RCC_APB1Periph_TIM3; tim_channel = TIM_Channel_2; break;
        case ATOM3_CH3_PB0:  timx = TIM3;  gpiox = GPIOB; gpio_pin = GPIO_Pin_0;  tim_rcc = RCC_APB1Periph_TIM3; tim_channel = TIM_Channel_3; break;
        case ATOM3_CH4_PB1:  timx = TIM3;  gpiox = GPIOB; gpio_pin = GPIO_Pin_1;  tim_rcc = RCC_APB1Periph_TIM3; tim_channel = TIM_Channel_4; break;

        /*------------- TIM3 部分重映射 ---*/
        case ATOM3_CH1_PB4:  timx = TIM3;  gpiox = GPIOB; gpio_pin = GPIO_Pin_4;  tim_rcc = RCC_APB1Periph_TIM3; remap = GPIO_PartialRemap_TIM3; need_jtag_off = 1; tim_channel = TIM_Channel_1; break;
        case ATOM3_CH2_PB5:  timx = TIM3;  gpiox = GPIOB; gpio_pin = GPIO_Pin_5;  tim_rcc = RCC_APB1Periph_TIM3; remap = GPIO_PartialRemap_TIM3; tim_channel = TIM_Channel_2; break;

        /*------------- TIM4 仅默认映射 ----*/
        case ATOM4_CH1_PB6:  timx = TIM4;  gpiox = GPIOB; gpio_pin = GPIO_Pin_6;  tim_rcc = RCC_APB1Periph_TIM4; tim_channel = TIM_Channel_1; break;
        case ATOM4_CH2_PB7:  timx = TIM4;  gpiox = GPIOB; gpio_pin = GPIO_Pin_7;  tim_rcc = RCC_APB1Periph_TIM4; tim_channel = TIM_Channel_2; break;
        case ATOM4_CH3_PB8:  timx = TIM4;  gpiox = GPIOB; gpio_pin = GPIO_Pin_8;  tim_rcc = RCC_APB1Periph_TIM4; tim_channel = TIM_Channel_3; break;
        case ATOM4_CH4_PB9:  timx = TIM4;  gpiox = GPIOB; gpio_pin = GPIO_Pin_9;  tim_rcc = RCC_APB1Periph_TIM4; tim_channel = TIM_Channel_4; break;

        default: return; /* 无效通道直接返回 */
    }

    /* 2. 开时钟 */
    if (timx == TIM1) RCC_APB2PeriphClockCmd(tim_rcc, ENABLE);
    else              RCC_APB1PeriphClockCmd(tim_rcc, ENABLE);

    /* 3. 重映射 & JTAG */
    if (AFIO_flag && remap)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(remap, ENABLE);
        if (need_jtag_off)
            GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    }

    /* 4. GPIO 配置为浮空输入（也可根据需求改成 IPU/IPD） */
		INIT_GPIO(gpiox, gpio_pin, GPIO_Speed_50MHz, GPIO_Mode_IPU);

    /* 5. 时基初始化——固定 1 MHz 计数频率，ARR 最大 0xFFFF，用户以后可再调 */
    TIM_TimeBaseInitTypeDef tbinit;
    TIM_TimeBaseStructInit(&tbinit);
    tbinit.TIM_ClockDivision     = TIM_CKD_DIV1;
    tbinit.TIM_CounterMode       = TIM_CounterMode_Up;
    tbinit.TIM_Period            = 0xFFFF;               /* 最大量程 */
    tbinit.TIM_Prescaler         = (SystemCoreClock / 1000000) - 1; /* 1 MHz */
    tbinit.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(timx, &tbinit);
		
    /* 6. 输入捕获初始化——仅给通道 X 做上升沿捕获，其余默认 */
    TIM_ICStructInit(&icinit);
    icinit.TIM_Channel     = tim_channel;
    icinit.TIM_ICPolarity  = TIM_ICPolarity_Rising;
    icinit.TIM_ICSelection = TIM_ICSelection_DirectTI;
    icinit.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    icinit.TIM_ICFilter    = 0xF;
    TIM_ICInit(timx, &icinit);

    /* 7. 从模式：复位模式，触发源选 TI1FP1（仅 CH1/CH2 有效，若 CH3/CH4 需改用 TI2FP1 等，可再细化） */
    if (tim_channel == TIM_Channel_1 || tim_channel == TIM_Channel_2)
    {
        TIM_SelectInputTrigger(timx, TIM_TS_TI1FP1);
        TIM_SelectSlaveMode(timx, TIM_SlaveMode_Reset);
        TIM_Cmd(timx, ENABLE);
    }
    else
    {
        /* CH3/CH4 无 TI1FP1，简单先开计数器，用户后续自行处理 */
        TIM_Cmd(timx, ENABLE);
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数名称      pwm_get_duty
// 参数说明      pwmch            PWM通道
// 返回类型      uint16           占空比（范围：0~PWM_DUTY_MAX）
// 使用示例      uint16 duty = pwm_get_duty(ATOM1_CH2_PA9); // 获取 ATOM1_CH2_PA9 的占空比
//-------------------------------------------------------------------------------------------------------------------
uint16 pwm_get_duty(pwm_channel_enum pwmch)
{
    // 定义变量
    TIM_TypeDef* TIMx;
    uint16 TIM_Channel;
    uint16 TIM_Period;   // 自动重装载寄存器（ARR）的值
    uint16 TIM_Pulse;    // 捕获/比较寄存器（CCR）的值
    uint16 duty;         // 占空比

    // 根据通道选择对应的定时器和通道
    switch (pwmch)
    {
        // TIM1 通道
        case ATOM1_CH2_PA9:{
            TIMx = TIM1;
            TIM_Channel = TIM_Channel_2;
				}break;
        case ATOM1_CH3_PA10:{
            TIMx = TIM1;
            TIM_Channel = TIM_Channel_3;
				}break;
        case ATOM1_CH4_PA11:{
            TIMx = TIM1;
            TIM_Channel = TIM_Channel_4;
				}break;

        // TIM2 通道
        case ATOM2_CH1_PA0:
        case ATOM2_CH1_PA15:{
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_1;
				}break;
        case ATOM2_CH2_PA1:
        case ATOM2_CH2_PB3:{
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_2;
				}break;
        case ATOM2_CH3_PA2:
        case ATOM2_CH3_PB10:{
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_3;
				}break;
        case ATOM2_CH4_PA3:
        case ATOM2_CH4_PB11:{
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_4;
				}break;

        // TIM3 通道
        case ATOM3_CH1_PA6:
        case ATOM3_CH1_PB4:{
            TIMx = TIM3;
            TIM_Channel = TIM_Channel_1;
				}break;
        case ATOM3_CH2_PA7:
        case ATOM3_CH2_PB5:{
            TIMx = TIM3;
            TIM_Channel = TIM_Channel_2;
				}break;
        case ATOM3_CH3_PB0:{
            TIMx = TIM3;
            TIM_Channel = TIM_Channel_3;
				}break;
        case ATOM3_CH4_PB1:{
            TIMx = TIM3;
            TIM_Channel = TIM_Channel_4;
				}break;

        // TIM4 通道
        case ATOM4_CH1_PB6:{
            TIMx = TIM4;
            TIM_Channel = TIM_Channel_1;
				}break;
        case ATOM4_CH2_PB7:{
            TIMx = TIM4;
            TIM_Channel = TIM_Channel_2;
				}break;
        case ATOM4_CH3_PB8:{
            TIMx = TIM4;
            TIM_Channel = TIM_Channel_3;
				}break;
        case ATOM4_CH4_PB9:{
            TIMx = TIM4;
            TIM_Channel = TIM_Channel_4;
				}break;

        default:
            return 0; // 无效通道，返回 0
    }

    // 获取定时器的自动重装载寄存器（ARR）的值
    TIM_Period = TIMx->ARR;

    // 根据通道获取捕获/比较寄存器（CCR）的值
    switch (TIM_Channel)
    {
        case TIM_Channel_1:
            TIM_Pulse = TIMx->CCR1;
            break;
        case TIM_Channel_2:
            TIM_Pulse = TIMx->CCR2;
            break;
        case TIM_Channel_3:
            TIM_Pulse = TIMx->CCR3;
            break;
        case TIM_Channel_4:
            TIM_Pulse = TIMx->CCR4;
            break;
        default:
            return 0; // 无效通道，返回 0
    }

    // 计算占空比
    if (TIM_Period == 0)
    {
        return 0; // 防止除以零
    }
    duty = (TIM_Pulse * PWM_DUTY_MAX) / TIM_Period;

    return duty;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数名称      pwm_get_freq
// 参数说明      pwmch            PWM通道
// 返回类型      uint16           PWM频率（单位：Hz）
// 使用示例      uint16 freq = pwm_get_freq(ATOM1_CH2_PA9); // 获取 ATOM1_CH2_PA9 的频率
//-------------------------------------------------------------------------------------------------------------------
uint16 pwm_get_freq(pwm_channel_enum pwmch)
{
    TIM_TypeDef *TIMx;
    uint16 TIM_Period;     /* ARR */
    uint16 TIM_Prescaler;  /* PSC */
    uint16 freq;

    /* 根据通道枚举得到 TIMx 指针 */
    switch (pwmch)
    {
        /* TIM1 */
        case ATOM1_CH2_PA9:
        case ATOM1_CH3_PA10:
        case ATOM1_CH4_PA11:
            TIMx = TIM1;
            break;

        /* TIM2 */
        case ATOM2_CH1_PA0:
        case ATOM2_CH1_PA15:
        case ATOM2_CH2_PA1:
        case ATOM2_CH2_PB3:
        case ATOM2_CH3_PA2:
        case ATOM2_CH3_PB10:
        case ATOM2_CH4_PA3:
        case ATOM2_CH4_PB11:
            TIMx = TIM2;
            break;

        /* TIM3 */
        case ATOM3_CH1_PA6:
        case ATOM3_CH1_PB4:
        case ATOM3_CH2_PA7:
        case ATOM3_CH2_PB5:
        case ATOM3_CH3_PB0:
        case ATOM3_CH4_PB1:
            TIMx = TIM3;
            break;

        /* TIM4 */
        case ATOM4_CH1_PB6:
        case ATOM4_CH2_PB7:
        case ATOM4_CH3_PB8:
        case ATOM4_CH4_PB9:
            TIMx = TIM4;
            break;

        default:
            return 0;
    }

    TIM_Period    = TIMx->ARR;
    TIM_Prescaler = TIMx->PSC;

    if (TIM_Period == 0 || TIM_Prescaler == 0)
        return 0;

    freq = (uint16)(SystemCoreClock / ((TIM_Period + 1) * (TIM_Prescaler + 1)));
    return freq;
}
