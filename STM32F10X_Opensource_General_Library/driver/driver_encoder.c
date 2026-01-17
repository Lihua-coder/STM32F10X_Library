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
#include "driver_encoder.h"
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      方向编码器采集初始化
//  参数说明      encoder_n       选择所使用的定时器
//  参数说明      ch1_pin         设置计数引脚
//  参数说明      ch2_pin         设置方向引脚
//  参数说明      AFIO_flag       重映射标志位 1：开启 0：关闭
//  返回参数      void
//  使用示例      encoder_quad_init(TIM2_ENCODER, TIM2_ENCODER_CH1_PA0, TIM2_ENCODER_CH2_PA1, 0);// 使用T2定时器   PA0引脚进行计数    计数方向使用PA6引脚
//-------------------------------------------------------------------------------------------------------------------
void encoder_dir_init (encoder_index_enum encoder_n, encoder_channel1_enum ch1_pin, encoder_channel2_enum ch2_pin, uint8 AFIO_flag)
{
    TIM_TypeDef* TIMx = NULL;

    /*-------- 1. 根据选择打开定时器时钟 --------*/
    switch(encoder_n)
    {
        case TIM2_ENCODER:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
            TIMx = TIM2;
            break;
        case TIM3_ENCODER:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
            TIMx = TIM3;
            break;
        case TIM4_ENCODER:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
            TIMx = TIM4;
            break;
        default: return;
    }

    /*-------- 2. 打开 AFIO 时钟（需要重映射时用） --------*/
		if(AFIO_flag)RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /*-------- 3. 根据引脚枚举初始化 GPIO --------*/
//    gpio_port_enum port_a, port_b;
//    uint16_t pin_a, pin_b;
gpio_pin_enum pin_a, pin_b;
    /* 通道 A */
    switch(ch1_pin)
    {
        case TIM2_ENCODER_CH1_PA0:  pin_a = PA0; break;
        case TIM2_ENCODER_CH1_PA15: pin_a = PA15;
                                     GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
                                     GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
                                     break;
        case TIM3_ENCODER_CH1_PA6:  pin_a = PA6; break;
        case TIM3_ENCODER_CH1_PB4:  pin_a = PB4;
                                     GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
                                     GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
                                     break;
        case TIM4_ENCODER_CH1_PB6:  pin_a = PB6; break;
        default: return;
    }

    /* 通道 B */
    switch(ch2_pin)
    {
        case TIM2_ENCODER_CH2_PA1:  pin_b = PA1; break;
        case TIM2_ENCODER_CH2_PB3:  pin_b = PB3;
                                     GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
                                     GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
                                     break;
        case TIM3_ENCODER_CH2_PA7:  pin_b = PA7; break;
        case TIM3_ENCODER_CH2_PB5:  pin_b = PB5;
                                     GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
                                     break;
        case TIM4_ENCODER_CH2_PB7:  pin_b = PB7; break;
        default: return;
    }

    /* 统一配置为上拉输入 */
    gpio_init(pin_a, GPI_PULL_UP, 1);
    gpio_init(pin_b, GPI_PULL_UP, 1);

    /*-------- 4. 定时器正交编码配置 --------*/
    TIM_DeInit(TIMx);

    TIM_TimeBaseInitTypeDef tim_base = {0};
    tim_base.TIM_Prescaler = 0;                    // 不分频
    tim_base.TIM_CounterMode = TIM_CounterMode_Up;
    tim_base.TIM_Period = 0xFFFF;                  // 最大计数范围
    tim_base.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIMx, &tim_base);

	  /*输入捕获初始化*/
		TIM_ICInitTypeDef TIM_ICInitStructure;							//定义结构体变量
		TIM_ICStructInit(&TIM_ICInitStructure);							//结构体初始化，若结构体没有完整赋值
																		//则最好执行此函数，给结构体所有成员都赋一个默认值
																		//避免结构体初值不确定的问题
		TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;				//选择配置定时器通道1
		TIM_ICInitStructure.TIM_ICFilter = 0xF;							//输入滤波器参数，可以过滤信号抖动
		TIM_ICInit(TIMx, &TIM_ICInitStructure);							//将结构体变量交给TIM_ICInit，配置TIM3的输入捕获通道
		TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;				//选择配置定时器通道2
		TIM_ICInitStructure.TIM_ICFilter = 0xF;							//输入滤波器参数，可以过滤信号抖动
		TIM_ICInit(TIMx, &TIM_ICInitStructure);							//将结构体变量交给TIM_ICInit，配置TIM3的输入捕获通道

    /* 编码器模式 3：TI1、TI2 均计数，双边沿 */
    TIM_EncoderInterfaceConfig(TIMx,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);

    TIM_SetCounter(TIMx, 0);   // 清零
    TIM_Cmd(TIMx, ENABLE);     // 启动
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介  获取编码器当前计数值（带符号）
// 参数      encoder_n  选择 TIM2_ENCODER/TIM3_ENCODER/TIM4_ENCODER
// 返回      int16      当前计数器值（正向为正，反向为负）
// 示例      int16 speed = encoder_get_count(TIM2_ENCODER);
//-------------------------------------------------------------------------------------------------------------------
int16 encoder_get_count(encoder_index_enum encoder_n)
{
    TIM_TypeDef* TIMx = NULL;
		int16 speed;
    switch(encoder_n)
    {
        case TIM2_ENCODER: TIMx = TIM2; break;
        case TIM3_ENCODER: TIMx = TIM3; break;
        case TIM4_ENCODER: TIMx = TIM4; break;
        default: return 0;
    }

	speed = TIM_GetCounter(TIMx);
	TIM_SetCounter(TIMx, 0);
	return speed;
		
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介  清零编码器计数器
// 参数      encoder_n  选择 TIM2_ENCODER/TIM3_ENCODER/TIM4_ENCODER
// 返回      void
// 示例      encoder_clear_count(TIM2_ENCODER);
//-------------------------------------------------------------------------------------------------------------------
void encoder_clear_count(encoder_index_enum encoder_n)
{
    TIM_TypeDef* TIMx = NULL;

    switch(encoder_n)
    {
        case TIM2_ENCODER: TIMx = TIM2; break;
        case TIM3_ENCODER: TIMx = TIM3; break;
        case TIM4_ENCODER: TIMx = TIM4; break;
        default: return;
    }

    TIM_SetCounter(TIMx, 0);
}


