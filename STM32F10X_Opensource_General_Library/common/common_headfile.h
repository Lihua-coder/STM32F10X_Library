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

#ifndef __common_headfile_h_
#define __common_headfile_h_
#include "stm32f10x.h"                  // Device header
//===================================================C语言 函数库===================================================
#include "math.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdarg.h"
//===================================================C语言 函数库===================================================

//====================================================公共层====================================================
#include "common_typedef.h"
#include "common_clock.h"
#include "common_debug.h"
#include "zf_common_fifo.h"
#include "zf_common_font.h"
#include "zf_common_function.h"
//====================================================公共层====================================================

//===================================================芯片外设驱动层===================================================
#include "driver_adc.h"
#include "driver_delay.h"
#include "driver_dma.h"
#include "driver_encoder.h"
#include "driver_exti.h"
#include "driver_flash.h"
#include "driver_gpio.h"
#include "driver_pit.h"
#include "driver_pwm.h"
#include "driver_pwmi.h"
#include "driver_soft_iic.h"
#include "driver_soft_spi.h"
#include "driver_iic.h"
#include "driver_spi.h"
#include "driver_uart.h"
#include "driver_can.h"
//===================================================芯片外设驱动层===================================================

//===================================================外接设备驱动层===================================================
#include "zf_device_oled.h"
#include "zf_device_tft180.h"
#include "zf_device_ips200.h"
#include "zf_device_mpu6050.h"
#include "zf_device_imu660ra.h"
#include "zf_device_imu963ra.h"
#include "zf_device_gnss.h"
#include "zf_device_wireless_uart.h"
#include "zf_device_type.h"
#include "zf_device_wifi_spi.h"
#include "zf_device_wifi_uart.h"
#include "zf_device_bluetooth_ch9141.h"
#include "device_nrf24l01.h"
#include "device_tb6612.h"
#include "device_w25q64.h"
//===================================================外接设备驱动层===================================================

//===================================================逐飞助手驱动层===================================================
#include "seekfree_assistant.h"
#include "seekfree_assistant_interface.h"
//===================================================逐飞助手驱动层===================================================

#endif


