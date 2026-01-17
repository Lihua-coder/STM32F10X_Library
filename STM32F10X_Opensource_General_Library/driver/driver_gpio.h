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

#ifndef __DRIVER_GPIO_H
#define __DRIVER_GPIO_H

#include "common_headfile.h"

/* 判断 gpio_mode_enum 是否为输出模式 */
#define IS_GPIO_OUTPUT_MODE(mode)  \
    ((mode) == GPO_OPEN_DTAIN ||   \
     (mode) == GPO_PUSH_PULL  ||   \
     (mode) == GPO_AF_OD      ||   \
     (mode) == GPO_AF_PP)
		 
// 定义初始化 GPIO 的宏
#define INIT_GPIO(gpio_port, pin, speed, mode) do { \
    GPIO_InitStructure.GPIO_Pin = pin; \
    GPIO_InitStructure.GPIO_Speed = speed; \
    GPIO_InitStructure.GPIO_Mode = (GPIOMode_TypeDef)mode; \
    GPIO_Init(gpio_port, &GPIO_InitStructure); \
} while (0)

typedef enum               // 枚举端口方向    此枚举定义不允许用户修改
{
    GPI = 0,               // 定义管脚输入方向
    GPO = 1,               // 定义管脚输出方向
}gpio_dir_enum;

typedef enum               // 枚举端口电平    此枚举定义不允许用户修改
{
    GPIO_LOW =  0,         // 定义低电平
    GPIO_HIGH = 1,         // 定义高电平
}gpio_level_enum;



void gpio_init(gpio_pin_enum pin, gpio_mode_enum pinmode, uint8 dat);
void gpio_set_level (gpio_pin_enum pin, uint8 dat);
uint8 gpio_get_level(gpio_pin_enum pin);
void gpio_toggle_level(gpio_pin_enum pin);



//-------------------------------------------------------------------------------------------------------------------
// 函数简介     对应 IO 复位为低电平
// 参数说明     x           选择的引脚 
// 返回参数     void
// 使用示例     gpio_low(PB5);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
#define gpio_low(x)            	gpio_set_level(x, 0)

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     对应 IO 置位为高电平
// 参数说明     x           选择的引脚 
// 返回参数     void
// 使用示例     gpio_high(PB5);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
#define gpio_high(x)            gpio_set_level(x, 1)


#endif

