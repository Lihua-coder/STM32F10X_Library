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

/* 根据SPI编号获取DMA通道 */
#define SPI_TX_DMA_CH(spi_n)    ((spi_n) == SPI_1 ? DMA1_Channel3 : DMA1_Channel5)
#define SPI_RX_DMA_CH(spi_n)    ((spi_n) == SPI_1 ? DMA1_Channel2 : DMA1_Channel4)

/* 只有当至少一个DMA使能时才需要dummy变量 */
#if SPI_TX_USE_DMA
static uint8 spi_tx_dummy = 0xFF;   /* 仅接收时发送的哑数据 */
#endif
#if SPI_RX_USE_DMA
static uint8 spi_rx_dummy;          /* 仅发送时接收的哑数据 */
#endif
//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI 读写 1Byte（内部调用,只有当至少一个DMA禁用时才需要）
// 参数说明          spi_n        SPI模块号
// 参数说明          tx           发送字节
// 返回类型          uint8        接收字节
// 使用示例          uint8 rx = spi_rw_byte(SPI_1, 0xA5);
// 备注信息          阻塞等待TXE/RXNE，未加超时
//-------------------------------------------------------------------------------------------------------------------
#if (!SPI_TX_USE_DMA || !SPI_RX_USE_DMA)
static uint8 spi_rw_byte(spi_index_enum spi_n, uint8 tx)
{
		SPI_TypeDef *spix = (spi_n == SPI_1) ? SPI1 : SPI2;
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(spix, tx);
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_RXNE) == RESET);
    return (uint8)SPI_I2S_ReceiveData(spix);
}
#endif
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
/* 等待SPI空闲 */
#if SPI_TX_USE_DMA || SPI_RX_USE_DMA 
static void spi_wait_idle(spi_index_enum spi_n)
{
    SPI_TypeDef *spix = (spi_n == SPI_1) ? SPI1 : SPI2;
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_BSY) == SET);
}
#endif
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
	#if SPI_TX_USE_DMA
    spi_transfer_8bit(spi_n, &data, NULL, 1);
#else
    spi_rw_byte(spi_n, data);
#endif
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
#if SPI_TX_USE_DMA
    spi_transfer_8bit(spi_n, data, NULL, len);
#else
    while (len--) spi_write_8bit(spi_n, *data++);
#endif
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI写16位数据
// 参数说明          spi_n        SPI模块号
// 参数说明          data         待发送字
// 返回类型          void
// 使用示例          spi_write_16bit(SPI_1, 0x1234);
// 备注信息          
//-------------------------------------------------------------------------------------------------------------------
void spi_write_16bit(spi_index_enum spi_n, const uint16 data)
{
    spi_write_8bit(spi_n, (uint8)(data >> 8));//高
    spi_write_8bit(spi_n, (uint8)(data & 0xFF));// 低
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
#if SPI_RX_USE_DMA
    uint8 rx;
    spi_transfer_8bit(spi_n, NULL, &rx, 1);
    return rx;
#else
    return spi_rw_byte(spi_n, 0xFF);
#endif

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
	  if (len == 0) return;
#if SPI_RX_USE_DMA
    spi_transfer_8bit(spi_n, NULL, data, len);
#else
    while (len--) *data++ = spi_rw_byte(spi_n, 0xFF);
#endif	
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
    if (len == 0 || len > 65535) return;
#if SPI_TX_USE_DMA || SPI_RX_USE_DMA    
    SPI_TypeDef *spix = (spi_n == SPI_1) ? SPI1 : SPI2;
#endif    
    // 双缓冲为NULL检查
    if (write_buffer == NULL && read_buffer == NULL) return;
    
    // 根据DMA配置选择传输策略
#if SPI_TX_USE_DMA && SPI_RX_USE_DMA
    // ==================== 模式1: 全DMA模式====================
    {
        DMA_Channel_TypeDef *tx_dma = SPI_TX_DMA_CH(spi_n);
        DMA_Channel_TypeDef *rx_dma = SPI_RX_DMA_CH(spi_n);
        
        SPI_Cmd(spix, DISABLE);
        spi_wait_idle(spi_n);
        
        // 配置TX DMA源地址
        tx_dma->CMAR = (uint32)(write_buffer ? write_buffer : &spi_tx_dummy);
        // 配置RX DMA目标地址
        rx_dma->CMAR = (uint32)(read_buffer ? read_buffer : &spi_rx_dummy);
        
        // 必须先启动RX DMA，再启动TX DMA！
        dma_start(rx_dma, (uint16)len);
        dma_start(tx_dma, (uint16)len);
        
        SPI_Cmd(spix, ENABLE);
        
        // 等待完成
        spi_wait_idle(spi_n);
        dma_wait_done(rx_dma);
        dma_wait_done(tx_dma);
        
        DMA_Cmd(rx_dma, DISABLE);
        DMA_Cmd(tx_dma, DISABLE);
    }
#elif SPI_TX_USE_DMA && !SPI_RX_USE_DMA
    // ==================== 模式2: 仅TX用DMA，RX用轮询 ====================
    // 策略：DMA发送 + 中断/轮询接收，但为简化统一使用轮询接收
    {
        // 清空接收缓冲区
        while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_RXNE) == SET) {
            (void)SPI_I2S_ReceiveData(spix);
        }
        
        if (read_buffer == NULL) {
            // 只发送：使用DMA，忽略接收
            DMA_Channel_TypeDef *tx_dma = SPI_TX_DMA_CH(spi_n);
            
            SPI_Cmd(spix, DISABLE);
            spi_wait_idle(spi_n);
            
            tx_dma->CMAR = (uint32)write_buffer;
            dma_start(tx_dma, (uint16)len);
            
            SPI_Cmd(spix, ENABLE);
            
            // 轮询清空接收（防止溢出）
            for (uint32 i = 0; i < len; i++) {
                while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_RXNE) == RESET);
                (void)SPI_I2S_ReceiveData(spix);
            }
            
            spi_wait_idle(spi_n);
            dma_wait_done(tx_dma);
            DMA_Cmd(tx_dma, DISABLE);
        } else if (write_buffer == NULL) {
            // 只接收：使用轮询（RX不用DMA）
            for (uint32 i = 0; i < len; i++) {
                read_buffer[i] = spi_rw_byte(spi_n, 0xFF);
            }
        } else {
            // 全双工：DMA发送 + 轮询接收
            DMA_Channel_TypeDef *tx_dma = SPI_TX_DMA_CH(spi_n);
            
            SPI_Cmd(spix, DISABLE);
            spi_wait_idle(spi_n);
            
            // 清空接收缓冲区
            while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_RXNE) == SET) {
                (void)SPI_I2S_ReceiveData(spix);
            }
            
            tx_dma->CMAR = (uint32)write_buffer;
            dma_start(tx_dma, (uint16)len);
            
            SPI_Cmd(spix, ENABLE);
            
            // 轮询接收
            for (uint32 i = 0; i < len; i++) {
                while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_RXNE) == RESET);
                read_buffer[i] = (uint8)SPI_I2S_ReceiveData(spix);
            }
            
            spi_wait_idle(spi_n);
            dma_wait_done(tx_dma);
            DMA_Cmd(tx_dma, DISABLE);
        }
    }
#elif !SPI_TX_USE_DMA && SPI_RX_USE_DMA
    // ==================== 模式3: 仅RX用DMA，TX用轮询 ====================
    // 策略：轮询发送 + DMA接收
    {
        if (write_buffer == NULL && read_buffer != NULL) {
            // 只接收：轮询发送0xFF + DMA接收
            DMA_Channel_TypeDef *rx_dma = SPI_RX_DMA_CH(spi_n);
            
            SPI_Cmd(spix, DISABLE);
            spi_wait_idle(spi_n);
            
            rx_dma->CMAR = (uint32)read_buffer;
            
            // 预发送第一个字节启动传输
            SPI_I2S_SendData(spix, 0xFF);
            
            dma_start(rx_dma, (uint16)len);
            SPI_Cmd(spix, ENABLE);
            
            // 轮询发送剩余字节
            for (uint32 i = 1; i < len; i++) {
                while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_TXE) == RESET);
                SPI_I2S_SendData(spix, 0xFF);
            }
            
            spi_wait_idle(spi_n);
            dma_wait_done(rx_dma);
            DMA_Cmd(rx_dma, DISABLE);
        } else {
            // 其他情况（只发送或全双工）：统一使用轮询
            const uint8 *tx_ptr = write_buffer;
            uint8 *rx_ptr = read_buffer;
            
            for (uint32 i = 0; i < len; i++) {
                uint8 rx = spi_rw_byte(spi_n, tx_ptr ? tx_ptr[i] : 0xFF);
                if (rx_ptr) rx_ptr[i] = rx;
            }
        }
    }
#else
    // ==================== 模式4: 全轮询模式 ====================
    {
        const uint8 *tx_ptr = write_buffer;
        uint8 *rx_ptr = read_buffer;
        
        for (uint32 i = 0; i < len; i++) {
            uint8 tx_data = tx_ptr ? tx_ptr[i] : 0xFF;
            uint8 rx_data = spi_rw_byte(spi_n, tx_data);
            if (rx_ptr) rx_ptr[i] = rx_data;
        }
    }
#endif
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
    if (len == 0 || len > 32767) return;  // 最大65534字节
    
    // 使用临时缓冲区处理字节序转换
    uint8 temp_tx[64];  // 栈临时缓冲区，限制单次处理32个字（64字节）
    uint8 temp_rx[64];
    
    uint32 offset = 0;
    while (offset < len) {
        // 计算本次处理的字数（最多32个）
        uint32 chunk_len = (len - offset > 32) ? 32 : (len - offset);
        uint32 chunk_bytes = chunk_len * 2;
        
        // 准备发送数据：16位转字节（大端）
        if (write_buffer) {
            for (uint32 i = 0; i < chunk_len; i++) {
                uint16 data = write_buffer[offset + i];
                temp_tx[2*i] = (uint8)(data >> 8);      // 高字节
                temp_tx[2*i + 1] = (uint8)(data & 0xFF); // 低字节
            }
        }
        
        // 执行字节传输
        const uint8 *tx_ptr = write_buffer ? temp_tx : NULL;
        uint8 *rx_ptr = read_buffer ? temp_rx : NULL;
        
        spi_transfer_8bit(spi_n, tx_ptr, rx_ptr, chunk_bytes);
        
        // 处理接收数据：字节转16位（大端）
        if (read_buffer) {
            for (uint32 i = 0; i < chunk_len; i++) {
                read_buffer[offset + i] = ((uint16)temp_rx[2*i] << 8) | temp_rx[2*i + 1];
            }
        }
        
        offset += chunk_len;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介          SPI初始化
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
		SPI_TypeDef *spix = (spi_n == SPI_1) ? SPI1 : SPI2;
    GPIO_TypeDef *sck_port, *mosi_port, *miso_port, *cs_port;
    uint16 sck_pin_, mosi_pin_, miso_pin_, cs_pin_;

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
		
    /* 5. 初始化DMA */
#if SPI_TX_USE_DMA
{
        DMA_Channel_TypeDef *dma_ch = SPI_TX_DMA_CH(spi_n);
        
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
        DMA_Cmd(dma_ch, DISABLE);
        while(dma_ch->CCR & 1);/* 等待EN位清零 */
        
        DMA_InitTypeDef dmaInit;
        DMA_StructInit(&dmaInit);
        dmaInit.DMA_PeripheralBaseAddr = (uint32)&spix->DR;
        dmaInit.DMA_MemoryBaseAddr     = (uint32)&spi_tx_dummy;
        dmaInit.DMA_DIR                = DMA_DIR_PeripheralDST;
        dmaInit.DMA_BufferSize         = 1;
        dmaInit.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
        dmaInit.DMA_MemoryInc          = DMA_MemoryInc_Enable;
        dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        dmaInit.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
        dmaInit.DMA_Mode               = DMA_Mode_Normal;
        dmaInit.DMA_Priority           = DMA_Priority_Medium;
        dmaInit.DMA_M2M                = DMA_M2M_Disable;
        DMA_Init(dma_ch, &dmaInit);
        DMA_Cmd(dma_ch, DISABLE);
        
        SPI_I2S_DMACmd(spix, SPI_I2S_DMAReq_Tx, ENABLE);
}
#endif

#if SPI_RX_USE_DMA
{
        DMA_Channel_TypeDef *dma_ch = SPI_RX_DMA_CH(spi_n);
        
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
        DMA_Cmd(dma_ch, DISABLE);
        while(dma_ch->CCR & 1);/* 等待EN位清零 */
        
        DMA_InitTypeDef dmaInit;
        DMA_StructInit(&dmaInit);
        dmaInit.DMA_PeripheralBaseAddr = (uint32)&spix->DR;
        dmaInit.DMA_MemoryBaseAddr     = (uint32)&spi_rx_dummy;
        dmaInit.DMA_DIR                = DMA_DIR_PeripheralSRC;
        dmaInit.DMA_BufferSize         = 1;
        dmaInit.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
        dmaInit.DMA_MemoryInc          = DMA_MemoryInc_Enable;
        dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        dmaInit.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
        dmaInit.DMA_Mode               = DMA_Mode_Normal;
        dmaInit.DMA_Priority           = DMA_Priority_Medium;
        dmaInit.DMA_M2M                = DMA_M2M_Disable;
        DMA_Init(dma_ch, &dmaInit);
        DMA_Cmd(dma_ch, DISABLE);
        
        SPI_I2S_DMACmd(spix, SPI_I2S_DMAReq_Rx, ENABLE);
}
#endif
}

