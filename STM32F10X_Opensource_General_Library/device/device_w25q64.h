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
/*********************************************************************************************************************
* 接线定义：
*                   ------------------------------------
*                   模块管脚            单片机管脚
*                   // 硬件 SPI 引脚
*                   SCL/SPC           查看 device_w25q64.h 中 W25Q64_SPC_PIN 宏定义
*                   SDA/DSI           查看 device_w25q64.h 中 W25Q64_SDI_PIN 宏定义
*                   SA0/SDO           查看 device_w25q64.h 中 W25Q64_SDO_PIN 宏定义
*                   CS                查看 device_w25q64.h 中 W25Q64_CS_PIN 宏定义
*                   VCC               3.3V电源
*                   GND               电源地
*                   其余引脚悬空
*
*                   // 软件 SPI 引脚
*                   同上
*                   ------------------------------------
********************************************************************************************************************/

#ifndef _device_w25q64_h_
#define _device_w25q64_h_

#include "common_headfile.h"

// W25Q64_USE_SOFT_SPI定义为0表示使用硬件SPI驱动 定义为1表示使用软件SPI驱动
// 当更改W25Q64_USE_SOFT_SPI定义后，需要先编译并下载程序，单片机与模块需要断电重启才能正常通讯
#define W25Q64_USE_SOFT_SPI         (0)                                       // 默认使用硬件 SPI 方式驱动	

#if W25Q64_USE_SOFT_SPI                                                       // 这两段 颜色正常的才是正确的 颜色灰的就是没有用的
//====================================================软件 SPI 驱动====================================================
#define W25Q64_SOFT_SPI_DELAY           (0 )                                    // 软件 SPI 的时钟延时周期 数值越小 SPI 通信速率越快	24 MHz 主频：2 ~ 4。	48 MHz 主频：4 ~ 8。	72 MHz 主频：8 ~ 15
#define W25Q64_SCL_PIN                  (PA7)                                 // 软件 SPI SCK 引脚
#define W25Q64_MOSI_PIN                 (PA5)                                 // 软件 SPI MOSI 引脚
#define W25Q64_MISO_PIN                 (PA6)                                 // 软件 SPI MISO 引脚
//====================================================软件 SPI 驱动====================================================
#else

//====================================================硬件 SPI 驱动====================================================
#define W25Q64_SPI_SPEED          (10 * 1000 * 1000)                          // 硬件 SPI 速率		spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）  
#define W25Q64_SPI                (SPI_1)                                     // 硬件 SPI 号
#define W25Q64_SPC_PIN            (SPI1_SCLK_PA5)                         // 硬件 SPI SCK 引脚
#define W25Q64_SDI_PIN            (SPI1_MOSI_PA7)                          // 硬件 SPI MOSI 引脚
#define W25Q64_SDO_PIN            (SPI1_MISO_PA6)                          // 硬件 SPI MISO 引脚
//====================================================硬件 SPI 驱动====================================================
#endif
#define W25Q64_CS_PIN             (PA4)                                       // CS 片选引脚,低电平有效
#define W25Q64_CS(x)              ((x) ? (gpio_high(W25Q64_CS_PIN)) : (gpio_low(W25Q64_CS_PIN)))
//================================================定义 W25Q64 内部地址================================================
#define W25Q64_WRITE_ENABLE												(0x06)
#define W25Q64_WRITE_DISABLE											(0x04)
#define W25Q64_READ_STATUS_REGISTER_1							(0x05)
#define W25Q64_READ_STATUS_REGISTER_2							(0x35)
#define W25Q64_WRITE_STATUS_REGISTER							(0x01)
#define W25Q64_PAGE_PROGRAM												(0x02)
#define W25Q64_QUAD_PAGE_PROGRAM									(0x32)
#define W25Q64_BLOCK_ERASE_64KB										(0xD8)
#define W25Q64_BLOCK_ERASE_32KB										(0x52)
#define W25Q64_SECTOR_ERASE_4KB										(0x20)
#define W25Q64_CHIP_ERASE													(0xC7)
#define W25Q64_ERASE_SUSPEND											(0x75)
#define W25Q64_ERASE_RESUME												(0x7A)
#define W25Q64_POWER_DOWN													(0xB9)
#define W25Q64_HIGH_PERFORMANCE_MODE							(0xA3)
#define W25Q64_CONTINUOUS_READ_MODE_RESET					(0xFF)
#define W25Q64_RELEASE_POWER_DOWN_HPM_DEVICE_ID		(0xAB)
#define W25Q64_MANUFACTURER_DEVICE_ID							(0x90)
#define W25Q64_READ_UNIQUE_ID											(0x4B)
#define W25Q64_JEDEC_ID														(0x9F)
#define W25Q64_READ_DATA													(0x03)
#define W25Q64_FAST_READ													(0x0B)
#define W25Q64_FAST_READ_DUAL_OUTPUT							(0x3B)
#define W25Q64_FAST_READ_DUAL_IO									(0xBB)
#define W25Q64_FAST_READ_QUAD_OUTPUT							(0x6B)
#define W25Q64_FAST_READ_QUAD_IO									(0xEB)
#define W25Q64_OCTAL_WORD_READ_QUAD_IO						(0xE3)

#define W25Q64_DUMMY_BYTE													(0xFF)
//================================================定义 W25Q64 内部地址================================================
void w25q64_sector_erase(uint32 addr);
void w25q64_page_program(uint32 addr, const uint8 *buf, uint16 len);
void w25q64_read_data(uint32 addr, uint8 *buf, uint32 len);


#endif


