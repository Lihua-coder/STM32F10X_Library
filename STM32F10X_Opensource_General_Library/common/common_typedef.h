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

#ifndef __common_typedef_h_
#define __common_typedef_h_

#include "common_headfile.h"

//数据类型声明
typedef unsigned char   uint8  ;	//  8 bits
typedef unsigned int  	uint16 ;	// 16 bits
typedef unsigned long  	uint32 ;	// 32 bits


typedef signed char     int8   ;	//  8 bits
typedef signed int      int16  ;	// 16 bits
typedef signed long     int32  ;	// 32 bits


typedef volatile int8   vint8  ;	//  8 bits
typedef volatile int16  vint16 ;	// 16 bits
typedef volatile int32  vint32 ;	// 32 bits


typedef volatile uint8  vuint8 ;	//  8 bits
typedef volatile uint16 vuint16;	// 16 bits
typedef volatile uint32 vuint32;	// 32 bits

typedef enum  // 枚举端口编号  
{
	PA=0,				
	PB,			
	PC,		
}gpio_port_enum;

typedef enum	//	GPIO 枚举
{
    PA0 = 0x00, PA1, PA2, PA3, PA4, PA5, PA6, PA7,
    PA8, PA9, PA10, PA11, PA12, PA13, PA14, PA15,
    PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7,
    PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15,
    PC13, PC14, PC15
} gpio_pin_enum;


typedef enum     // 枚举引脚模式   此枚举定义不允许用户修改   
{
	// 输入
	GPI_AIN = GPIO_Mode_AIN,                              //0x0,// 模拟输入
  GPI_FLOATING_IN = GPIO_Mode_IN_FLOATING,              //0x04,// 浮空输入模式
  GPI_PULL_DOWN = GPIO_Mode_IPD,                        //0x28, // 下拉输入
  GPI_PULL_UP = GPIO_Mode_IPU,                          //0x48,// 上拉输入
	// 输出
  GPO_OPEN_DTAIN = GPIO_Mode_Out_OD,                    //0x14,// 通用开漏输出模式
  GPO_PUSH_PULL = GPIO_Mode_Out_PP,                     //0x10,// 通用推挽输出模式
  GPO_AF_OD = GPIO_Mode_AF_OD,                          //0x1C,// 复用开漏输出模式
  GPO_AF_PP = GPIO_Mode_AF_PP,                          //0x18,// 复用推挽输出模式
	
}gpio_mode_enum;


typedef enum  // 枚举ERU通道
{
    // 一个通道只能选择其中一个引脚作为 外部中断的输入
    ERU_CH0_REQ0_PB0,
    ERU_CH1_REQ1_PB1,
    ERU_CH5_REQ9_5_PB5,
    ERU_CH6_REQ9_5_PB6,
    ERU_CH7_REQ9_5_PB7,
    ERU_CH8_REQ9_5_PB8,
    ERU_CH9_REQ9_5_PB9,
    ERU_CH10_REQ15_10_PB10,
    ERU_CH11_REQ15_10_PB11,
    ERU_CH12_REQ15_10_PB12,
    ERU_CH13_REQ15_10_PB13,
    ERU_CH14_REQ15_10_PB14,
    ERU_CH15_REQ15_10_PB15,
}exti_pin_enum;
//EXTI0~EXTI4：独立中断通道（如 EXTI0 对应 EXTI0_IRQn）；
//EXTI5~EXTI9：共享一个中断通道（EXTI9_5_IRQn）；
//EXTI10~EXTI15：共享一个中断通道（EXTI15_10_IRQn）。
/*
PB0  PB1  PB5	PB6	PB7	PB8	PB9	PB10  PB11
PB12	PB13	PB14	PB15
*/


typedef enum           // 枚举 TIM通道
{
    TIM2_PIT,
    TIM3_PIT,
    TIM4_PIT,
}pit_index_enum;

//------------------------------------------------------------------------------------------------
// 此枚举定义不允许用户修改
//=====注意：请慎重使用ATOM2_CH1_PA15，ATOM2_CH2_PB3，ATOM3_CH1_PB4=============//
//==================对于STM32F10X而言，这些引脚为JTAG调试端口===================//
//=====注意：请慎重使用ATOM2_CH1_PA15，ATOM2_CH2_PB3，ATOM3_CH1_PB4=============//
typedef enum // 枚举PWM引脚      按列使用
{
    ATOM1_CH2_PA9, 
    ATOM1_CH3_PA10,
    ATOM1_CH4_PA11,
	
		ATOM2_CH1_PA0,  ATOM2_CH1_PA15,//完全重映射   *
    ATOM2_CH2_PA1,  ATOM2_CH2_PB3,//   *
    ATOM2_CH3_PA2,  ATOM2_CH3_PB10,
    ATOM2_CH4_PA3,  ATOM2_CH4_PB11,
	
    ATOM3_CH1_PA6,  ATOM3_CH1_PB4,//部分重映射   *
    ATOM3_CH2_PA7,  ATOM3_CH2_PB5,
    ATOM3_CH3_PB0, 
    ATOM3_CH4_PB1, 
	
    ATOM4_CH1_PB6, 
    ATOM4_CH2_PB7, 
    ATOM4_CH3_PB8,
    ATOM4_CH4_PB9,
}pwm_channel_enum;
//=====注意：请慎重使用ATOM2_CH1_PA15，ATOM2_CH2_PB3，ATOM3_CH1_PB4=============//
//================对于STM32F10X而言，这些引脚为JTAG调试端口=====================//
//=====注意：请慎重使用ATOM2_CH1_PA15，ATOM2_CH2_PB3，ATOM3_CH1_PB4=============//
//-----------------------------------------------------------------------------------------------------

typedef enum  // 枚举 编码器定时器编号
{
    TIM2_ENCODER,
    TIM3_ENCODER,
    TIM4_ENCODER,
}encoder_index_enum;

//-------------------------------------------------------------------------------------------------------
// 此枚举定义不允许用户修改       按行使用
//=====注意：请慎重使用TIM2_ENCODER_CH1_PA15，TIM2_ENCODER_CH2_PB3，TIM3_ENCODER_CH1_PB4======//
//========================对于STM32F10X而言，这些引脚为JTAG调试端口===========================//
//=====注意：请慎重使用TIM2_ENCODER_CH1_PA15，TIM2_ENCODER_CH2_PB3，TIM3_ENCODER_CH1_PB4======//
typedef enum // 枚举编码器引脚
{
    TIM2_ENCODER_CH1_PA0,      // T2定时器 计数引脚可选范围
    TIM2_ENCODER_CH1_PA15,		 // T2定时器 完全重映射   *

    TIM3_ENCODER_CH1_PA6,      // T3定时器 计数引脚可选范围
    TIM3_ENCODER_CH1_PB4,      // T3定时器 部分重映射   *

  	TIM4_ENCODER_CH1_PB6,      // T4定时器 计数引脚可选范围
}encoder_channel1_enum;
typedef enum // 枚举编码器引脚
{
    TIM2_ENCODER_CH2_PA1,     // T2定时器 计数方向引脚可选范围
    TIM2_ENCODER_CH2_PB3,     // T2定时器 完全重映射   *

    TIM3_ENCODER_CH2_PA7,     // T3定时器 计数方向引脚可选范围
    TIM3_ENCODER_CH2_PB5,     // T3定时器 部分重映射

		TIM4_ENCODER_CH2_PB7,     // T4定时器 计数方向引脚可选范围
}encoder_channel2_enum;
//=====注意：请慎重使用TIM2_ENCODER_CH1_PA15，TIM2_ENCODER_CH2_PB3，TIM3_ENCODER_CH1_PB4======//
//=====================对于STM32F10X而言，这些引脚为JTAG调试端口==============================//
//=====注意：请慎重使用TIM2_ENCODER_CH1_PA15，TIM2_ENCODER_CH2_PB3，TIM3_ENCODER_CH1_PB4======//
//-----------------------------------------------------------------------------------------------------------

typedef enum    // 枚举ADC通道
{
    // ADC1可选引脚
    ADC1_CH0_PA0,
    ADC1_CH1_PA1,
   	ADC1_CH2_PA2,
    ADC1_CH3_PA3,
    ADC1_CH4_PA4,
    ADC1_CH5_PA5,
    ADC1_CH6_PA6,
    ADC1_CH7_PA7,
    ADC1_CH8_PB0,
    ADC1_CH9_PB1,
}adc1_channel_enum;
// 此枚举定义不允许用户修改
typedef enum        // 枚举ADC通道
{
    ADC_8BIT,       // 8位分辨率
    ADC_12BIT,      // 12位分辨率  
}adc_resolution_enum;
typedef enum    // 枚举DMA通道
{
    // DMA1可选通道
    dma1_CH1,//ADC
   	dma1_CH2,//USART3 TX
    dma1_CH3,
    dma1_CH4,//USART1 TX
    dma1_CH5,
    dma1_CH6,
    dma1_CH7,//USART2 TX
}dma_channel_enum;
#define DMA_Byte           DMA_PeripheralDataSize_Byte       // ((uint32_t)0x00000000)
#define DMA_HalfWord       DMA_PeripheralDataSize_HalfWord   // ((uint32_t)0x00000100)
#define DMA_Word           DMA_PeripheralDataSize_Word        //((uint32_t)0x00000200)


//按行使用
typedef enum            // 枚举串口引脚 此枚举定义不允许用户修改
{
    UART1_TX_PA9,     // 串口1 发送引脚可选范围
    UART1_TX_PB6,     // 重映射

    UART2_TX_PA2,     // 串口2 发送引脚可选范围

    UART3_TX_PB10,     // 串口3 发送引脚可选范围
}uart_tx_pin_enum;
typedef enum            // 枚举串口引脚 此枚举定义不允许用户修改
{  
		UART1_RX_PA10, // 串口1 接收引脚可选范围
    UART1_RX_PB7,    //重映射

    UART2_RX_PA3,     // 串口2 接收引脚可选范围

    UART3_RX_PB11,     // 串口3 接收引脚可选范围
}uart_rx_pin_enum;
typedef enum            // 枚举串口号 此枚举定义不允许用户修改
{
    UART_1,
    UART_2,
    UART_3,
}uart_index_enum;


/* 软件 IIC 信息结构体 */
typedef struct
{
    gpio_pin_enum       scl_pin;                                                // 用于记录对应的引脚编号
    gpio_pin_enum       sda_pin;                                                // 用于记录对应的引脚编号
    uint8               addr;                                                   // 器件地址 七位地址模式
    uint32              delay;                                                  // 模拟 IIC 软延时时长
}soft_iic_info_struct;
// 该枚举体禁止用户修改
typedef enum
{
    IIC_1,			// 与UART1，共用一组寄存器。使用时，只能二选一
    IIC_2,			// 与UART2，共用一组寄存器。使用时，只能二选一
    //其中SS引脚由软件控制
} iic_index_enum;
// 该枚举体禁止用户修改
typedef enum
{
    IIC1_SCL_PB6,
    IIC1_SDA_PB7,
	
    IIC1_SCL_PB8,//重映射
    IIC1_SDA_PB9,
	
	// IIC2与串口3复用，要么使用串口3，要么使用IIC2
    IIC2_SCL_PB10,
    IIC2_SDA_PB11,
} iic_pin_enum;


/* 软件 SPI 信息结构体 */
typedef struct //枚举模块号
{
    uint8       mode        :6;                                         // SPI 模式
    uint8       use_miso    :1;                                         // 是否使用 MISO 引脚
    uint8       use_cs      :1;                                         // 是否使用 CS 引脚
}spi_config_info_struct;
typedef struct
{
    spi_config_info_struct  config;                                     // 配置整体数据
    gpio_pin_enum           sck_pin;                                    // 用于记录对应的引脚编号
    gpio_pin_enum           mosi_pin;                                   // 用于记录对应的引脚编号
    gpio_pin_enum           miso_pin;                                   // 用于记录对应的引脚编号
    gpio_pin_enum           cs_pin;                                     // 用于记录对应的引脚编号
    uint32                  delay;                                      // 模拟 SPI 软延时时长
}soft_spi_info_struct;

/* 硬件 SPI 信息结构体 */
//-----------------------------------------------------------------------------------------------------------
//===============================注意：请慎重使用SPI1重映射引脚===============================//
//=====================对于STM32F10X而言，这些引脚为JTAG调试端口==============================//
//===============================注意：请慎重使用SPI1重映射引脚===============================//
typedef enum        // SPI模块号
{
    SPI_1 = 1,
    SPI_2,
}spi_index_enum;
typedef enum        // 枚举 SPI 模式 此枚举定义不允许用户修改
{
    SPI_MODE0,
    SPI_MODE1,
    SPI_MODE2,
    SPI_MODE3,
}spi_mode_enum;
typedef enum  // 枚举SPI CS引脚 此枚举定义不允许用户修改
{

    SPI1_CS_PA4   = 1*102+0*6 , SPI1_CS_PA15,//重映射*

    SPI2_CS_PB12   = 2*102+0*6 ,

    SPI_CS_NULL,
}spi_cs_pin_enum;
typedef enum                                                                                                                // 枚举SPI CLK引脚 此枚举定义不允许用户修改
{
    SPI1_SCLK_PA5  = 1*102+1*6 , SPI1_SCLK_PB3,//重映射*                                                                         // SPI1 CLK 引脚可选范围

    SPI2_SCLK_PB13  = 2*102+1*6 ,   // SPI2 CLK 引脚可选范围
                                 
}spi_sck_pin_enum;
typedef enum                                                                                                                // 枚举SPI MISO引脚 此枚举定义不允许用户修改
{

    SPI1_MISO_PA6  = 1*102+2*6 , SPI1_MISO_PB4,//重映射*                                                                         // SPI1 MISO引脚可选范围

    SPI2_MISO_PB14  = 2*102+2*6 ,                    // SPI2 MISO引脚可选范围

}spi_miso_pin_enum;
typedef enum                                                                                                                // 枚举SPI MOSI引脚 此枚举定义不允许用户修改
{
    SPI1_MOSI_PA7  = 1*102+3*6 , SPI1_MOSI_PB5,//重映射*                                                      // SPI1 MOSI引脚可选范围

    SPI2_MOSI_PB15  = 2*102+3*6 , 
	
}spi_mosi_pin_enum;
//===============================注意：请慎重使用SPI1重映射引脚===============================//
//=====================对于STM32F10X而言，这些引脚为JTAG调试端口==============================//
//===============================注意：请慎重使用SPI1重映射引脚===============================//
//-----------------------------------------------------------------------------------------------------------

typedef enum
{
    CAN_1,	
} can_index_enum;
//按行使用
typedef enum            // 枚举CAN引脚 此枚举定义不允许用户修改
{
    CAN1_TX_PA12,     // CAN1 发送引脚可选范围
    CAN1_TX_PB9,     // 重映射
}can_tx_pin_enum;
typedef enum            // 枚举CAN引脚 此枚举定义不允许用户修改
{  
		CAN1_RX_PA11, // CAN1 接收引脚可选范围
    CAN1_RX_PB8,    //重映射
}can_rx_pin_enum;








#define ZF_WEAK         __attribute__((weak))


#endif
