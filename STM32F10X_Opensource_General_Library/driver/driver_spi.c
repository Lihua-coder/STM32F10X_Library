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
#include "driver_spi.h"

/*-------------------- 私有宏 / 常量 --------------------*/
static SPI_TypeDef* const SPIx[3] = {NULL, SPI1, SPI2};

/*-------------------- 私有函数 --------------------*/
//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI 读写 1Byte（内部调用）
// 参数说明          spi_n        SPI模块号
// 参数说明          tx           发送字节
// 返回类型          uint8        接收字节
// 使用示例          uint8 rx = spi_rw_byte(SPI_1, 0xA5);
// 备注信息          阻塞等待TXE/RXNE，未加超时
//-------------------------------------------------------------------------------------------------------------------
static uint8 spi_rw_byte(spi_index_enum spi_n, uint8 tx)
{
    SPI_TypeDef* spix = SPIx[spi_n];
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(spix, tx);
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_RXNE) == RESET);
    return (uint8)SPI_I2S_ReceiveData(spix);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          打包引脚号解包为GPIO端口与Pin
// 参数说明          packed       打包值（port*102+pin）
// 参数说明          *port        输出GPIO端口
// 参数说明          *pin         输出Pin位图
// 返回类型          void
// 使用示例          spi_pin_unpack(SPI1_SCLK_PB3, &port, &pin);
// 备注信息          内部工具函数
//-------------------------------------------------------------------------------------------------------------------
static void spi_pin_unpack(uint32 packed, GPIO_TypeDef** port, uint16* pin)
{
    uint8 portIdx = (packed / 102) - 1;
    uint8 pinIdx  = (packed % 102) % 6 + ((packed % 102) / 6) * 16;
    *port = (GPIO_TypeDef*)(GPIOA_BASE + (portIdx << 10));
    *pin  = 1 << pinIdx;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          判断是否为SPI1重映射引脚
// 参数说明          packed       打包值
// 返回类型          bool         1-是重映射脚 0-否
// 使用示例          if(is_spi1_remap_pin(SPI1_SCLK_PB3)) ...
// 备注信息          内部工具函数
//-------------------------------------------------------------------------------------------------------------------
static bool is_spi1_remap_pin(uint32 packed)
{
    return (packed == SPI1_SCLK_PB3)  ||
           (packed == SPI1_MISO_PB4)  ||
           (packed == SPI1_MOSI_PB5)  ||
           (packed == SPI1_CS_PA15);
}

/*==================== 对外接口实现 ====================*/
//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI写8位数据
// 参数说明          spi_n        SPI模块号
// 参数说明          data         待发送字节
// 返回类型          void
// 使用示例          spi_write_8bit(SPI_1, 0x55);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_write_8bit(spi_index_enum spi_n, const uint8 data)
{
    (void)spi_rw_byte(spi_n, data);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI写8位数组
// 参数说明          spi_n        SPI模块号
// 参数说明          *data        数据缓冲区
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_write_8bit_array(SPI_1, buf, 10);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_write_8bit_array(spi_index_enum spi_n, const uint8 *data, uint32 len)
{
    while (len--) spi_write_8bit(spi_n, *data++);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI写16位数据
// 参数说明          spi_n        SPI模块号
// 参数说明          data         待发送字
// 返回类型          void
// 使用示例          spi_write_16bit(SPI_1, 0x1234);
// 备注信息          先发高8位
//-------------------------------------------------------------------------------------------------------------------
void spi_write_16bit(spi_index_enum spi_n, const uint16 data)
{
    spi_write_8bit(spi_n, data >> 8);
    spi_write_8bit(spi_n, data);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI写16位数组
// 参数说明          spi_n        SPI模块号
// 参数说明          *data        数据缓冲区
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_write_16bit_array(SPI_1, buf, 5);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_write_16bit_array(spi_index_enum spi_n, const uint16 *data, uint32 len)
{
    while (len--) spi_write_16bit(spi_n, *data++);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI写寄存器（8位）
// 参数说明          spi_n        SPI模块号
// 参数说明          register_name 寄存器地址
// 参数说明          data         待写数据
// 返回类型          void
// 使用示例          spi_write_8bit_register(SPI_1, 0x20, 0x0F);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_write_8bit_register(spi_index_enum spi_n, const uint8 register_name, const uint8 data)
{
    spi_write_8bit(spi_n, register_name);
    spi_write_8bit(spi_n, data);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI写寄存器连续多字节（8位）
// 参数说明          spi_n        SPI模块号
// 参数说明          register_name 起始寄存器地址
// 参数说明          *data        数据缓冲区
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_write_8bit_registers(SPI_1, 0x20, buf, 6);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_write_8bit_registers(spi_index_enum spi_n, const uint8 register_name, const uint8 *data, uint32 len)
{
    spi_write_8bit(spi_n, register_name);
    spi_write_8bit_array(spi_n, data, len);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI写寄存器（16位）
// 参数说明          spi_n        SPI模块号
// 参数说明          register_name 寄存器地址
// 参数说明          data         待写数据
// 返回类型          void
// 使用示例          spi_write_16bit_register(SPI_1, 0x2021, 0x0FF0);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_write_16bit_register(spi_index_enum spi_n, const uint16 register_name, const uint16 data)
{
    spi_write_16bit(spi_n, register_name);
    spi_write_16bit(spi_n, data);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI写寄存器连续多字（16位）
// 参数说明          spi_n        SPI模块号
// 参数说明          register_name 起始寄存器地址
// 参数说明          *data        数据缓冲区
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_write_16bit_registers(SPI_1, 0x2021, buf, 4);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_write_16bit_registers(spi_index_enum spi_n, const uint16 register_name, const uint16 *data, uint32 len)
{
    spi_write_16bit(spi_n, register_name);
    spi_write_16bit_array(spi_n, data, len);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI读8位数据
// 参数说明          spi_n        SPI模块号
// 返回类型          uint8        接收字节
// 使用示例          uint8 dat = spi_read_8bit(SPI_1);
// 备注信息          发送0xFF以读取
//-------------------------------------------------------------------------------------------------------------------
uint8 spi_read_8bit(spi_index_enum spi_n)
{
    return spi_rw_byte(spi_n, 0xFF);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI读8位数组
// 参数说明          spi_n        SPI模块号
// 参数说明          *data        接收缓冲区
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_read_8bit_array(SPI_1, buf, 10);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_read_8bit_array(spi_index_enum spi_n, uint8 *data, uint32 len)
{
    while (len--) *data++ = spi_read_8bit(spi_n);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI读16位数据
// 参数说明          spi_n        SPI模块号
// 返回类型          uint16       接收字
// 使用示例          uint16 dat = spi_read_16bit(SPI_1);
// 备注信息          先读高8位
//-------------------------------------------------------------------------------------------------------------------
uint16 spi_read_16bit(spi_index_enum spi_n)
{
    uint16 h = spi_read_8bit(spi_n);
    uint16 l = spi_read_8bit(spi_n);
    return (h << 8) | l;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI读16位数组
// 参数说明          spi_n        SPI模块号
// 参数说明          *data        接收缓冲区
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_read_16bit_array(SPI_1, buf, 5);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_read_16bit_array(spi_index_enum spi_n, uint16 *data, uint32 len)
{
    while (len--) *data++ = spi_read_16bit(spi_n);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI读寄存器（8位）
// 参数说明          spi_n        SPI模块号
// 参数说明          register_name 寄存器地址
// 返回类型          uint8        寄存器值
// 使用示例          uint8 val = spi_read_8bit_register(SPI_1, 0x0F);
// 备注信息          最高位自动置1
//-------------------------------------------------------------------------------------------------------------------
uint8 spi_read_8bit_register(spi_index_enum spi_n, const uint8 register_name)
{
    spi_write_8bit(spi_n, register_name | 0x80);
    return spi_read_8bit(spi_n);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI读寄存器连续多字节（8位）
// 参数说明          spi_n        SPI模块号
// 参数说明          register_name 起始寄存器地址
// 参数说明          *data        接收缓冲区
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_read_8bit_registers(SPI_1, 0x0F, buf, 6);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_read_8bit_registers(spi_index_enum spi_n, const uint8 register_name, uint8 *data, uint32 len)
{
    spi_write_8bit(spi_n, register_name | 0x80);
    spi_read_8bit_array(spi_n, data, len);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI读寄存器（16位）
// 参数说明          spi_n        SPI模块号
// 参数说明          register_name 寄存器地址
// 返回类型          uint16       寄存器值
// 使用示例          uint16 val = spi_read_16bit_register(SPI_1, 0x2021);
// 备注信息          最高位自动置1
//-------------------------------------------------------------------------------------------------------------------
uint16 spi_read_16bit_register(spi_index_enum spi_n, const uint16 register_name)
{
    spi_write_16bit(spi_n, register_name | 0x8000);
    return spi_read_16bit(spi_n);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI读寄存器连续多字（16位）
// 参数说明          spi_n        SPI模块号
// 参数说明          register_name 起始寄存器地址
// 参数说明          *data        接收缓冲区
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_read_16bit_registers(SPI_1, 0x2021, buf, 4);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_read_16bit_registers(spi_index_enum spi_n, const uint16 register_name, uint16 *data, uint32 len)
{
    spi_write_16bit(spi_n, register_name | 0x8000);
    spi_read_16bit_array(spi_n, data, len);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI 8bit全双工数据传输
// 参数说明          spi_n        SPI模块号
// 参数说明          write_buffer 发送缓冲区（NULL则只接收）
// 参数说明          read_buffer  接收缓冲区（NULL则只发送）
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_transfer_8bit(SPI_1, txbuf, rxbuf, 8);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_transfer_8bit(spi_index_enum spi_n, const uint8 *write_buffer, uint8 *read_buffer, uint32 len)
{
    if (!read_buffer) { spi_write_8bit_array(spi_n, write_buffer, len); return; }
    while (len--) *read_buffer++ = spi_rw_byte(spi_n, *write_buffer++);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI 16bit全双工数据传输
// 参数说明          spi_n        SPI模块号
// 参数说明          write_buffer 发送缓冲区（NULL则只接收）
// 参数说明          read_buffer  接收缓冲区（NULL则只发送）
// 参数说明          len          长度
// 返回类型          void
// 使用示例          spi_transfer_16bit(SPI_1, txbuf, rxbuf, 4);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_transfer_16bit(spi_index_enum spi_n, const uint16 *write_buffer, uint16 *read_buffer, uint32 len)
{
    if (!read_buffer) { spi_write_16bit_array(spi_n, write_buffer, len); return; }
    while (len--)
    {
        uint16 h = spi_rw_byte(spi_n, *write_buffer >> 8);
        uint16 l = spi_rw_byte(spi_n, *write_buffer & 0xFF);
        *read_buffer++ = (h << 8) | l;
        write_buffer++;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI初始化（整改版）
// 参数说明          spi_n        SPI模块号（SPI_1/SPI_2）
// 参数说明          mode         模式0-3
// 参数说明          baud         波特率（Hz）
// 参数说明          sck_pin      SCK引脚
// 参数说明          mosi_pin     MOSI引脚
// 参数说明          miso_pin     MISO引脚
// 参数说明          cs_pin       CS引脚
// 返回类型          void
// 使用示例          spi_init(SPI_1, SPI_MODE0, 8000000, SPI1_SCLK_PA5, SPI1_MOSI_PA7, SPI1_MISO_PA6, SPI1_CS_PA4);
// 备注信息          使用PB3/PB4/PA15时自动关闭JTAG，保留SWD
//-------------------------------------------------------------------------------------------------------------------
void spi_init(spi_index_enum spi_n, spi_mode_enum mode, uint32 baud, spi_sck_pin_enum  sck_pin, spi_mosi_pin_enum mosi_pin, spi_miso_pin_enum miso_pin, spi_cs_pin_enum cs_pin)
{
    SPI_TypeDef* spix = SPIx[spi_n];
    GPIO_TypeDef *sck_port, *mosi_port, *miso_port, *cs_port;
    uint16       sck_pin_, mosi_pin_, miso_pin_, cs_pin_;

    /* 1. 开时钟 */
    if (spi_n == SPI_1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

        /* 若用到重映射脚，关闭 JTAG，保留 SWD */
        if (is_spi1_remap_pin(sck_pin)  ||
            is_spi1_remap_pin(mosi_pin) ||
            is_spi1_remap_pin(miso_pin) ||
            (cs_pin != SPI_CS_NULL && is_spi1_remap_pin(cs_pin)))
        {
            GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
        }
        GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
    }
    else /* SPI2 */
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    }

    /* 2. 解包引脚 */
    spi_pin_unpack(sck_pin,  &sck_port,  &sck_pin_);
    spi_pin_unpack(mosi_pin, &mosi_port, &mosi_pin_);
    spi_pin_unpack(miso_pin, &miso_port, &miso_pin_);
    if (cs_pin != SPI_CS_NULL)
        spi_pin_unpack(cs_pin, &cs_port, &cs_pin_);

    /* 3. 配置 GPIO */
    GPIO_InitTypeDef gpio;
    GPIO_StructInit(&gpio);

    /* SCK/MOSI 复用推挽 */
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Pin = sck_pin_;
    GPIO_Init(sck_port, &gpio);
    gpio.GPIO_Pin = mosi_pin_;
    GPIO_Init(mosi_port, &gpio);

    /* MISO 浮空输入 */
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Pin = miso_pin_;
    GPIO_Init(miso_port, &gpio);

    /* CS 推挽输出，默认高 */
    if (cs_pin != SPI_CS_NULL)
    {
        gpio.GPIO_Mode = GPIO_Mode_Out_PP;
        gpio.GPIO_Pin = cs_pin_;
        GPIO_Init(cs_port, &gpio);
        GPIO_SetBits(cs_port, cs_pin_);
    }

    /* 4. 配置 SPI 外设 */
    SPI_InitTypeDef spi;
    SPI_StructInit(&spi);
    spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode      = SPI_Mode_Master;
    spi.SPI_DataSize  = SPI_DataSize_8b;
    spi.SPI_CPOL      = (mode & 0x02) ? SPI_CPOL_High : SPI_CPOL_Low;
    spi.SPI_CPHA      = (mode & 0x01) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
    spi.SPI_NSS       = SPI_NSS_Soft;

    /* 按 APB 时钟计算分频，SPI1-72M，SPI2-36M */
    const uint32 apbclk = (spi_n == SPI_1) ? 72000000 : 36000000;
    spi.SPI_BaudRatePrescaler =
        (baud >= apbclk / 2)   ? SPI_BaudRatePrescaler_2 :
        (baud >= apbclk / 4)   ? SPI_BaudRatePrescaler_4 :
        (baud >= apbclk / 8)   ? SPI_BaudRatePrescaler_8 :
        (baud >= apbclk / 16)  ? SPI_BaudRatePrescaler_16 :
        (baud >= apbclk / 32)  ? SPI_BaudRatePrescaler_32 :
        (baud >= apbclk / 64)  ? SPI_BaudRatePrescaler_64 :
        (baud >= apbclk / 128) ? SPI_BaudRatePrescaler_128 :
                                 SPI_BaudRatePrescaler_256;

    spi.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(spix, &spi);
    SPI_Cmd(spix, ENABLE);          /* 仅保留一次 */
}

