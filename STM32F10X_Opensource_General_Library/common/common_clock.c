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
#include "common_clock.h"
//----------------------------------------------------------------------------
// 函数简介          时钟初始化
// 参数说明          port        引脚的端口
// 返回类型          void
// 使用示例          clock_gpio_init(PA);//开启GPIOA的时钟
// 备注信息          I/O
//----------------------------------------------------------------------------
void clock_gpio_init(gpio_port_enum port){     // 核心时钟初始化
	   switch(port){
        case PA: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);break;	//开启 GPIOA 的时钟
        case PB: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);break;	//开启 GPIOB 的时钟
        case PC: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);break;	//开启 GPIOC 的时钟
        default: break; // 如果端口不匹配，直接返回
    }
}







