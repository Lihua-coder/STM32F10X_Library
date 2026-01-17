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

#ifndef _driver_flash_h_
#define _driver_flash_h_

#include "common_headfile.h"
/* STM32F103C8T6  64 KB */
#define FLASH_PAGE_SIZE     (1024)        /* 1 kB */
#define FLASH_BASE_ADDR     (0x08000000)
#define FLASH_PAGE(n)       (FLASH_BASE_ADDR + (n)*FLASH_PAGE_SIZE)//从最后一页开始使用

typedef union                                                                   // 固定的数据缓冲单元格式
{
    float   float_type;                                                         // float  类型
    uint32  uint32_type;                                                        // uint32 类型
    int32   int32_type;                                                         // int32  类型
    uint16  uint16_type;                                                        // uint16 类型
    int16   int16_type;                                                         // int16  类型
    uint8   uint8_type;                                                         // uint8  类型
    int8    int8_type;                                                          // int8   类型
}flash_data_union;                                                              // 所有类型数据共用同一个 32bit 地址

extern flash_data_union flash_union_buffer[FLASH_PAGE_SIZE / 4]; // 256 个 32-bit 字

//====================================================FLASH 基础函数====================================================
uint8 	flash_check(uint32 page_num);
void    flash_erase_page(uint32 page_num);
void    flash_read_page(uint32 page_num, uint32 *buf, uint16 len);
void    flash_write_page(uint32 page_num, const uint32 *buf, uint16 len);
void    flash_read_page_to_buffer(uint32 page_num);
uint8 	flash_write_page_from_buffer(uint32 page_num);
void    flash_buffer_clear(void);
//====================================================FLASH 基础函数====================================================

#endif

