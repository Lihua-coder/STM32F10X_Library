#ifndef _DEVICE_HC04_H_
#define _DEVICE_HC04_H_

#include "common_headfile.h"
/* =========================  用户可改宏  ========================= */
#define HC04_UART           UART_3                // 使用哪一路串口 
#define HC04_UART_BAUD      9600                  // 串口通信波特率 /
#define HC04_TX_PIN         UART3_TX_PB10         // TX 引脚 /
#define HC04_RX_PIN         UART3_RX_PB11         // RX 引脚 */
#define HC04_KEY_PIN        PA8                   /* KEY/SET 引脚（高电平进入 AT） */
#define HC04_RST_PIN        PA9                   // RST 引脚（低电平复位） */
/* 掉电保存参数，AT 指令仅需在上电时发送一次 */
#define HC04_NAME           "SeekFree_HC04"       // 蓝牙名称      /
#define HC04_PASSWORD       "1234"                // 配对密码      /
#define HC04_BAUD           9600                  //蓝牙空中速率  /
#define HC04_DATABIT        8                     // 数据位 8/9    /
#define HC04_PARITY         0                     // 0=None 1=Odd 2=Even /
#define HC04_STOPBIT        1                     // 1 或 2        /
#define HC04_FLOWCTRL       0                     // 0=无流控 1=有 */
/* 强制上电进入 AT 模式（保持 KEY=1），配置完成后自动退出 */
#define HC04_FORCE_AT_CFG   1
// ============================================================= */
/* 对外接口：与逐飞助手对接的钩子 */
uint32 hc04_send_buffer(const uint8 *buf, uint32 len);
uint32 hc04_read_buffer(uint8 *buf, uint32 len);
/* 模块初始化：完成串口+引脚+AT参数配置 */
void   hc04_init(void);

#endif

