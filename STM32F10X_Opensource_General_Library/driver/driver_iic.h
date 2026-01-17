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

#ifndef _driver_iic_h_
#define _driver_iic_h_

#include "driver_gpio.h"

void iic_write_8bit(iic_index_enum iic_n, const uint8 dat);
void iic_write_8bit_array(iic_index_enum iic_n, const uint8 *dat, uint32 len);
void iic_write_16bit(iic_index_enum iic_n, const uint16 dat);
void iic_write_16bit_array(iic_index_enum iic_n, const uint16 *dat, uint32 len);
void iic_write_8bit_register(iic_index_enum iic_n, const uint8 reg, const uint8 dat);
void iic_write_8bit_registers(iic_index_enum iic_n, const uint8 reg, const uint8 *dat, uint32 len);
void iic_write_16bit_register(iic_index_enum iic_n, const uint16 reg, const uint16 dat);
void iic_write_16bit_registers(iic_index_enum iic_n, const uint16 reg, const uint16 *dat, uint32 len);
uint8 iic_read_8bit(iic_index_enum iic_n);
void iic_read_8bit_array(iic_index_enum iic_n, uint8 *dat, uint32 len);
uint16 iic_read_16bit(iic_index_enum iic_n);
void iic_read_16bit_array(iic_index_enum iic_n, uint16 *dat, uint32 len);
uint8 iic_read_8bit_register(iic_index_enum iic_n, const uint8 reg);
void iic_read_8bit_registers(iic_index_enum iic_n, const uint8 reg, uint8 *dat, uint32 len);
uint16 iic_read_16bit_register(iic_index_enum iic_n, const uint16 reg);
void iic_read_16bit_registers(iic_index_enum iic_n, const uint16 reg, uint16 *dat, uint32 len);
void iic_transfer_8bit_array(iic_index_enum iic_n, const uint8 *wdat, uint32 wlen, uint8 *rdat, uint32 rlen);
void iic_transfer_16bit_array(iic_index_enum iic_n, const uint16 *wdat, uint32 wlen, uint16 *rdat, uint32 rlen);
void iic_sccb_write_register(iic_index_enum iic_n, const uint8 reg, uint8 dat);
uint8 iic_sccb_read_register(iic_index_enum iic_n, const uint8 reg);
void iic_init(iic_index_enum iic_n, uint8 addr, uint32 speed_khz, iic_pin_enum scl_pin, iic_pin_enum sda_pin);




#endif

