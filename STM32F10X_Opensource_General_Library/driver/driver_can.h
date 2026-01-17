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

#ifndef __DRIVER_CAN_H__
#define __DRIVER_CAN_H__

#include "common_headfile.h"

extern CanRxMsg can_rxmsg;//接收信息组
extern uint8 can_rxflag;//接收完成标志位

void can_init_no_filter(can_index_enum cann, uint8 can_mode, uint32 baud, can_tx_pin_enum tx_pin, can_rx_pin_enum rx_pin);
void can_send(can_index_enum cann, CanTxMsg *TxMessage);

#endif 

