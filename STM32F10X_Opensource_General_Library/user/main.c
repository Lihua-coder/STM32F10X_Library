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
#include "common_headfile.h"
/* 只需改这一行即可换串口 ---------------------------------------------------- */
#define DEBUG_UART      UART_1          // 也可用 UART_2 / UART_3
#define DEBUG_BAUD      115200

/* 根据上表自动配管脚 ------------------------------------------------------- */
#if DEBUG_UART == UART_1
  #define DEBUG_TX_PIN  UART1_TX_PA9
  #define DEBUG_RX_PIN  UART1_RX_PA10
#elif DEBUG_UART == UART_2
  #define DEBUG_TX_PIN  UART2_TX_PA2
  #define DEBUG_RX_PIN  UART2_RX_PA3
#elif DEBUG_UART == UART_3
  #define DEBUG_TX_PIN  UART3_TX_PB10
  #define DEBUG_RX_PIN  UART3_RX_PB11
#endif

static uint8 rx_byte;

int main(void)
{
	 system_delay_init();//禁止关闭
	 clock_gpio_init(PA);     // gpio时钟初始化


    uart_init(DEBUG_UART, DEBUG_BAUD, DEBUG_TX_PIN, DEBUG_RX_PIN);
    uart_write_string(DEBUG_UART, "hello Lihua\r\n");

    while (1)
    {
        /* 非阻塞查询接收 */
        if (uart_query_byte(DEBUG_UART, &rx_byte))        // 收到一帧
        {
            /* 原样回发 */
            uart_write_byte(DEBUG_UART, rx_byte);

            /* 如果是回车，再补个换行，方便串口助手 */
            if (rx_byte == '\r')
                uart_write_byte(DEBUG_UART, '\n');
        }
    }
	
}
