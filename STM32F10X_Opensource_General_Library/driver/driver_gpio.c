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
#include "driver_gpio.h"
//--------------------------------------------------------------------------------------------------------------------
// 函数名     gpio_init     
// 功能说明   gpio初始化（管脚 + 模式 + 初始电平）
// 参数说明   pin       选择引脚
// 参数说明   pinmode   引脚模式
// 参数说明   dat       初始电平，仅输出模式有效（0 低 1 高）
// 返回参数   void
// 使用示例   gpio_init(PA3, GPO_PUSH_PULL, 1); // PA3 推挽输出，初始高电平
//--------------------------------------------------------------------------------------------------------------------
void gpio_init(gpio_pin_enum pin, gpio_mode_enum pinmode, uint8 dat)
{
	GPIO_InitTypeDef GPIO_InitStructure; // 定义结构体变量
    /* 1. 根据管脚号算出所属端口 */
    gpio_port_enum port;
    if (pin <= PA15)        port = PA;
    else if (pin <= PB15)   port = PB;
    else                    port = PC;   /* 目前只用到 PC13~PC15 */

    /* 2. 把枚举脚号转成 STM32 标准库位掩码 GPIO_Pin_x */
    uint16 pinRaw;
    if (pin <= PA15)        pinRaw = (uint16)(1 << (pin - PA0));
    else if (pin <= PB15)   pinRaw = (uint16)(1 << (pin - PB0));
    else                    pinRaw = (uint16)(1 << (pin - PC13 + 13));

    /* 3. 得到 GPIOx 寄存器基地址 */
    GPIO_TypeDef *gpiox = (GPIO_TypeDef *)(GPIOA_BASE + (port << 10));

    /* 4. 调用宏完成模式、速度配置 */
    INIT_GPIO(gpiox, pinRaw, GPIO_Speed_50MHz, pinmode);

    /* 5. 如果是输出模式，再给一次初始电平 */
    if (IS_GPIO_OUTPUT_MODE(pinmode))
    {
        if (dat)  GPIO_SetBits(gpiox, pinRaw);
        else      GPIO_ResetBits(gpiox, pinRaw);
    }
}
	//-------------------------------------------------------------------------------------------------------------------
// 函数简介     gpio 输出设置
// 参数说明     pin         选择的引脚 
// 参数说明     dat         0：低电平 1：高电平
// 返回参数     void
// 使用示例      gpio_set_level(PA3, 1); // PA3 输出高
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void gpio_set_level (gpio_pin_enum pin, uint8 dat)
{
	    /* 1. 根据脚号得到端口序号 0~4 */
    uint32 portIndex;
    if (pin <= PA15)        portIndex = 0;   // PA
    else if (pin <= PB15)   portIndex = 1;   // PB
    else                    portIndex = 2;   // PC

    /* 2. 得到 GPIOx 基地址（同 gpio_init 的算法） */
    GPIO_TypeDef *gpiox = (GPIO_TypeDef *)(GPIOA_BASE + (portIndex << 10));

    /* 3. 把枚举脚号转成 PinSource 0~15 */
    uint16 pinSource;
    if (pin <= PA15)        pinSource = pin - PA0;
    else if (pin <= PB15)   pinSource = pin - PB0;
    else                    pinSource = pin - PC13 + 13;

    /* 4. 一条语句搞定：写 BSRR 寄存器
     *    bit0~15  置 1 → 对应引脚输出高
     *    bit16~31 置 1 → 对应引脚输出低（复位）
     */
    if (dat)
        gpiox->BSRR = (uint32)(1 << pinSource);        /* 置高 */
    else
        gpiox->BSRR = (uint32)(1 << (pinSource + 16));/* 置低 */
	
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     gpio 电平获取
// 参数说明     pin         选择的引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 返回参数     uint8       引脚当前电平
// 使用示例     uint8 status = gpio_get_level(PA0);// 获取PA0引脚电平
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 gpio_get_level(gpio_pin_enum pin)
{
    /* 1. 得到端口序号 0~2 */
    uint32 portIndex;
    if (pin <= PA15)        portIndex = 0;   // PA
    else if (pin <= PB15)   portIndex = 1;   // PB
    else                    portIndex = 2;   // PC

    /* 2. 得到 GPIOx 基地址 */
    GPIO_TypeDef *gpiox = (GPIO_TypeDef *)(GPIOA_BASE + (portIndex << 10));

    /* 3. 把枚举脚号转成 0~15 的 bit 位置 */
    uint16 pinSource;
    if (pin <= PA15)        pinSource = pin - PA0;
    else if (pin <= PB15)   pinSource = pin - PB0;
    else                    pinSource = pin - PC13 + 13;

    /* 4. 读 IDR 对应位并返回 0/1 */
    return (gpiox->IDR & (1U << pinSource)) ? 1U : 0U;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     GPIO电平翻转
// 参数说明     pin         引脚号选择的引脚
// 返回参数     void
// 使用示例     gpio_toggle_level(PB1);//翻转PB1电平
//-------------------------------------------------------------------------------------------------------------------
void gpio_toggle_level(gpio_pin_enum pin)
{
    gpio_set_level(pin, !gpio_get_level(pin));
}

