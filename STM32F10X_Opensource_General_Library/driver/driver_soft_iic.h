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

#ifndef _driver_soft_iic_h_
#define _driver_soft_iic_h_

#include "common_headfile.h"


void        soft_iic_write_8bit             (soft_iic_info_struct *soft_iic_obj, const uint8 dat);
void        soft_iic_write_8bit_array       (soft_iic_info_struct *soft_iic_obj, const uint8 *dat, uint32 len);

void        soft_iic_write_16bit            (soft_iic_info_struct *soft_iic_obj, const uint16 dat);
void        soft_iic_write_16bit_array      (soft_iic_info_struct *soft_iic_obj, const uint16 *dat, uint32 len);

void        soft_iic_write_8bit_register    (soft_iic_info_struct *soft_iic_obj, const uint8 register_name, const uint8 dat);
void        soft_iic_write_8bit_registers   (soft_iic_info_struct *soft_iic_obj, const uint8 register_name, const uint8 *dat, uint32 len);

void        soft_iic_write_16bit_register   (soft_iic_info_struct *soft_iic_obj, const uint16 register_name, const uint16 dat);
void        soft_iic_write_16bit_registers  (soft_iic_info_struct *soft_iic_obj, const uint16 register_name, const uint16 *dat, uint32 len);

uint8       soft_iic_read_8bit              (soft_iic_info_struct *soft_iic_obj);
void        soft_iic_read_8bit_array        (soft_iic_info_struct *soft_iic_obj, uint8 *dat, uint32 len);

uint16      soft_iic_read_16bit             (soft_iic_info_struct *soft_iic_obj);
void        soft_iic_read_16bit_array       (soft_iic_info_struct *soft_iic_obj, uint16 *dat, uint32 len);

uint8       soft_iic_read_8bit_register     (soft_iic_info_struct *soft_iic_obj, const uint8 register_name);
void        soft_iic_read_8bit_registers    (soft_iic_info_struct *soft_iic_obj, const uint8 register_name, uint8 *dat, uint32 len);

uint16      soft_iic_read_16bit_register    (soft_iic_info_struct *soft_iic_obj, const uint16 register_name);
void        soft_iic_read_16bit_registers   (soft_iic_info_struct *soft_iic_obj, const uint16 register_name, uint16 *dat, uint32 len);

void        soft_iic_transfer_8bit_array    (soft_iic_info_struct *soft_iic_obj, const uint8 *write_data, uint32 write_len, uint8 *read_data, uint32 read_len);
void        soft_iic_transfer_16bit_array   (soft_iic_info_struct *soft_iic_obj, const uint16 *write_data, uint32 write_len, uint16 *read_data, uint32 read_len);

void        soft_iic_sccb_write_register    (soft_iic_info_struct *soft_iic_obj, const uint8 register_name, uint8 dat);
uint8       soft_iic_sccb_read_register     (soft_iic_info_struct *soft_iic_obj, const uint8 register_name);

void        soft_iic_init                   (soft_iic_info_struct *soft_iic_obj, uint8 addr, uint32 delay, gpio_pin_enum scl_pin, gpio_pin_enum sda_pin);

#endif

