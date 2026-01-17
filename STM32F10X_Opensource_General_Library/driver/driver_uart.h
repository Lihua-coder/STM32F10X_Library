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

#ifndef __DRIVER_UART_H
#define __DRIVER_UART_H
#include "common_headfile.h"


//====================================================串口 基础函数====================================================
void    uart_write_byte                     (uart_index_enum uartn, const uint8 dat);
void    uart_write_buffer                   (uart_index_enum uartn, const uint8 *buff, uint32 len);
void    uart_write_string                   (uart_index_enum uartn, const char *str);

uint8   uart_read_byte                      (uart_index_enum uartn);
uint8   uart_query_byte                     (uart_index_enum uartn, uint8 *dat);

void    uart_tx_interrupt                   (uart_index_enum uartn, uint32 status);
//void    uart_rx_interrupt                   (uart_index_enum uartn, uint32 status);

void    uart_sbus_init                      (uart_index_enum uartn, uint32 baud, uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin);
void    uart_init                           (uart_index_enum uartn, uint32 baud, uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin);
//====================================================串口 基础函数====================================================
void uart_send_number(uart_index_enum uartn, uint32 number, uint8 length);

void uart_send_hexpacket(uart_index_enum uart_n,  uint8 hdr_len, const uint8 packethdr, const uint8 *array, uint32 len, const uint8 packetend, uint8 end_len);
void uart_send_txtpacket(uart_index_enum uart_n,  uint8 hdr_len, const char *packethdr, const char *str, uint32 len, const char *packetend, uint8 end_len);



#endif


