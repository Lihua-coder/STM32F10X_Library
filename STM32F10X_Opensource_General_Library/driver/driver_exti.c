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
#include "driver_exti.h"
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      EXTI 中断初始化
//  参数说明      exti_pin         选择 eru 通道
//  参数说明      trigger          触发方式
//  参数说明      PreemptPriority  抢占优先级 0~3
//  参数说明      Subpriority      响应优先级 0~3
//  返回参数      void
//  使用示例      exti_init(ERU_CH0_REQ0_PB0, EXTI_Trigger_Falling, 1, 1);
//  备注信息      仅支持 PB0/PB1/PB5~PB15 对应通道
//-------------------------------------------------------------------------------------------------------------------
void exti_init(exti_pin_enum exti_pin, EXTITrigger_TypeDef trigger, uint8 PreemptPriority, uint8 Subpriority)
{
    GPIO_InitTypeDef  gpio_init_struct;
    EXTI_InitTypeDef  exti_init_struct;
    NVIC_InitTypeDef  nvic_init_struct;
    GPIO_TypeDef*     gpio_port = GPIOB;
    uint16          gpio_pin;
    uint8           pin_source;
    uint32          exti_line;
    uint8           irqn;

    /* 1. 根据 eru 通道解析出引脚号、AFIO 源、EXTI_Line、IRQn */
    switch (exti_pin)
    {
        case ERU_CH0_REQ0_PB0:  gpio_pin = GPIO_Pin_0;  pin_source = GPIO_PinSource0;  exti_line = EXTI_Line0;  irqn = EXTI0_IRQn;       break;
        case ERU_CH1_REQ1_PB1:  gpio_pin = GPIO_Pin_1;  pin_source = GPIO_PinSource1;  exti_line = EXTI_Line1;  irqn = EXTI1_IRQn;       break;
        case ERU_CH5_REQ9_5_PB5: gpio_pin = GPIO_Pin_5; pin_source = GPIO_PinSource5; exti_line = EXTI_Line5; irqn = EXTI9_5_IRQn;     break;
        case ERU_CH6_REQ9_5_PB6: gpio_pin = GPIO_Pin_6; pin_source = GPIO_PinSource6; exti_line = EXTI_Line6; irqn = EXTI9_5_IRQn;     break;
        case ERU_CH7_REQ9_5_PB7: gpio_pin = GPIO_Pin_7; pin_source = GPIO_PinSource7; exti_line = EXTI_Line7; irqn = EXTI9_5_IRQn;     break;
        case ERU_CH8_REQ9_5_PB8: gpio_pin = GPIO_Pin_8; pin_source = GPIO_PinSource8; exti_line = EXTI_Line8; irqn = EXTI9_5_IRQn;     break;
        case ERU_CH9_REQ9_5_PB9: gpio_pin = GPIO_Pin_9; pin_source = GPIO_PinSource9; exti_line = EXTI_Line9; irqn = EXTI9_5_IRQn;     break;
        case ERU_CH10_REQ15_10_PB10: gpio_pin = GPIO_Pin_10; pin_source = GPIO_PinSource10; exti_line = EXTI_Line10; irqn = EXTI15_10_IRQn; break;
        case ERU_CH11_REQ15_10_PB11: gpio_pin = GPIO_Pin_11; pin_source = GPIO_PinSource11; exti_line = EXTI_Line11; irqn = EXTI15_10_IRQn; break;
        case ERU_CH12_REQ15_10_PB12: gpio_pin = GPIO_Pin_12; pin_source = GPIO_PinSource12; exti_line = EXTI_Line12; irqn = EXTI15_10_IRQn; break;
        case ERU_CH13_REQ15_10_PB13: gpio_pin = GPIO_Pin_13; pin_source = GPIO_PinSource13; exti_line = EXTI_Line13; irqn = EXTI15_10_IRQn; break;
        case ERU_CH14_REQ15_10_PB14: gpio_pin = GPIO_Pin_14; pin_source = GPIO_PinSource14; exti_line = EXTI_Line14; irqn = EXTI15_10_IRQn; break;
        case ERU_CH15_REQ15_10_PB15: gpio_pin = GPIO_Pin_15; pin_source = GPIO_PinSource15; exti_line = EXTI_Line15; irqn = EXTI15_10_IRQn; break;
        default: return; /* 非法通道，直接返回 */
    }

    /* 2. 开启 AFIO 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* 3. GPIO 初始化：上拉输入，50 MHz */
    gpio_init_struct.GPIO_Pin  = gpio_pin;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpio_port, &gpio_init_struct);

    /* 4. AFIO 映射 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, pin_source);

    /* 5. EXTI 配置 */
    EXTI_ClearITPendingBit(exti_line);              /* 清空中断标志 */
    exti_init_struct.EXTI_Line    = exti_line;
    exti_init_struct.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti_init_struct.EXTI_Trigger = trigger;
    exti_init_struct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti_init_struct);

    /* 6. NVIC 配置 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    nvic_init_struct.NVIC_IRQChannel                   = irqn;
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = PreemptPriority;
    nvic_init_struct.NVIC_IRQChannelSubPriority        = Subpriority;
    nvic_init_struct.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic_init_struct);
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      EXTI 中断使能（仅把对应 Line 的 EXTI 使能位置 1）
//  参数说明      eru_pin   选择 eru 通道（PB0~PB15 已映射）
//  返回参数      void
//  使用示例      exti_enable(ERU_CH0_REQ0_PB0);
//  备注信息      须保证此前已调用过 exti_init() 完成完整配置
//-------------------------------------------------------------------------------------------------------------------
void exti_enable(exti_pin_enum eru_pin)
{
    uint32 exti_line;

    /* 根据通道号解析出 EXTI_Line */
    switch (eru_pin)
    {
        case ERU_CH0_REQ0_PB0:  exti_line = EXTI_Line0;  break;
        case ERU_CH1_REQ1_PB1:  exti_line = EXTI_Line1;  break;
        case ERU_CH5_REQ9_5_PB5: exti_line = EXTI_Line5; break;
        case ERU_CH6_REQ9_5_PB6: exti_line = EXTI_Line6; break;
        case ERU_CH7_REQ9_5_PB7: exti_line = EXTI_Line7; break;
        case ERU_CH8_REQ9_5_PB8: exti_line = EXTI_Line8; break;
        case ERU_CH9_REQ9_5_PB9: exti_line = EXTI_Line9; break;
        case ERU_CH10_REQ15_10_PB10: exti_line = EXTI_Line10; break;
        case ERU_CH11_REQ15_10_PB11: exti_line = EXTI_Line11; break;
        case ERU_CH12_REQ15_10_PB12: exti_line = EXTI_Line12; break;
        case ERU_CH13_REQ15_10_PB13: exti_line = EXTI_Line13; break;
        case ERU_CH14_REQ15_10_PB14: exti_line = EXTI_Line14; break;
        case ERU_CH15_REQ15_10_PB15: exti_line = EXTI_Line15; break;
        default: return; /* 非法通道，直接返回 */
    }

    /* 仅把对应 Line 的 IMR 位置 1（不改动其它位） */
    EXTI->IMR |= exti_line;
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      EXTI 中断失能（仅把对应 Line 的 EXTI 使能位清 0）
//  参数说明      eru_pin   选择 eru 通道（PB0~PB15 已映射）
//  返回参数      void
//  使用示例      exti_disable(ERU_CH0_REQ0_PB0);
//  备注信息      失能后该 Line 不再产生 NVIC 中断，但 GPIO/AFIO/EXTI 配置仍保留
//-------------------------------------------------------------------------------------------------------------------
void exti_disable(exti_pin_enum eru_pin)
{
    uint32 exti_line;

    /* 根据通道号解析出 EXTI_Line */
    switch (eru_pin)
    {
        case ERU_CH0_REQ0_PB0:  exti_line = EXTI_Line0;  break;
        case ERU_CH1_REQ1_PB1:  exti_line = EXTI_Line1;  break;
        case ERU_CH5_REQ9_5_PB5: exti_line = EXTI_Line5; break;
        case ERU_CH6_REQ9_5_PB6: exti_line = EXTI_Line6; break;
        case ERU_CH7_REQ9_5_PB7: exti_line = EXTI_Line7; break;
        case ERU_CH8_REQ9_5_PB8: exti_line = EXTI_Line8; break;
        case ERU_CH9_REQ9_5_PB9: exti_line = EXTI_Line9; break;
        case ERU_CH10_REQ15_10_PB10: exti_line = EXTI_Line10; break;
        case ERU_CH11_REQ15_10_PB11: exti_line = EXTI_Line11; break;
        case ERU_CH12_REQ15_10_PB12: exti_line = EXTI_Line12; break;
        case ERU_CH13_REQ15_10_PB13: exti_line = EXTI_Line13; break;
        case ERU_CH14_REQ15_10_PB14: exti_line = EXTI_Line14; break;
        case ERU_CH15_REQ15_10_PB15: exti_line = EXTI_Line15; break;
        default: return; /* 非法通道，直接返回 */
    }

    /* 仅把对应 Line 的 IMR 位清 0（不改动其它位） */
    EXTI->IMR &= ~exti_line;
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      关闭全部 EXTI 中断（IMR 全部清 0）
//  参数说明      无
//  返回参数      void
//  使用示例      exti_all_close();
//  备注信息      仅失能中断，不删除配置；后续可单独 exti_enable() 打开
//-------------------------------------------------------------------------------------------------------------------
void exti_all_close(void)
{
    /* 直接清零中断屏蔽寄存器，所有 Line 的 EXTI 中断均被屏蔽 */
    EXTI->IMR = 0x00000000;
}

