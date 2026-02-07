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

#ifndef _driver_uart_h_
#define _driver_uart_h_

#include "common_headfile.h"
// 1表示使用DMA驱动，0表示使用普通驱动
// 当更改定义后，需要先编译并下载程序，单片机与模块需要断电重启才能正常通讯
#define UART_TX_USE_DMA         (0)                                       // 默认使用 DMA 方式驱动	
#define UART_RX_USE_DMA         (0)                                       // 默认使用 DMA 方式驱动	


// USART 中断优先级（数值越小优先级越高，范围0-15）
#define UART1_NVIC_PREEMPT_PRIORITY     (3)
#define UART2_NVIC_PREEMPT_PRIORITY     (4)
#define UART3_NVIC_PREEMPT_PRIORITY     (5)

// DMA 中断优先级（必须高于对应USART，防止数据覆盖）
#define UART1_DMA_NVIC_PREEMPT_PRIORITY (0)
#define UART2_DMA_NVIC_PREEMPT_PRIORITY (1)
#define UART3_DMA_NVIC_PREEMPT_PRIORITY (2)


/* 内部循环缓冲区大小 */
#define UART_RX_BUF_SIZE  256
//====================================================串口 基础函数====================================================
void    uart_write_byte                     (uart_index_enum uartn, const uint8 dat);
void    uart_write_buffer                   (uart_index_enum uartn, const uint8 *buff, uint32 len);
void    uart_write_string                   (uart_index_enum uartn, const char *str);

uint8   uart_read_byte                      (uart_index_enum uartn);
uint8   uart_query_byte                     (uart_index_enum uartn, uint8 *dat);

void    uart_init                           (uart_index_enum uartn, uint32 baud, uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin);
//====================================================串口 基础函数=====================================================



#endif


