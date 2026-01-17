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
#include "driver_pwm.h"
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PWM初始化
// 参数说明     pwmch				PWM通道号及引脚
// 参数说明     freq            PWM频率
// 参数说明     duty            PWM占空比
// 参数说明     AFIO_flag       重映射标志位 1：开启 0：关闭
// 返回参数     void
// 使用示例     pwm_init(ATOM1_CH2_PA9, 50, 5000, 0);   //初始化TIM1通道2 使用引脚PA9  输出PWM频率50HZ   占空比为百分之 5000/PWM_DUTY_MAX*100
// 备注信息     PWM_DUTY_MAX=10000
//-------------------------------------------------------------------------------------------------------------------
void pwm_init(pwm_channel_enum pwmch, uint16 freq, uint16 duty, uint8 AFIO_flag)
{
    TIM_TypeDef          *timx;
    GPIO_TypeDef         *gpiox;
    uint16              gpio_pin;
    uint32              tim_rcc;
    uint32              remap = 0;          /* 0 表示不重映射 */
    uint8               need_jtag_off = 0;  /* 是否需要关闭 JTAG */
    TIM_OCInitTypeDef     ocinit;
    TIM_TimeBaseInitTypeDef tbinit;

    /* 1. 根据通道枚举，把 TIM、GPIO、时钟、重映射信息一次性配好 */
    switch (pwmch)
    {
        /* TIM1 CH2/3/4 */
        case ATOM1_CH2_PA9:  timx = TIM1;  gpiox = GPIOA; gpio_pin = GPIO_Pin_9;  tim_rcc = RCC_APB2Periph_TIM1; break;
        case ATOM1_CH3_PA10: timx = TIM1;  gpiox = GPIOA; gpio_pin = GPIO_Pin_10; tim_rcc = RCC_APB2Periph_TIM1; break;
        case ATOM1_CH4_PA11: timx = TIM1;  gpiox = GPIOA; gpio_pin = GPIO_Pin_11; tim_rcc = RCC_APB2Periph_TIM1; break;

        /* TIM2 CH1/2/3/4  默认引脚 */
        case ATOM2_CH1_PA0:  timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_0;  tim_rcc = RCC_APB1Periph_TIM2;  break;
        case ATOM2_CH2_PA1:  timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_1;  tim_rcc = RCC_APB1Periph_TIM2;  break;
        case ATOM2_CH3_PA2:  timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_2;  tim_rcc = RCC_APB1Periph_TIM2; break;
        case ATOM2_CH4_PA3:  timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_3;  tim_rcc = RCC_APB1Periph_TIM2; break;

        /* TIM2 重映射引脚 */
        case ATOM2_CH1_PA15: timx = TIM2;  gpiox = GPIOA; gpio_pin = GPIO_Pin_15; tim_rcc = RCC_APB1Periph_TIM2;
                             remap = GPIO_FullRemap_TIM2; need_jtag_off = 1; break;
        case ATOM2_CH2_PB3:  timx = TIM2;  gpiox = GPIOB; gpio_pin = GPIO_Pin_3;  tim_rcc = RCC_APB1Periph_TIM2; 
                             remap = GPIO_FullRemap_TIM2; need_jtag_off = 1; break;
        case ATOM2_CH3_PB10: timx = TIM2;  gpiox = GPIOB; gpio_pin = GPIO_Pin_10; tim_rcc = RCC_APB1Periph_TIM2;
                             remap = GPIO_FullRemap_TIM2; break;
        case ATOM2_CH4_PB11: timx = TIM2;  gpiox = GPIOB; gpio_pin = GPIO_Pin_11; tim_rcc = RCC_APB1Periph_TIM2;
                             remap = GPIO_FullRemap_TIM2; break;

        /* TIM3 默认 */
        case ATOM3_CH1_PA6:  timx = TIM3;  gpiox = GPIOA; gpio_pin = GPIO_Pin_6;  tim_rcc = RCC_APB1Periph_TIM3;break;
        case ATOM3_CH2_PA7:  timx = TIM3;  gpiox = GPIOA; gpio_pin = GPIO_Pin_7;  tim_rcc = RCC_APB1Periph_TIM3; break;
        case ATOM3_CH3_PB0:  timx = TIM3;  gpiox = GPIOB; gpio_pin = GPIO_Pin_0;  tim_rcc = RCC_APB1Periph_TIM3; break;
        case ATOM3_CH4_PB1:  timx = TIM3;  gpiox = GPIOB; gpio_pin = GPIO_Pin_1;  tim_rcc = RCC_APB1Periph_TIM3;  break;

        /* TIM3 部分重映射 */
        case ATOM3_CH1_PB4:  timx = TIM3;  gpiox = GPIOB; gpio_pin = GPIO_Pin_4;  tim_rcc = RCC_APB1Periph_TIM3; 
                             remap = GPIO_PartialRemap_TIM3; need_jtag_off = 1; break;
        case ATOM3_CH2_PB5:  timx = TIM3;  gpiox = GPIOB; gpio_pin = GPIO_Pin_5;  tim_rcc = RCC_APB1Periph_TIM3; 
                             remap = GPIO_PartialRemap_TIM3; break;

        /* TIM4 只有默认引脚 */
        case ATOM4_CH1_PB6:  timx = TIM4;  gpiox = GPIOB; gpio_pin = GPIO_Pin_6;  tim_rcc = RCC_APB1Periph_TIM4; break;
        case ATOM4_CH2_PB7:  timx = TIM4;  gpiox = GPIOB; gpio_pin = GPIO_Pin_7;  tim_rcc = RCC_APB1Periph_TIM4;   break;
        case ATOM4_CH3_PB8:  timx = TIM4;  gpiox = GPIOB; gpio_pin = GPIO_Pin_8;  tim_rcc = RCC_APB1Periph_TIM4;  break;
        case ATOM4_CH4_PB9:  timx = TIM4;  gpiox = GPIOB; gpio_pin = GPIO_Pin_9;  tim_rcc = RCC_APB1Periph_TIM4;  break;

        default: return;   /* 通道号非法，直接返回 */
    }

    /* 2. 开时钟 */
    if (timx == TIM1) RCC_APB2PeriphClockCmd(tim_rcc, ENABLE);
    else              RCC_APB1PeriphClockCmd(tim_rcc, ENABLE);

    /* 3. 重映射 & JTAG 处理 */
    if (AFIO_flag && remap)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(remap, ENABLE);
        if (need_jtag_off)
            GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    }

    /* 4. GPIO 配置为复用推挽输出 50 MHz */
    GPIO_InitTypeDef gpioinit;
    gpioinit.GPIO_Pin   = gpio_pin;
    gpioinit.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpioinit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpiox, &gpioinit);

    /* 5. 时基：频率 = 72 MHz / (PSC+1) / (ARR+1)  */
    uint32_t arr = (SystemCoreClock / freq) - 1;   /* SystemCoreClock 在库中默认 72 MHz */
    TIM_TimeBaseStructInit(&tbinit);
    tbinit.TIM_ClockDivision     = TIM_CKD_DIV1;
    tbinit.TIM_CounterMode       = TIM_CounterMode_Up;
    tbinit.TIM_Period            = arr;
    tbinit.TIM_Prescaler         = 0;              /* 不再分频，直接 72 M 计数 */
    tbinit.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(timx, &tbinit);

    /* 6. 通道 PWM 模式配置 */
    TIM_OCStructInit(&ocinit);
    ocinit.TIM_OCMode      = TIM_OCMode_PWM1;
    ocinit.TIM_OutputState = TIM_OutputState_Enable;
    ocinit.TIM_OCPolarity  = TIM_OCPolarity_High;
    ocinit.TIM_Pulse       = ((uint32_t)duty * (arr + 1)) / PWM_DUTY_MAX;

    /* 7. 根据通道号把 OC 结构体写进对应寄存器 */
    switch (pwmch)
    {
        /* 通道 1 */
        case ATOM2_CH1_PA0:
        case ATOM2_CH1_PA15:
        case ATOM3_CH1_PA6:
        case ATOM3_CH1_PB4:
        case ATOM4_CH1_PB6:
            TIM_OC1Init(timx, &ocinit);  break;

        /* 通道 2 */
        case ATOM1_CH2_PA9:
        case ATOM2_CH2_PA1:
        case ATOM2_CH2_PB3:
        case ATOM3_CH2_PA7:
        case ATOM3_CH2_PB5:
        case ATOM4_CH2_PB7:
            TIM_OC2Init(timx, &ocinit);  break;

        /* 通道 3 */
        case ATOM1_CH3_PA10:
        case ATOM2_CH3_PA2:
        case ATOM2_CH3_PB10:
        case ATOM3_CH3_PB0:
        case ATOM4_CH3_PB8:
            TIM_OC3Init(timx, &ocinit);  break;

        /* 通道 4 */
        case ATOM1_CH4_PA11:
        case ATOM2_CH4_PA3:
        case ATOM2_CH4_PB11:
        case ATOM3_CH4_PB1:
        case ATOM4_CH4_PB9:
            TIM_OC4Init(timx, &ocinit);  break;

        default: break;
    }

    /* 8. 使能定时器 */
    TIM_Cmd(timx, ENABLE);

    /* 9. TIM1 是高级定时器，需要再使能主输出 */
    if (timx == TIM1)
        TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PWM占空比设置
// 参数说明     pin             选择 PWM 引脚
// 参数说明     duty            设置占空比
// 返回参数     void
// 使用示例     pwm_set_duty(ATOM1_CH4_PA11, 5000); // 设置占空比为百分之5000/PWM_DUTY_MAX*100
// 备注信息     GTM_ATOM0_PWM_DUTY_MAX宏定义在driver_pwm.h  默认为10000
//-------------------------------------------------------------------------------------------------------------------
void pwm_set_duty(pwm_channel_enum pwmch, uint32 duty)
{
    // 定义局部变量，用于存储定时器和通道号
    TIM_TypeDef* TIMx;
    uint16 TIM_Channel;

    // 根据通道选择对应的定时器和通道号
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
        case ATOM2_CH1_PA15:
        case ATOM2_CH1_PA0:{
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_1;
				}break;
        case ATOM2_CH2_PB3:
        case ATOM2_CH2_PA1:{
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_2;
				}break;
        case ATOM2_CH3_PB10:
        case ATOM2_CH3_PA2:{
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_3;
				}break;
        case ATOM2_CH4_PB11:
        case ATOM2_CH4_PA3:{
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_4;
				}break;

        // TIM3 通道
        case ATOM3_CH1_PB4:
        case ATOM3_CH1_PA6:{
            TIMx = TIM3;
            TIM_Channel = TIM_Channel_1;
				}break;
        case ATOM3_CH2_PB5:
        case ATOM3_CH2_PA7:{
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
            return; // 如果通道无效，直接返回
    }

    // 获取当前定时器的周期值（ARR）
    uint32 TIM_Period = TIMx->ARR;

    // 计算新的占空比值（CCR）
    uint32 CCR = (duty * TIM_Period) / 10000;

    // 根据通道号设置新的占空比
    switch (TIM_Channel)
    {
        case TIM_Channel_1:
            TIM_SetCompare1(TIMx, CCR);
            break;
        case TIM_Channel_2:
            TIM_SetCompare2(TIMx, CCR);
            break;
        case TIM_Channel_3:
            TIM_SetCompare3(TIMx, CCR);
            break;
        case TIM_Channel_4:
            TIM_SetCompare4(TIMx, CCR);
            break;
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数名称      PWM频率设置
// 参数说明      pwmch            PWM通道
// 参数说明      freq             新的PWM频率
// 返回类型      void
// 使用示例      pwm_set_freq(ATOM1_CH2_PA9, 100); // 将通道 ATOM1_CH2_PA9 的频率设置为 100 Hz
//-------------------------------------------------------------------------------------------------------------------
void pwm_set_freq(pwm_channel_enum pwmch, uint16 freq)
{
    // 定义变量
    TIM_TypeDef* TIMx;
    uint16 TIM_Channel;
    uint32 TIM_Period;

    // 根据通道选择对应的定时器和通道
    switch (pwmch)
    {
        // TIM1 通道
        case ATOM1_CH2_PA9:
            TIMx = TIM1;
            TIM_Channel = TIM_Channel_2;
            break;
        case ATOM1_CH3_PA10:
            TIMx = TIM1;
            TIM_Channel = TIM_Channel_3;
            break;
        case ATOM1_CH4_PA11:
            TIMx = TIM1;
            TIM_Channel = TIM_Channel_4;
            break;

        // TIM2 通道
        case ATOM2_CH1_PA0:
        case ATOM2_CH1_PA15:
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_1;
            break;
        case ATOM2_CH2_PA1:
        case ATOM2_CH2_PB3:
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_2;
            break;
        case ATOM2_CH3_PA2:
        case ATOM2_CH3_PB10:
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_3;
            break;
        case ATOM2_CH4_PA3:
        case ATOM2_CH4_PB11:
            TIMx = TIM2;
            TIM_Channel = TIM_Channel_4;
            break;

        // TIM3 通道
        case ATOM3_CH1_PA6:
        case ATOM3_CH1_PB4:
            TIMx = TIM3;
            TIM_Channel = TIM_Channel_1;
            break;
        case ATOM3_CH2_PA7:
        case ATOM3_CH2_PB5:
            TIMx = TIM3;
            TIM_Channel = TIM_Channel_2;
            break;
        case ATOM3_CH3_PB0:
            TIMx = TIM3;
            TIM_Channel = TIM_Channel_3;
            break;
        case ATOM3_CH4_PB1:
            TIMx = TIM3;
            TIM_Channel = TIM_Channel_4;
            break;

        // TIM4 通道
        case ATOM4_CH1_PB6:
            TIMx = TIM4;
            TIM_Channel = TIM_Channel_1;
            break;
        case ATOM4_CH2_PB7:
            TIMx = TIM4;
            TIM_Channel = TIM_Channel_2;
            break;
        case ATOM4_CH3_PB8:
            TIMx = TIM4;
            TIM_Channel = TIM_Channel_3;
            break;
        case ATOM4_CH4_PB9:
            TIMx = TIM4;
            TIM_Channel = TIM_Channel_4;
            break;

        default:
            return; // 无效通道，直接返回
    }

    // 计算新的定时器周期
    TIM_Period = SystemCoreClock / freq - 1;

    // 更新定时器的周期寄存器
    TIM_SetAutoreload(TIMx, TIM_Period);

    // 获取当前占空比比例
    uint32 current_duty = 0;
    switch (TIM_Channel)
    {
        case TIM_Channel_1:
            current_duty = TIMx->CCR1;
            break;
        case TIM_Channel_2:
            current_duty = TIMx->CCR2;
            break;
        case TIM_Channel_3:
            current_duty = TIMx->CCR3;
            break;
        case TIM_Channel_4:
            current_duty = TIMx->CCR4;
            break;
    }

    // 根据新的周期值重新计算占空比
    uint32 new_duty = (current_duty * TIM_Period) / (TIMx->ARR);

    // 更新占空比
    switch (TIM_Channel)
    {
        case TIM_Channel_1:
            TIM_SetCompare1(TIMx, new_duty);
            break;
        case TIM_Channel_2:
            TIM_SetCompare2(TIMx, new_duty);
            break;
        case TIM_Channel_3:
            TIM_SetCompare3(TIMx, new_duty);
            break;
        case TIM_Channel_4:
            TIM_SetCompare4(TIMx, new_duty);
            break;
    }
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      关闭所有通道的PWM输出
//  返回参数      void
//  使用示例      pwm_all_channel_close();
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void pwm_all_channel_close(void)
{
    // 定义所有可能的定时器
    TIM_TypeDef* TIMx;

    // 遍历所有定时器
    TIMx = TIM1;
    TIM_OC1Init(TIMx, NULL);  // 清空通道1的配置
    TIM_OC2Init(TIMx, NULL);  // 清空通道2的配置
    TIM_OC3Init(TIMx, NULL);  // 清空通道3的配置
    TIM_OC4Init(TIMx, NULL);  // 清空通道4的配置

    TIMx = TIM2;
    TIM_OC1Init(TIMx, NULL);  // 清空通道1的配置
    TIM_OC2Init(TIMx, NULL);  // 清空通道2的配置
    TIM_OC3Init(TIMx, NULL);  // 清空通道3的配置
    TIM_OC4Init(TIMx, NULL);  // 清空通道4的配置

    TIMx = TIM3;
    TIM_OC1Init(TIMx, NULL);  // 清空通道1的配置
    TIM_OC2Init(TIMx, NULL);  // 清空通道2的配置
    TIM_OC3Init(TIMx, NULL);  // 清空通道3的配置
    TIM_OC4Init(TIMx, NULL);  // 清空通道4的配置

    TIMx = TIM4;
    TIM_OC1Init(TIMx, NULL);  // 清空通道1的配置
    TIM_OC2Init(TIMx, NULL);  // 清空通道2的配置
    TIM_OC3Init(TIMx, NULL);  // 清空通道3的配置
    TIM_OC4Init(TIMx, NULL);  // 清空通道4的配置
}


