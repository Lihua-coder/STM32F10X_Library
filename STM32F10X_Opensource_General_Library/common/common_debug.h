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
#ifndef _common_debug_h_
#define _common_debug_h_

#include "common_headfile.h"

#define DEBUG_UART_INDEX            (UART_1)            // 指定 debug uart 所使用的的串口
#define DEBUG_UART_BAUDRATE         (115200)            // 指定 debug uart 所使用的的串口波特率
#define DEBUG_UART_TX_PIN           (UART1_TX_PA9)    // 指定 debug uart 所使用的的串口引脚
#define DEBUG_UART_RX_PIN           (UART1_RX_PA10)    // 指定 debug uart 所使用的的串口引脚
#define DEBUG_UART_USE_INTERRUPT    (1)                 // 是否启用 debug uart 接收中断

uint32 debug_send_buffer(const uint8 *buff, uint32 len);
uint32 debug_read_ring_buffer (uint8 *buff, uint32 len);
void debug_init (void);


void debug_assert_handler (uint8 pass, char *file, int line);
void debug_log_handler (uint8 pass, char *str, char *file, int line);


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     Log 信息输出
// 参数说明     x           判断是否触发输出 0-触发输出 1-不触发输出
// 参数说明     *str        需要输出的 Log 信息
// 返回参数     void
// 使用示例     zf_log(0, "Error");
// 备注信息     调试信息输出 用来做一些报错或者警告之类的输出
//             默认情况下会在 Debug UART 输出
//             但如果使用开源库内屏幕接口初始化了屏幕 则会在屏幕上显示
//-------------------------------------------------------------------------------------------------------------------
#define zf_log(x, str)              (debug_log_handler((x), (str), __FILE__, __LINE__)) // 调试信息输出 用来做一些报错或者警告之类的输出
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     断言
// 参数说明     x           判断是否触发断言 0-触发断言 1-不触发断言
// 返回参数     void
// 使用示例     zf_assert(0);
// 备注信息     一般用于参数判断 zf_assert(0) 就断言报错
//              默认情况下会在 Debug UART 输出
//              但如果使用开源库内屏幕接口初始化了屏幕 则会在屏幕上显示
//-------------------------------------------------------------------------------------------------------------------
#define zf_assert(x)                (debug_assert_handler((x), __FILE__, __LINE__))





#endif

