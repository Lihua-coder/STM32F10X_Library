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
#include "driver_pit.h"
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     内部调用
// 参数说明     TIMx                选择定时器
// 参数说明     priority     				设置优先级（范围0~15）
// 返回参数     void
// 使用示例     pit_irq_init(TIM2_PIT, 1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
static void pit_irq_init(TIM_TypeDef *TIMx, uint8 priority)
{
    NVIC_InitTypeDef nvic;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);          /* 4 位抢占、0 位响应 */
    nvic.NVIC_IRQChannelPreemptionPriority = priority;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;

    /* 根据定时器选择中断向量 */
    if (TIMx == TIM2) nvic.NVIC_IRQChannel = TIM2_IRQn;
    else if (TIMx == TIM3) nvic.NVIC_IRQChannel = TIM3_IRQn;
    else if (TIMx == TIM4) nvic.NVIC_IRQChannel = TIM4_IRQn;
    else return;                                             /* 防御性编程 */

    NVIC_Init(&nvic);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIM PIT 中断初始化   us 周期
// 参数说明     pit_n                使用的 PIT 编号
// 参数说明     period               PIT 周期 us 级别
// 参数说明     priority     				 设置优先级（范围0~15）
// 返回参数     void
// 使用示例     pit_us_init(TIM2_PIT, 100, 0);// 设置周期中断100us
// 备注信息			范围0us~65.535ms
//-------------------------------------------------------------------------------------------------------------------
void pit_us_init(pit_index_enum pit_n, uint16 period, uint8 priority)
{
    TIM_TypeDef *timx = NULL;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量

    /* 1. 开启时钟并映射句柄 */
    switch (pit_n) {
        case TIM2_PIT: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); timx = TIM2; break;
        case TIM3_PIT: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); timx = TIM3; break;
        case TIM4_PIT: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); timx = TIM4; break;
        default: return;
    }

    /* 2. 时基配置：1 MHz 计数频率 -> 1 us 分辨率 */
    TIM_TimeBaseInitStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period            = (uint16_t)(period - 1);              /* 自动重装载值 */
    TIM_TimeBaseInitStructure.TIM_Prescaler         = (uint16_t)(SystemCoreClock / 1000000) - 1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(timx, &TIM_TimeBaseInitStructure);

    /* 3. 中断配置 */
    TIM_ClearFlag(timx, TIM_FLAG_Update);   /* 清更新标志 */
    TIM_ITConfig(timx, TIM_IT_Update, ENABLE);
    pit_irq_init(timx, priority);

    /* 4. 启动定时器 */
    TIM_Cmd(timx, ENABLE);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIM PIT 中断初始化 us 周期
// 参数说明     pit_n                使用的 PIT 编号
// 参数说明     period               PIT 周期 ms 级别
// 参数说明     priority     				 设置抢占优先级（范围0~15）
// 返回参数     void
// 使用示例     pit_ms_init(TIM2_PIT, 100, 1);
// 备注信息			范围0ms~65.535s
//-------------------------------------------------------------------------------------------------------------------
void pit_ms_init(pit_index_enum pit_n, uint16 period, uint8 priority)
{
    TIM_TypeDef *timx = NULL;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
    /* 1. 开启时钟并映射句柄 */
    switch (pit_n) {
        case TIM2_PIT: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); timx = TIM2; break;
        case TIM3_PIT: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); timx = TIM3; break;
        case TIM4_PIT: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); timx = TIM4; break;
        default: return;
    }

    /* 2. 时基配置：1 kHz 计数频率 -> 1 ms 分辨率 */
    TIM_TimeBaseInitStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period            = (uint16_t)(period - 1);
    TIM_TimeBaseInitStructure.TIM_Prescaler         = (uint16_t)(72000 - 1);   /* 72 MHz / 72 k = 1 kHz */
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(timx, &TIM_TimeBaseInitStructure);

    /* 3. 中断配置 */
    TIM_ClearFlag(timx, TIM_FLAG_Update);
    TIM_ITConfig(timx, TIM_IT_Update, ENABLE);
    pit_irq_init(timx, priority);

    /* 4. 启动定时器 */
    TIM_Cmd(timx, ENABLE);
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      pit启动
//  参数说明      pit_index           选择CCU6模块
//  返回参数      void
//  使用示例      pit_start(TIM2_PIT); // 打开TIM2
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_start(pit_index_enum pit_n)
{
    TIM_TypeDef *timx = NULL;

    /* 根据枚举映射寄存器基地址 */
    switch (pit_n) {
        case TIM2_PIT: timx = TIM2; break;
        case TIM3_PIT: timx = TIM3; break;
        case TIM4_PIT: timx = TIM4; break;
        default: return;                 /* 非法通道直接返回 */
    }

    /* 直接使能计数器 */
    TIM_Cmd(timx, ENABLE);
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      pit关闭
//  参数说明      pit_n        定时器
//  返回参数      void
//  使用示例      pit_close(TIM2_PIT); // 关闭CCU60 通道0的计时器
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_close(pit_index_enum pit_n)
{
    TIM_TypeDef *timx = NULL;
    uint32 rcc_apb1Periph = 0;
    uint8  irqn = 0;

    /* 1. 根据枚举映射寄存器基地址、时钟位、IRQn --------------------------*/
    switch (pit_n) {
        case TIM2_PIT:
            timx  = TIM2;
            rcc_apb1Periph = RCC_APB1Periph_TIM2;
            irqn  = TIM2_IRQn;
            break;

        case TIM3_PIT:
            timx  = TIM3;
            rcc_apb1Periph = RCC_APB1Periph_TIM3;
            irqn  = TIM3_IRQn;
            break;

        case TIM4_PIT:
            timx  = TIM4;
            rcc_apb1Periph = RCC_APB1Periph_TIM4;
            irqn  = TIM4_IRQn;
            break;

        default:
            return;                 /* 非法通道直接返回 */
    }

    /* 2. 关闭计数器 */
    TIM_Cmd(timx, DISABLE);

    /* 3. 关闭更新中断 */
    TIM_ITConfig(timx, TIM_IT_Update, DISABLE);

    /* 4. 关闭 NVIC 中断通道 */
    NVIC_InitTypeDef nvic = {0};
    nvic.NVIC_IRQChannel             = irqn;
    nvic.NVIC_IRQChannelCmd          = DISABLE;
    NVIC_Init(&nvic);

    /* 5. 关闭外设时钟（可选，如后续可能再次使用可注释掉） */
    RCC_APB1PeriphClockCmd(rcc_apb1Periph, DISABLE);
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      禁止所有pit中断
//  返回参数      void
//  使用示例      pit_all_close();
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_all_close(void)
{
    static const pit_index_enum pit_list[] = {
        TIM2_PIT,
        TIM3_PIT,
        TIM4_PIT
    };

    for(uint8 i = 0; i < sizeof(pit_list)/sizeof(pit_list[0]); i++) {
        pit_close(pit_list[i]);
    }
}

