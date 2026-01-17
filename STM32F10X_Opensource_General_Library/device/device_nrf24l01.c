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
*                   SCL/SPC           查看 device_nrf24l01.h 中 NRF24L01_SPC_PIN 宏定义
*                   SDA/DSI           查看 device_nrf24l01.h 中 NRF24L01_SDI_PIN 宏定义
*                   SA0/SDO           查看 device_nrf24l01.h 中 NRF24L01_SDO_PIN 宏定义
*                   CS                查看 device_nrf24l01.h 中 NRF24L01_CS_PIN 宏定义
*                   CE                查看 device_nrf24l01.h 中 NRF24L01_CE_PIN 宏定义
*                   IRQ               查看 device_nrf24l01.h 中 NRF24L01_IRQ_PIN 宏定义
*                   VCC               3.3V电源
*                   GND               电源地
*                   其余引脚悬空
*
*                   // 软件 SPI 引脚
*                   同上
*                   ------------------------------------
********************************************************************************************************************/
#include "device_nrf24l01.h"

const uint8 nrf24l01_tx_address[NRF24L01_TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址
const uint8 nrf24l01_rx_address[NRF24L01_RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01};


#if NRF24L01_USE_SOFT_SPI
static soft_spi_info_struct nrf24l01_spi_struct;

#define nrf24l01_write_register(reg, data)        (soft_spi_write_8bit_register (&nrf24l01_spi_struct, (reg|NRF24L01_W_REGISTER), (data)))
#define nrf24l01_write_registers(reg, data, len)  (soft_spi_write_8bit_registers(&nrf24l01_spi_struct, (reg|NRF24L01_W_REGISTER), (data), (len)))
#define nrf24l01_read_register(reg)               (soft_spi_read_8bit_register  (&nrf24l01_spi_struct, (reg|NRF24L01_R_REGISTER)))
#define nrf24l01_read_registers(reg, data, len)   (soft_spi_read_8bit_registers (&nrf24l01_spi_struct, (reg|NRF24L01_R_REGISTER), (data), (len)))
#else
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     NRF24L01 写寄存器
// 参数说明     reg             寄存器地址
// 参数说明     data            数据
// 返回参数     void
// 使用示例     nrf24l01_write_register(NRF24L01_PWR_CONF, 0x00);                   // 关闭高级省电模式
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static void nrf24l01_write_register (uint8 reg, uint8 data)
{
   	NRF24L01_CS(0);                 //使能SPI传输
	  spi_write_8bit_register(NRF24L01_SPI, reg | NRF24L01_W_REGISTER, data);
  	NRF24L01_CS(1);                 //禁止SPI传输	   
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     NRF24L01 写数据
// 参数说明     reg             寄存器地址
// 参数说明     data            数据
// 返回参数     void
// 使用示例     nrf24l01_write_registers(NRF24L01_INIT_DATA, NRF24L01_config_file, sizeof(NRF24L01_config_file));
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static void nrf24l01_write_registers (uint8 reg, const uint8 *data, uint32 len)
{
    NRF24L01_CS(0);
    spi_write_8bit_registers(NRF24L01_SPI, reg | NRF24L01_W_REGISTER, data, len);
    NRF24L01_CS(1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     NRF24L01 读寄存器
// 参数说明     reg             寄存器地址
// 返回参数     uint8           数据
// 使用示例     nrf24l01_read_register(NRF24L01_CHIP_ID);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static uint8 nrf24l01_read_register (uint8 reg)
{
    uint8 data[2];
    NRF24L01_CS(0);
    spi_read_8bit_registers(NRF24L01_SPI, reg | NRF24L01_R_REGISTER, data, 2);
    NRF24L01_CS(1);
    return data[1];
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     NRF24L01 读数据
// 参数说明     reg             寄存器地址
// 参数说明     data            数据缓冲区
// 参数说明     len             数据长度
// 返回参数     void
// 使用示例     nrf24l01_read_registers(NRF24L01_ACC_ADDRESS, dat, 6);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static void nrf24l01_read_registers (uint8 reg, uint8 *data, uint32 len)
{
    uint8 temp_data[33];
    NRF24L01_CS(0);
    spi_read_8bit_registers(NRF24L01_SPI, reg | NRF24L01_R_REGISTER, temp_data, len + 1);
    NRF24L01_CS(1);
    for(int i = 0; i < len; i ++)
    {
        *(data ++) = temp_data[i + 1];
    }
}
#endif

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     NRF24L01 自检
// 参数说明     void
// 返回参数     uint8           1-自检失败 0-自检成功
// 使用示例     NRF24L01_self_check();
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static uint8 nrf24l01_self_check (void)
{
    const uint8 test_val[5]={0xA5,0xA5,0xA5,0xA5,0xA5};
    uint8 buf[5];
    nrf24l01_write_registers(NRF24L01_TX_ADDR, test_val, 5);
    nrf24l01_read_registers(NRF24L01_TX_ADDR, buf, 5);
    for(uint8 i=0;i<5;i++) if(buf[i]!=test_val[i]) return 1;
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     初始化 NRF24L01
// 参数说明     void
// 返回参数     uint8           1-初始化失败 0-初始化成功
// 使用示例     nrf24l01_init();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 nrf24l01_init (void)
{
    uint8 return_state = 0;
    system_delay_ms(20);                                                        // 等待设备上电成功

#if NRF24L01_USE_SOFT_SPI
    soft_spi_init(&nrf24l01_spi_struct, (uint8)0, (uint32)NRF24L01_SOFT_SPI_DELAY, (gpio_pin_enum)NRF24L01_SCL_PIN, (gpio_pin_enum)NRF24L01_MOSI_PIN, (gpio_pin_enum)NRF24L01_MISO_PIN, (gpio_pin_enum)SOFT_SPI_PIN_NULL);
#else
    spi_init(NRF24L01_SPI, SPI_MODE0, NRF24L01_SPI_SPEED, NRF24L01_SPC_PIN, NRF24L01_SDI_PIN, NRF24L01_SDO_PIN, SPI_CS_NULL);   // 配置 NRF24L01 的 SPI 端口
#endif
	  gpio_init(NRF24L01_CS_PIN, GPO_PUSH_PULL, 1);                  // 配置 NRF24L01 的CS端口
    gpio_init(NRF24L01_CE_PIN, GPO_PUSH_PULL, 0);                  // 使能 NRF24L01 
    gpio_init(NRF24L01_IRQ_PIN, GPI_PULL_UP,   1);
	do{
        if(nrf24l01_self_check())                                               // NRF24L01 自检
        {
            // 如果程序在输出了断言信息 并且提示出错位置在这里
            // 那么就是 NRF24L01 自检出错并超时退出了
            // 检查一下接线有没有问题 如果没问题可能就是坏了
            zf_log(0, "NRF24L01 self check error.");
            return_state = 1;
            break;
        }
				
    }while(0);
    return return_state;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数名称 : nrf24l01_txpacket
// 描  述   : 发送一包数据（32 字节），带超时保护
// 参  数   : txbuf – 待发送缓冲区
// 返  回   : NRF24L01_TX_OK  发送成功
//            NRF24L01_TX_MAX  达到最大重发
//           0xFF             其它异常
//-------------------------------------------------------------------------------------------------------------------
uint8 nrf24l01_txpacket(uint8 *txbuf)
{
    uint8 sta;

    NRF24L01_CE(0);
    nrf24l01_write_registers(NRF24L01_W_TX_PAYLOAD, txbuf, NRF24L01_TX_PLOAD_WIDTH);
    NRF24L01_CE(1);
    system_delay_us(15);                 /* >10 us */

    /* 等待 IRQ 变低（最长 10 ms）*/
    for(uint16 timeout=0;timeout<10000;timeout++)
    {
        if(gpio_get_level(NRF24L01_IRQ_PIN)==0) break;
        system_delay_us(1);
    }

    sta = nrf24l01_read_register(NRF24L01_STATUS);
    nrf24l01_write_register(NRF24L01_STATUS, sta);   /* 清中断 */

    if(sta & NRF24L01_TX_MAX)
    {
        nrf24l01_write_register(NRF24L01_FLUSH_TX, 0xff);
        return NRF24L01_TX_MAX;
    }
    if(sta & NRF24L01_TX_OK) return NRF24L01_TX_OK;
    return 0xff;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名称 : nrf24l01_rxpacket
//描  述   : 接收一包数据（非阻塞）
// 参  数   : rxbuf – 接收缓冲区（≥32 字节）
// 返  回   : 0 – 收到数据；1 – 无数据
//-------------------------------------------------------------------------------------------------------------------
uint8 nrf24l01_rxpacket(uint8 *rxbuf)
{
    uint8 sta = nrf24l01_read_register(NRF24L01_STATUS);
    if(sta & NRF24L01_RX_OK)
    {
        nrf24l01_read_registers(NRF24L01_R_RX_PAYLOAD, rxbuf, NRF24L01_RX_PLOAD_WIDTH);
        nrf24l01_write_register(NRF24L01_FLUSH_RX, 0xff);
    }
    nrf24l01_write_register(NRF24L01_STATUS, sta);
    return (sta & NRF24L01_RX_OK)?0:1;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名称 : nrf24l01_rx_mode
// 描  述   : 配置为接收模式
// 参  数   : void
// 返  回   : void
//-------------------------------------------------------------------------------------------------------------------
void nrf24l01_rx_mode(void)
{
    NRF24L01_CE(0);
    nrf24l01_write_registers(NRF24L01_RX_ADDR_P0, nrf24l01_rx_address, NRF24L01_RX_ADR_WIDTH);
    nrf24l01_write_register(NRF24L01_EN_AA,       0x01);
    nrf24l01_write_register(NRF24L01_EN_RXADDR,   0x01);
    nrf24l01_write_register(NRF24L01_RF_CH,       NRF24L01_WORK_CHANNEL);
    nrf24l01_write_register(NRF24L01_RX_PW_P0,    NRF24L01_RX_PLOAD_WIDTH);
    nrf24l01_write_register(NRF24L01_RF_SETUP,    NRF24L01_DATA_SPEED);
    nrf24l01_write_register(NRF24L01_CONFIG,      0x0f);   /* PWR_UP | EN_CRC | CRCO | PRIM_RX */
    NRF24L01_CE(1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名称 : nrf24l01_tx_mode
// 描  述   : 配置为发送模式
// 参  数   : void
// 返  回   : void
//-------------------------------------------------------------------------------------------------------------------
void nrf24l01_tx_mode(void)
{
    NRF24L01_CE(0);
    nrf24l01_write_registers(NRF24L01_TX_ADDR,     nrf24l01_tx_address, NRF24L01_TX_ADR_WIDTH);
    nrf24l01_write_registers(NRF24L01_RX_ADDR_P0,  nrf24l01_rx_address, NRF24L01_RX_ADR_WIDTH); /* ACK 地址 */
    nrf24l01_write_register(NRF24L01_EN_AA,       0x01);
    nrf24l01_write_register(NRF24L01_EN_RXADDR,   0x01);
    nrf24l01_write_register(NRF24L01_SETUP_RETR,  0x1a);   /* 500+86 μs, 10 次重发 */
    nrf24l01_write_register(NRF24L01_RF_CH,       NRF24L01_WORK_CHANNEL);
    nrf24l01_write_register(NRF24L01_RF_SETUP,    NRF24L01_DATA_SPEED);
    nrf24l01_write_register(NRF24L01_CONFIG,      0x0e);   /* PWR_UP | EN_CRC | CRCO */
    NRF24L01_CE(1);   /* 10 us 后启动发送 */
}


