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
*                   // 硬件 SPI 引脚
*                   SCL/SPC           查看 device_w25q64.h 中 W25Q64_SPC_PIN 宏定义
*                   SDA/DSI           查看 device_w25q64.h 中 W25Q64_SDI_PIN 宏定义
*                   SA0/SDO           查看 device_w25q64.h 中 W25Q64_SDO_PIN 宏定义
*                   CS                查看 device_w25q64.h 中 W25Q64_CS_PIN 宏定义
*                   VCC               3.3V电源
*                   GND               电源地
*                   其余引脚悬空
*
*                   // 软件 SPI 引脚
*                   同上
*                   ------------------------------------
********************************************************************************************************************/
#include "device_w25q64.h"

#if W25Q64_USE_SOFT_SPI
static soft_spi_info_struct w25q64_spi_struct;

#define w25q64_write_byte(dat)        soft_spi_write_8bit(&w25q64_spi_struct, (dat))
#define w25q64_write_bytes(data, len) soft_spi_write_8bit_array(&w25q64_spi_struct, (data), (len))
#define w25q64_read_byte()            soft_spi_read_8bit(&w25q64_spi_struct)
#define w25q64_read_bytes(data, len)  soft_spi_read_8bit_array(&w25q64_spi_struct, (data), (len))   
#else
#define w25q64_write_byte(dat)        spi_write_8bit(W25Q64_SPI, (dat))
#define w25q64_write_bytes(data, len) spi_write_8bit_array(W25Q64_SPI, (data), (len))
#define w25q64_read_byte()            spi_read_8bit(W25Q64_SPI)
#define w25q64_read_bytes(data, len)  spi_read_8bit_array(W25Q64_SPI, (data), (len))               
#endif

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     等待 Flash 内部 BUSY 位清零
// 参数说明     void
// 返回参数     void
// 使用示例     w25q64_wait_busy();
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static void w25q64_wait_busy(void)
{
    uint8 sr;
    do {
        W25Q64_CS(0);
        w25q64_write_byte(W25Q64_READ_STATUS_REGISTER_1);
        sr = w25q64_read_byte();
        W25Q64_CS(1);
    } while (sr & 0x01);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     写使能
// 参数说明     void
// 返回参数     void
// 使用示例     w25q64_write_enable();
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static void w25q64_write_enable(void)
{
    W25Q64_CS(0);
    w25q64_write_byte(W25Q64_WRITE_ENABLE);
    W25Q64_CS(1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     校验 JEDEC ID，判断芯片是否存在
// 参数说明     void
// 返回参数     uint8   0-成功 1-失败
// 使用示例     if(w25q64_self_check()) { /* 错误处理 */ }
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static uint8 w25q64_self_check(void)
{
    uint8 id[3];
    W25Q64_CS(0);
    w25q64_write_byte(W25Q64_JEDEC_ID);
    w25q64_read_bytes(id, 3);
    W25Q64_CS(1);
    /* W25Q64 固定返回 0xEF 0x40 0x17 */
    if (id[0] == 0xEF && id[1] == 0x40 && id[2] == 0x17)
        return 0;
    return 1;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     4 KB 扇区擦除
// 参数说明     addr   4 KB 对齐地址
// 返回参数     void
// 使用示例     w25q64_sector_erase(0x000000);
// 备注信息     擦除后区域变为 0xFF
//-------------------------------------------------------------------------------------------------------------------
void w25q64_sector_erase(uint32 addr)
{
    w25q64_write_enable();
    W25Q64_CS(0);
    w25q64_write_byte(W25Q64_SECTOR_ERASE_4KB);
    w25q64_write_byte((addr >> 16) & 0xFF);
    w25q64_write_byte((addr >> 8)  & 0xFF);
    w25q64_write_byte(addr & 0xFF);
    W25Q64_CS(1);
    w25q64_wait_busy();
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     页编程（≤256 Byte）
// 参数说明     addr   起始地址
// 参数说明     buf    数据源
// 参数说明     len    长度（1-256）
// 返回参数     void
// 使用示例     w25q64_page_program(0x000000, buf, 128);
// 备注信息     函数内已包含写使能与忙等待
//-------------------------------------------------------------------------------------------------------------------
void w25q64_page_program(uint32 addr, const uint8 *buf, uint16 len)
{
    if (!len || len > 256) return;
    w25q64_write_enable();
    W25Q64_CS(0);
    w25q64_write_byte(W25Q64_PAGE_PROGRAM);
    w25q64_write_byte((addr >> 16) & 0xFF);
    w25q64_write_byte((addr >> 8)  & 0xFF);
    w25q64_write_byte(addr & 0xFF);
    w25q64_write_bytes(buf, len);
    W25Q64_CS(1);
    w25q64_wait_busy();
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     读取 Flash 任意长度数据
// 参数说明     addr   起始地址
// 参数说明     buf    接收缓存
// 参数说明     len    长度
// 返回参数     void
// 使用示例     w25q64_read_data(0x000000, buf, 256);
// 备注信息     无
//-------------------------------------------------------------------------------------------------------------------
void w25q64_read_data(uint32 addr, uint8 *buf, uint32 len)
{
    W25Q64_CS(0);
    w25q64_write_byte(W25Q64_READ_DATA);
    w25q64_write_byte((addr >> 16) & 0xFF);
    w25q64_write_byte((addr >> 8)  & 0xFF);
    w25q64_write_byte(addr & 0xFF);
    w25q64_read_bytes(buf, len);        
    W25Q64_CS(1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     W25Q64 初始化
// 参数说明     void
// 返回参数     uint8   0-成功 1-失败
// 使用示例     if(w25q64_init()) { /* 错误处理 */ }
// 备注信息     必须先调用，再使用其他 API
//-------------------------------------------------------------------------------------------------------------------
uint8 w25q64_init(void)
{
#if W25Q64_USE_SOFT_SPI
    soft_spi_init(&w25q64_spi_struct, (uint8)SPI_MODE0, (uint32)W25Q64_SOFT_SPI_DELAY, (gpio_pin_enum)W25Q64_SCL_PIN, (gpio_pin_enum)W25Q64_MOSI_PIN, (gpio_pin_enum)W25Q64_MISO_PIN, (gpio_pin_enum)SOFT_SPI_PIN_NULL);   /* CS 手动控制 */
#else
    spi_init(W25Q64_SPI, SPI_MODE0, W25Q64_SPI_SPEED, W25Q64_SPC_PIN, W25Q64_SDI_PIN, W25Q64_SDO_PIN, SPI_CS_NULL);          /* CS 手动控制 */
#endif
    gpio_init(W25Q64_CS_PIN, GPO_PUSH_PULL, 1);
    W25Q64_CS(1);

    if (w25q64_self_check())
    {
        zf_log(0, "w25q64 self check error.");
        return 1;
    }
    return 0;
}

/*-------------------- 测试例程（main.c 中引用） --------------------*/
#if 0
#include "device_w25q64.h"
void w25q64_demo(void)
{
    uint8 wbuf[256], rbuf[256];
    for (uint16 i = 0; i < 256; i++) wbuf[i] = i;

    if (w25q64_init()) { zf_log(0, "w25q64 init fail"); return; }

    w25q64_sector_erase(0x000000);              // 擦第 0 扇区
    w25q64_page_program(0x000000, wbuf, 256);   // 写 1 页
    w25q64_read_data(0x000000, rbuf, 256);      // 读回校验

    if (memcmp(wbuf, rbuf, 256) == 0)
        zf_log(0, "w25q64 test success!");
    else
        zf_log(0, "w25q64 test failed!");
}
#endif

