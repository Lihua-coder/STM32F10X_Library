

#include "device_hc04.h"
/* 局部变量：保存当前串口句柄 */
static uart_index_enum hc04_uart = HC04_UART;
/* 发 AT 并等待返回指定字符串，超时返回 0 */
static uint8_t at_wait(const char *cmd, const char *ack, uint32_t timeout_ms)
{
uart_write_string(hc04_uart, cmd);
uart_write_string(hc04_uart, "\r\n");
//uint32_t cnt = 0;
uint8_t  ch  = 0;
const char *p = ack;
while (timeout_ms--)
{
    if (uart_query_byte(hc04_uart, &ch))
    {
        if (ch == *p) { p++; if (*p == '\0') return 1; }
        else p = ack;                         /* 不匹配则重新开始 */
    }
    system_delay_us(1000);
}
return 0;
}
/* 发送一条 AT 指令，带参数，自动追加 \r\n */
static void at_cmd(const char *fmt, ...)
{
char buf[64];
va_list ap;
va_start(ap, fmt);
vsnprintf(buf, sizeof(buf), fmt, ap);
va_end(ap);
at_wait(buf, "OK", 500);
}
/* 引脚初始化 */
static void hc04_gpio_init(void)
{
gpio_init(HC04_KEY_PIN, GPO_PUSH_PULL, 0);
gpio_init(HC04_RST_PIN, GPO_PUSH_PULL, 1);   // 默认高电平不复位 */
}
/* 硬件复位：拉低 >200 ms 后释放 */
static void hc04_hard_reset(void)
{
gpio_low(HC04_RST_PIN);
system_delay_ms(250);
gpio_high(HC04_RST_PIN);
system_delay_ms(250);
}
/* 进入 AT 模式：KEY=1 并复位 */
static void hc04_enter_at(void)
{
gpio_high(HC04_KEY_PIN);
hc04_hard_reset();
}
/* 退出 AT 模式：KEY=0 并软复位 */
static void hc04_exit_at(void)
{
gpio_low(HC04_KEY_PIN);
uart_write_string(hc04_uart, "AT+RESET\r\n"); // 模块软复位 */
system_delay_ms(500);
}
/* 一次性下发用户参数，掉电保存 */
static void hc04_config_once(void)
{
// 确保在 AT 模式下 */
hc04_enter_at();

at_cmd("AT+NAME=%s", HC04_NAME);
at_cmd("AT+PSWD=%s", HC04_PASSWORD);
at_cmd("AT+UART=%d,%d,%d,%d",
       HC04_BAUD,
       HC04_DATABIT,
       HC04_PARITY,
       HC04_STOPBIT);
at_cmd("AT+FLOW=%d", HC04_FLOWCTRL);
at_cmd("AT+SAVE");          /* 保存到 FLASH */
hc04_exit_at();
}
/* 对外串口初始化：波特率可独立，与空中速率无关 */
void hc04_init(void)
{
hc04_gpio_init();
uart_init(hc04_uart, HC04_UART_BAUD, HC04_TX_PIN, HC04_RX_PIN);
#if (HC04_FORCE_AT_CFG)
hc04_config_once();         /* 上电即配置，只需一次 */
#endif
}
/* 逐飞助手发送钩子 */
uint32 hc04_send_buffer(const uint8 *buf, uint32 len)
{
uart_write_buffer_dma(hc04_uart, buf, len);
return 0;
}


