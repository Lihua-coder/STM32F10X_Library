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
*                   EN                  查看 device_esp8266.h 中 ESP8266_EN 宏定义
*                   RX                  查看 device_esp8266.h 中 ESP8266_RX 宏定义
*                   TX                  查看 device_esp8266.h 中 ESP8266_TX 宏定义
*                   VCC                 3.3V电源
*                   GND                 电源地
*                   其余引脚悬空
*                   ------------------------------------
********************************************************************************************************************/
#include "device_esp8266.h"

#define REV_OK		0	//接收完成标志
#define REV_WAIT	1	//接收未完成标志

unsigned char esp8266_buf[512];
unsigned short esp8266_cnt = 0;
unsigned short esp8266_cntPre = 0;

/* 内部使用：把 FIFO 里所有已接收字节一次性倒进 esp8266_buf */
static void flush_fifo_to_buf(void)
{
    uint8 dat;
    while (uart_query_byte(ESP8266_UART, &dat) == 1)   /* FIFO 非空 */
    {
        if (esp8266_cnt < sizeof(esp8266_buf) - 1)
            esp8266_buf[esp8266_cnt++] = dat;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     清空 ESP8266 接收缓冲区
// 参数说明     void
// 返回参数     void
// 使用示例     esp8266_clear();
// 备注信息     将全局缓冲区 esp8266_buf 清零，并复位计数器
//-------------------------------------------------------------------------------------------------------------------
void esp8266_clear(void)
{
    memset(esp8266_buf, 0, sizeof(esp8266_buf));
    esp8266_cnt = 0;
    esp8266_cntPre = 0;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     等待 ESP8266 完成一次数据接收
// 参数说明     void
// 返回参数     _Bool           REV_OK-接收完成  REV_WAIT-仍在等待
// 使用示例     if(esp8266_waitrecive()==REV_OK) { ... }
// 备注信息     采用“两次计数相等即判定帧结束”策略
//-------------------------------------------------------------------------------------------------------------------
_Bool esp8266_waitrecive(void)
{
    flush_fifo_to_buf();              /* 先同步一次 */

    if (esp8266_cnt == 0)             /* 还没收到任何数据 */
        return REV_WAIT;

    if (esp8266_cnt == esp8266_cntPre)/* 长度不再增加 → 结束 */
    {
        esp8266_cnt = 0;              /* 清下标，方便下次 */
        return REV_OK;
    }
    esp8266_cntPre = esp8266_cnt;     /* 继续等 */
    return REV_WAIT;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     向 ESP8266 发送 AT 指令并等待指定应答
// 参数说明     cmd							待发送指令字符串
// 参数说明     res							期望应答子串
// 返回参数     _Bool           0-成功收到应答  1-超时未收到
// 使用示例     if(!esp8266_sendcmd("AT\r\n","OK")) { ... }
// 备注信息     指令未以换行结尾时会自动补发 "\r\n"
//-------------------------------------------------------------------------------------------------------------------
_Bool esp8266_sendcmd(char *cmd, char *res)
{
    unsigned char timeOut = 200;      /* 200*10 ms = 2 s */

    uart_write_string(ESP8266_UART, cmd);
    if (cmd[strlen(cmd) - 1] != '\n')
        uart_write_string(ESP8266_UART, "\r\n");

    while (timeOut--)
    {
        if (esp8266_waitrecive() == REV_OK)
        {
            if (strstr((char *)esp8266_buf, res) != NULL)
            {
                esp8266_clear();
                return 0;
            }
        }
        system_delay_ms(10);
    }
    return 1;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     通过透传模式向服务器发送指定长度数据
// 参数说明     data							待发送数据指针  
// 参数说明     len								数据长度
// 返回参数     void
// 使用示例     esp8266_senddata(buf, 120);
// 备注信息     内部先发送 AT+CIPSEND 指令，收到 ">" 后继续发数据
//-------------------------------------------------------------------------------------------------------------------
void esp8266_senddata(unsigned char *data, unsigned short len)
{
    char cmdBuf[32];
    esp8266_clear();
    sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);
    if (!esp8266_sendcmd(cmdBuf, ">"))
        uart_write_buffer(ESP8266_UART, data, len);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     从接收缓冲区中提取 +IPD 数据 payload 指针
// 参数说明     timeout								最大等待时长（单位 5 ms）
// 返回参数     unsigned char*  			成功返回 payload 首地址，失败返回 NULL
// 使用示例     ptr = esp8266_getipd(200);
// 备注信息     仅返回第一个 +IPD 帧的 payload 起始位置
//-------------------------------------------------------------------------------------------------------------------
unsigned char *esp8266_getipd(unsigned short timeout)
{
    char *ptrIPD;
    do
    {
        if (esp8266_waitrecive() == REV_OK)
        {
            ptrIPD = strstr((char *)esp8266_buf, "IPD,");
            if (ptrIPD == NULL);   /* 继续等 */
            else
            {
                ptrIPD = strchr(ptrIPD, ':');
                if (ptrIPD != NULL)
                    return (unsigned char *)(ptrIPD + 1);
                else
                    return NULL;
            }
        }
        system_delay_ms(5);
    } while (--timeout);
    return NULL;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     初始化 ESP8266 并连接 OneNet 平台
// 参数说明     void
// 返回参数     void
// 使用示例     esp8266_init();
// 备注信息     依次完成 AT、CWMODE、CWDHCP、CWJAP、CIPSTART 五步
//-------------------------------------------------------------------------------------------------------------------
void esp8266_init(void)/* 调试信息可改 PRINT_UART，这里用 ESP8266_UART 发 AT，PRINT_UART 打印日志 */
{
		gpio_init(ESP8266_EN, GPO_PUSH_PULL, 1);//高电平使能
		uart_init(ESP8266_UART, 115200, ESP8266_RX, ESP8266_TX);
    esp8266_clear();

    uart_write_string(PRINT_UART, "1. AT\r\n");
    while (esp8266_sendcmd("AT\r\n", "OK"))
        system_delay_ms(500);

    uart_write_string(PRINT_UART, "2. CWMODE\r\n");
    while (esp8266_sendcmd("AT+CWMODE=1\r\n", "OK"))
        system_delay_ms(500);

    uart_write_string(PRINT_UART, "3. AT+CWDHCP\r\n");
    while (esp8266_sendcmd("AT+CWDHCP=1,1\r\n", "OK"))
        system_delay_ms(500);

    uart_write_string(PRINT_UART, "4. CWJAP\r\n");
    while (esp8266_sendcmd(ESP8266_WIFI_INFO, "GOT IP"))
        system_delay_ms(500);

    uart_write_string(PRINT_UART, "5. CIPSTART\r\n");
    while (esp8266_sendcmd(ESP8266_ONENET_INFO, "CONNECT"))
        system_delay_ms(500);

    uart_write_string(PRINT_UART, "6. ESP8266 Init OK\r\n");
}

