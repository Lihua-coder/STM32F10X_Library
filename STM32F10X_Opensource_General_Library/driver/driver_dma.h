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
#ifndef _driver_dma_h_
#define _driver_dma_h_
#include "common_headfile.h"

void dma1_init(dma_channel_enum dma1_ch, uint32 source_addr, uint32 destination_addr, uint32 datasize, uint16 dma_count, uint32 priority);
void dma1_disable(dma_channel_enum dma1_ch);
void dma1_enable(dma_channel_enum dma1_ch);
void dma1_transfer(dma_channel_enum dma1_ch, uint16_t dma_count);


#endif
