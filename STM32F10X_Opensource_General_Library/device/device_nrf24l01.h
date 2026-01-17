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

#ifndef _device_nrf24l01_h_
#define _device_nrf24l01_h_

#include "common_headfile.h"

// NRF24L01_USE_SOFT_SPI定义为0表示使用硬件SPI驱动 定义为1表示使用软件SPI驱动
// 当更改NRF24L01_USE_SOFT_SPI定义后，需要先编译并下载程序，单片机与模块需要断电重启才能正常通讯
#define NRF24L01_USE_SOFT_SPI         (0)                                       // 默认使用硬件 SPI 方式驱动	

#if NRF24L01_USE_SOFT_SPI                                                       // 这两段 颜色正常的才是正确的 颜色灰的就是没有用的
//====================================================软件 SPI 驱动====================================================
#define NRF24L01_SOFT_SPI_DELAY           (0 )                                    // 软件 SPI 的时钟延时周期 数值越小 SPI 通信速率越快	24 MHz 主频：2 ~ 4。	48 MHz 主频：4 ~ 8。	72 MHz 主频：8 ~ 15
#define NRF24L01_SCL_PIN                  (PB13)                                 // 软件 SPI SCK 引脚
#define NRF24L01_MOSI_PIN                 (PB15)                                 // 软件 SPI MOSI 引脚
#define NRF24L01_MISO_PIN                 (PB14)                                 // 软件 SPI MISO 引脚
//====================================================软件 SPI 驱动====================================================
#else

//====================================================硬件 SPI 驱动====================================================
#define NRF24L01_SPI_SPEED          (9 * 1000 * 1000)                          // 硬件 SPI 速率		spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）  
#define NRF24L01_SPI                (SPI_1)                                     // 硬件 SPI 号
#define NRF24L01_SPC_PIN            (SPI1_SCLK_PA5)                         // 硬件 SPI SCK 引脚
#define NRF24L01_SDI_PIN            (SPI1_MOSI_PA7)                          // 硬件 SPI MOSI 引脚
#define NRF24L01_SDO_PIN            (SPI1_MISO_PA6)                          // 硬件 SPI MISO 引脚
//====================================================硬件 SPI 驱动====================================================
#endif
#define NRF24L01_CS_PIN             (PA4)                                       // CS 片选引脚,低电平有效
#define NRF24L01_CS(x)              ((x) ? (gpio_high(NRF24L01_CS_PIN)) : (gpio_low(NRF24L01_CS_PIN)))
#define NRF24L01_CE_PIN             (PA3)                                       // CE 收发使能引脚
#define NRF24L01_CE(x)     					((x) ? (gpio_high(NRF24L01_CE_PIN)) : (gpio_low(NRF24L01_CE_PIN)))
#define NRF24L01_IRQ_PIN            (PA5)                                      // 必须接 IRQ，驱动里要读 

/* 24L01发送接收数据宽度定义 
 * 用户必须根据实际情况设置正确的数据宽度和数据长度
 * 发送端&接收端必须保持一致, 否则将导致通信失败!!!!
 */
#define NRF24L01_TX_ADR_WIDTH    		(5)       // 5字节的地址宽度 (可选范围3~5)
#define NRF24L01_RX_ADR_WIDTH    		(5)       // 5字节的地址宽度 (可选范围3~5)
#define NRF24L01_TX_PLOAD_WIDTH  		(32)      // 32字节的用户数据宽度 (可选范围1~32)
#define NRF24L01_RX_PLOAD_WIDTH  		(32)      // 32字节的用户数据宽度 (可选范围1~32)
#define NRF24L01_DATA_SPEED		   		(0x0F)    // 数据传输速率 (2 Mbps → 0x0F, 1 Mbps → 0x07, 250 kbps → 0x27(仅+版本支持))
#define NRF24L01_WORK_CHANNEL		   	(40)  		// NRF工作频道 (范围：0 – 125 对应 2.400 – 2.525 GHz）注意!!!2.400~2.4835GHz为全球ISM频段
//================================================定义 NRF24L01 内部地址================================================
/*NRF24L01指令宏定义*/
#define NRF24L01_R_REGISTER						(0x00)	//读寄存器，高3位为指令码，低5位为寄存器地址，后续跟1~5字节读数据
#define NRF24L01_W_REGISTER						(0x20)	//写寄存器，高3位为指令码，低5位为寄存器地址，后续跟1~5字节写数据
#define NRF24L01_R_RX_PAYLOAD					(0x61)	//读Rx有效载荷，后续跟1~32字节读数据
#define NRF24L01_W_TX_PAYLOAD					(0xA0)	//写Tx有效载荷，后续跟1~32字节写数据
#define NRF24L01_FLUSH_TX							(0xE1)	//清空Tx FIFO所有数据，单独指令
#define NRF24L01_FLUSH_RX							(0xE2)	//清空Rx FIFO所有数据，单独指令
#define NRF24L01_REUSE_TX_PL					(0xE3)//重新使用最后一次发送的有效载荷，单独指令
#define NRF24L01_R_RX_PL_WID					(0x60)//读取Rx FIFO最前面一个数据包的宽度，后续跟1字节读数据，仅适用于动态包长模式
#define NRF24L01_W_ACK_PAYLOAD				(0xA8)//写应答附带的有效载荷，高5位为指令码，低3位为通道号，后续跟1~32字节写数据，仅适用于应答附带载荷模式
#define NRF24L01_W_TX_PAYLOAD_NOACK		(0xB0)	//写Tx有效载荷，不要求应答，后续跟1~32字节写数据，仅适用于不要求应答模式
#define NRF24L01_NOP									(0xFF)	//空操作，单独指令，可以用读取状态寄存器

/*NRF24L01寄存器地址宏定义*/
#define NRF24L01_CONFIG								(0x00)	//配置寄存器，1字节
#define NRF24L01_EN_AA								(0x01)//使能自动应答，1字节
#define NRF24L01_EN_RXADDR						(0x02)	//使能接收通道，1字节
#define NRF24L01_SETUP_AW							(0x03)//设置地址宽度，1字节
#define NRF24L01_SETUP_RETR						(0x04)	//设置自动重传，1字节
#define NRF24L01_RF_CH								(0x05)	//射频通道，1字节
#define NRF24L01_RF_SETUP							(0x06)	//射频相关参数设置，1字节
#define NRF24L01_STATUS								(0x07)	//状态寄存器，1字节
#define NRF24L01_OBSERVE_TX						(0x08)	//发送观察寄存器，1字节
#define NRF24L01_RPD									(0x09)	//接收功率检测，1字节
#define NRF24L01_RX_ADDR_P0						(0x0A)	//接收通道0地址，5字节
#define NRF24L01_RX_ADDR_P1						(0x0B)	//接收通道1地址，5字节
#define NRF24L01_RX_ADDR_P2						(0x0C)	//接收通道2地址，1字节，高位地址与接收通道1相同
#define NRF24L01_RX_ADDR_P3						(0x0D)	//接收通道3地址，1字节，高位地址与接收通道1相同
#define NRF24L01_RX_ADDR_P4						(0x0E)	//接收通道4地址，1字节，高位地址与接收通道1相同
#define NRF24L01_RX_ADDR_P5						(0x0F)	//接收通道5地址，1字节，高位地址与接收通道1相同
#define NRF24L01_TX_ADDR							(0x10)	//发送地址，5字节
#define NRF24L01_RX_PW_P0							(0x11)	//接收通道0有效载荷数据宽度，1字节
#define NRF24L01_RX_PW_P1							(0x12)	//接收通道1有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P2							(0x13)	//接收通道2有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P3							(0x14)	//接收通道3有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P4							(0x15)	//接收通道4有效载荷的数据宽度，1字节
#define NRF24L01_RX_PW_P5							(0x16)	//接收通道5有效载荷的数据宽度，1字节
#define NRF24L01_FIFO_STATUS					(0x17)	//发送和接收FIFO状态，1字节
#define NRF24L01_DYNPD								(0x1C)	//使能接收通道的动态包长模式，1字节
#define NRF24L01_FEATURE							(0x1D)	//使能高级功能，1字节

#define NRF24L01_TX_MAX  		(0x10)  //达到最大发送次数中断
#define NRF24L01_TX_OK   		(0x20)  //TX发送完成中断
#define NRF24L01_RX_OK   		(0x40)  //接收到数据中断
//================================================定义 NRF24L01 内部地址================================================


void nrf24l01_rx_mode(void);//配置为接收模式
void nrf24l01_tx_mode(void);	//配置为发送模式
uint8 nrf24l01_txpacket(uint8 *txbuf);//发送一个包的数据
uint8 nrf24l01_rxpacket(uint8 *txbuf);//接收一个包的数据
uint8 nrf24l01_init(void);//初始化


#endif


