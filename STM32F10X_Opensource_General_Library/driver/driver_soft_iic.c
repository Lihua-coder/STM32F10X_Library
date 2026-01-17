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

#include "driver_soft_iic.h"

//#pragma warning disable = 183
//#pragma warning disable = 177


#define SOFT_IIC_SDA_IO_SWITCH          (0)                                     // 是否需要 SDA 进行 I/O 切换 0-不需要 1-需要

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 延时
// 参数说明     delay           延时次数
// 返回参数     void
// 使用示例     soft_iic_delay(1);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_delay (vuint32 delay)
{
    volatile uint32 count = delay;
    while(count --);
}
//#define soft_iic_delay(x)  for(uint32 i = x; i--; )

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC START 信号
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 返回参数     void
// 使用示例     soft_iic_start(soft_iic_obj);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_start (soft_iic_info_struct *soft_iic_obj)
{
    zf_assert(soft_iic_obj != NULL);
    gpio_high(soft_iic_obj->scl_pin);                                           // SCL 高电平
    gpio_high(soft_iic_obj->sda_pin);                                           // SDA 高电平

    soft_iic_delay(soft_iic_obj->delay);
    gpio_low(soft_iic_obj->sda_pin);                                            // SDA 先拉低
    soft_iic_delay(soft_iic_obj->delay);
    gpio_low(soft_iic_obj->scl_pin);                                            // SCL 再拉低
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC STOP 信号
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 返回参数     void
// 使用示例     soft_iic_stop(soft_iic_obj);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_stop (soft_iic_info_struct *soft_iic_obj)
{
    zf_assert(soft_iic_obj != NULL);
    gpio_low(soft_iic_obj->sda_pin);                                            // SDA 低电平
    gpio_low(soft_iic_obj->scl_pin);                                            // SCL 低电平

    soft_iic_delay(soft_iic_obj->delay);
    gpio_high(soft_iic_obj->scl_pin);                                           // SCL 先拉高
    soft_iic_delay(soft_iic_obj->delay);
    gpio_high(soft_iic_obj->sda_pin);                                           // SDA 再拉高
    soft_iic_delay(soft_iic_obj->delay);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 发送 ACK/NAKC 信号 内部调用
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     ack             ACK 电平
// 返回参数     void
// 使用示例     soft_iic_send_ack(soft_iic_obj, 1);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_send_ack (soft_iic_info_struct *soft_iic_obj, uint8 ack)
{
    zf_assert(soft_iic_obj != NULL);
    gpio_low(soft_iic_obj->scl_pin);                                            // SCL 低电平

    if(ack)
    {
        gpio_high(soft_iic_obj->sda_pin);                                       // SDA 拉高
    }
    else
    {
        gpio_low(soft_iic_obj->sda_pin);                                        // SDA 拉低
    }

    soft_iic_delay(soft_iic_obj->delay);
    gpio_high(soft_iic_obj->scl_pin);                                           // SCL 拉高
    soft_iic_delay(soft_iic_obj->delay);
    gpio_low(soft_iic_obj->scl_pin);                                            // SCL 拉低
    gpio_high(soft_iic_obj->sda_pin);                                           // SDA 拉高
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 获取 ACK/NAKC 信号
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 返回参数     uint8           ACK 状态
// 使用示例     soft_iic_wait_ack(soft_iic_obj);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
uint8 soft_iic_wait_ack (soft_iic_info_struct *soft_iic_obj)
{
	uint8 temp = 0;
    zf_assert(soft_iic_obj != NULL);
  
    gpio_low(soft_iic_obj->scl_pin);                                            // SCL 低电平
    gpio_high(soft_iic_obj->sda_pin);                                           // SDA 高电平 释放 SDA
#if SOFT_IIC_SDA_IO_SWITCH
    gpio_set_dir(soft_iic_obj->sda_pin, GPI, GPI_FLOATING_IN);
#endif
    soft_iic_delay(soft_iic_obj->delay);

    gpio_high(soft_iic_obj->scl_pin);                                           // SCL 高电平
    soft_iic_delay(soft_iic_obj->delay);

    if(gpio_get_level((gpio_pin_enum)soft_iic_obj->sda_pin))
    {
        temp = 1;
    }
    gpio_low(soft_iic_obj->scl_pin);                                            // SCL 低电平
#if SOFT_IIC_SDA_IO_SWITCH
    gpio_set_dir(soft_iic_obj->sda_pin, GPO, GPO_OPEN_DTAIN);
#endif
    soft_iic_delay(soft_iic_obj->delay);

    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 发送 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     dat            数据
// 返回参数     uint8           ACK 状态
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
uint8 soft_iic_send_data (soft_iic_info_struct *soft_iic_obj, const uint8 dat)
{
	uint8 temp = 0x80;
	
   zf_assert(soft_iic_obj != NULL);

    while(temp)
    {
        ((dat & temp) ? (gpio_high(soft_iic_obj->sda_pin)) : (gpio_low(soft_iic_obj->sda_pin)));
        temp >>= 1;

        soft_iic_delay(soft_iic_obj->delay);
        gpio_high(soft_iic_obj->scl_pin);                                       // SCL 拉高
        soft_iic_delay(soft_iic_obj->delay);
        gpio_low(soft_iic_obj->scl_pin);                                        // SCL 拉低
    }
    return ((soft_iic_wait_ack(soft_iic_obj) == 1) ? 0 : 1 );
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 读取 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     ack             ACK 或 NACK
// 返回参数     uint8           数据
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
uint8 soft_iic_read_data (soft_iic_info_struct *soft_iic_obj, uint8 ack)
{
    uint8 dat = 0x00;
    uint8 temp = 8;
		zf_assert(soft_iic_obj != NULL);
    gpio_low(soft_iic_obj->scl_pin);                                            // SCL 低电平
    soft_iic_delay(soft_iic_obj->delay);
    gpio_high(soft_iic_obj->sda_pin);                                           // SDA 高电平 释放 SDA
#if SOFT_IIC_SDA_IO_SWITCH
    gpio_set_dir(soft_iic_obj->sda_pin, GPI, GPI_FLOATING_IN);
#endif

    while(temp --)
    {
        gpio_low(soft_iic_obj->scl_pin);                                        // SCL 拉低
        soft_iic_delay(soft_iic_obj->delay);
        gpio_high(soft_iic_obj->scl_pin);                                       // SCL 拉高
        soft_iic_delay(soft_iic_obj->delay);
        dat = ((dat << 1) | gpio_get_level((gpio_pin_enum)soft_iic_obj->sda_pin));
    }
    gpio_low(soft_iic_obj->scl_pin);                                            // SCL 低电平
#if SOFT_IIC_SDA_IO_SWITCH
    gpio_set_dir(soft_iic_obj->sda_pin, GPO, GPO_OPEN_DTAIN);
#endif
    soft_iic_delay(soft_iic_obj->delay);
    soft_iic_send_ack(soft_iic_obj, ack);
    return dat;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口写 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     soft_iic_write_8bit_register(soft_iic_obj, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_8bit (soft_iic_info_struct *soft_iic_obj, const uint8 dat)
{
    zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, dat);
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口写 8bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     *dat           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void            
// 使用示例     soft_iic_write_8bit_array(soft_iic_obj, dat, 6);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_8bit_array (soft_iic_info_struct *soft_iic_obj, const uint8 *dat, uint32 len)
{
    zf_assert(soft_iic_obj != NULL);
    zf_assert(dat != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    while(len --)
    {
        soft_iic_send_data(soft_iic_obj, *dat ++);
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口器写 16bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     soft_iic_write_16bit(soft_iic_obj, 0x0101);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_16bit (soft_iic_info_struct *soft_iic_obj, const uint16 dat)
{
    zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8)((dat & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8)(dat & 0x00FF));
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口写 16bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     *dat           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void            
// 使用示例     soft_iic_write_16bit_array(soft_iic_obj, dat, 6);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_16bit_array (soft_iic_info_struct *soft_iic_obj, const uint16 *dat, uint32 len)
{
    zf_assert(soft_iic_obj != NULL);
    zf_assert(dat != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    while(len --)
    {
        soft_iic_send_data(soft_iic_obj, (uint8)((*dat & 0xFF00) >> 8));
        soft_iic_send_data(soft_iic_obj, (uint8)(*dat ++ & 0x00FF));
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口向传感器寄存器写 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     soft_iic_write_8bit_register(soft_iic_obj, 0x01, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_8bit_register (soft_iic_info_struct *soft_iic_obj, const uint8 register_name, const uint8 dat)
{
    zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_send_data(soft_iic_obj, dat);
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口向传感器寄存器写 8bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void            
// 使用示例     soft_iic_write_8bit_registers(soft_iic_obj, 0x01, dat, 6);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_8bit_registers (soft_iic_info_struct *soft_iic_obj, const uint8 register_name, const uint8 *dat, uint32 len)
{
    zf_assert(soft_iic_obj != NULL);
    zf_assert(dat != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    while(len --)
    {
        soft_iic_send_data(soft_iic_obj, *dat ++);
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口向传感器寄存器写 16bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     soft_iic_write_16bit_register(soft_iic_obj, 0x0101, 0x0101);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_16bit_register (soft_iic_info_struct *soft_iic_obj, const uint16 register_name, const uint16 dat)
{
    zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8)((register_name & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8)(register_name & 0x00FF));
    soft_iic_send_data(soft_iic_obj, (uint8)((dat & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8)(dat & 0x00FF));
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口向传感器寄存器写 16bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void            
// 使用示例     soft_iic_write_16bit_registers(soft_iic_obj, 0x0101, dat, 6);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_write_16bit_registers (soft_iic_info_struct *soft_iic_obj, const uint16 register_name, const uint16 *dat, uint32 len)
{
    zf_assert(soft_iic_obj != NULL);
    zf_assert(dat != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8)((register_name & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8)(register_name & 0x00FF));
    while(len--)
    {
        soft_iic_send_data(soft_iic_obj, (uint8)((*dat & 0xFF00) >> 8));
        soft_iic_send_data(soft_iic_obj, (uint8)(*dat ++ & 0x00FF));
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口读取 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 返回参数     uint8           返回读取的 8bit 数据
// 使用示例     soft_iic_read_8bit(soft_iic_obj);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 soft_iic_read_8bit (soft_iic_info_struct *soft_iic_obj)
{
    uint8 temp = 0;
		zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 1);
    soft_iic_stop(soft_iic_obj);
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 8bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void            
// 使用示例     soft_iic_read_8bit_array(soft_iic_obj, dat, 8);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_read_8bit_array (soft_iic_info_struct *soft_iic_obj, uint8 *dat, uint32 len)
{
    zf_assert(soft_iic_obj != NULL);
    zf_assert(dat != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    while(len --)
    {
        *dat ++ = soft_iic_read_data(soft_iic_obj, len == 0);
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口读取 16bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint16          返回读取的 16bit 数据
// 使用示例     soft_iic_read_16bit(soft_iic_obj);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint16 soft_iic_read_16bit (soft_iic_info_struct *soft_iic_obj)
{
    uint16 temp = 0;
		zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 0);
    temp = ((temp << 8)| soft_iic_read_data(soft_iic_obj, 1));
    soft_iic_stop(soft_iic_obj);
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口读取 16bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 可以参照 
// 参数说明     *dat           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void            
// 使用示例     soft_iic_read_16bit_array(soft_iic_obj, dat, 8);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_read_16bit_array (soft_iic_info_struct *soft_iic_obj, uint16 *dat, uint32 len)
{
		zf_assert(soft_iic_obj != NULL);
		zf_assert(dat != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    while(len --)
    {
        *dat = soft_iic_read_data(soft_iic_obj, 0);
        *dat = ((*dat << 8)| soft_iic_read_data(soft_iic_obj, len == 0));
        dat ++;
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint8           返回读取的 8bit 数据
// 使用示例     soft_iic_read_8bit_register(soft_iic_obj, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 soft_iic_read_8bit_register (soft_iic_info_struct *soft_iic_obj, const uint8 register_name)
{
    uint8 temp = 0;
		zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 1);
    soft_iic_stop(soft_iic_obj);
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 8bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void            
// 使用示例     soft_iic_read_8bit_registers(soft_iic_obj, 0x01, dat, 8);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_read_8bit_registers (soft_iic_info_struct *soft_iic_obj, const uint8 register_name, uint8 *dat, uint32 len)
{
    zf_assert(soft_iic_obj != NULL);
    zf_assert(dat != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    while(len --)
    {
        *dat ++ = soft_iic_read_data(soft_iic_obj, len == 0);
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 16bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint16          返回读取的 16bit 数据
// 使用示例     soft_iic_read_16bit_register(soft_iic_obj, 0x0101);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint16 soft_iic_read_16bit_register (soft_iic_info_struct *soft_iic_obj, const uint16 register_name)
{
    uint16 temp = 0;
		zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8)((register_name & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8)(register_name & 0x00FF));
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 0);
    temp = ((temp << 8)| soft_iic_read_data(soft_iic_obj, 1));
    soft_iic_stop(soft_iic_obj);
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口从传感器寄存器读取 16bit 数组
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void            
// 使用示例     soft_iic_read_16bit_registers(soft_iic_obj, 0x0101, dat, 8);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_read_16bit_registers (soft_iic_info_struct *soft_iic_obj, const uint16 register_name, uint16 *dat, uint32 len)
{
    zf_assert(soft_iic_obj != NULL);
    zf_assert(dat != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, (uint8)((register_name & 0xFF00) >> 8));
    soft_iic_send_data(soft_iic_obj, (uint8)(register_name & 0x00FF));
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    while(len --)
    {
        *dat = soft_iic_read_data(soft_iic_obj, 0);
        *dat = ((*dat << 8)| soft_iic_read_data(soft_iic_obj, len == 0));
        dat ++;
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口传输 8bit 数组 先写后读取
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     *write_data     发送数据存放缓冲区
// 参数说明     write_len       发送缓冲区长度
// 参数说明     *read_data      读取数据存放缓冲区
// 参数说明     read_len        读取缓冲区长度
// 返回参数     void            
// 使用示例     iic_transfer_8bit_array(IIC_1, addr, dat, 64, dat, 64);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_transfer_8bit_array (soft_iic_info_struct *soft_iic_obj, const uint8 *write_data, uint32 write_len, uint8 *read_data, uint32 read_len)
{
    zf_assert(soft_iic_obj != NULL);
    zf_assert(write_data != NULL);
    zf_assert(read_data != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    while(write_len --)
    {
        soft_iic_send_data(soft_iic_obj, *write_data ++);
    }

    if(read_len)
    {
        soft_iic_start(soft_iic_obj);
        soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
        while(read_len --)
        {
            *read_data ++ = soft_iic_read_data(soft_iic_obj, read_len == 0);
        }
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口传输 16bit 数组 先写后读取
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     *write_data     发送数据存放缓冲区
// 参数说明     write_len       发送缓冲区长度
// 参数说明     *read_data      读取数据存放缓冲区
// 参数说明     read_len        读取缓冲区长度
// 返回参数     void            
// 使用示例     iic_transfer_16bit_array(IIC_1, addr, dat, 64, dat, 64);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_transfer_16bit_array (soft_iic_info_struct *soft_iic_obj, const uint16 *write_data, uint32 write_len, uint16 *read_data, uint32 read_len)
{
		zf_assert(soft_iic_obj != NULL);
		zf_assert(write_data != NULL);
		zf_assert(read_data != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    while(write_len--)
    {
        soft_iic_send_data(soft_iic_obj, (uint8)((*write_data & 0xFF00) >> 8));
        soft_iic_send_data(soft_iic_obj, (uint8)(*write_data ++ & 0x00FF));
    }
    if(read_len)
    {
        soft_iic_start(soft_iic_obj);
        soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
        while(read_len --)
        {
            *read_data = soft_iic_read_data(soft_iic_obj, 0);
            *read_data = ((*read_data << 8)| soft_iic_read_data(soft_iic_obj, read_len == 0));
            read_data ++;
        }
    }
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口 SCCB 模式向传感器寄存器写 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     soft_iic_sccb_write_register(soft_iic_obj, 0x01, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_sccb_write_register (soft_iic_info_struct *soft_iic_obj, const uint8 register_name, uint8 dat)
{
    zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_send_data(soft_iic_obj, dat);
    soft_iic_stop(soft_iic_obj);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口 SCCB 模式从传感器寄存器读取 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint8           返回读取的 8bit 数据
// 使用示例     soft_iic_sccb_read_register(soft_iic_obj, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 soft_iic_sccb_read_register (soft_iic_info_struct *soft_iic_obj, const uint8 register_name)
{
    uint8 temp = 0;
		zf_assert(soft_iic_obj != NULL);
    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1);
    soft_iic_send_data(soft_iic_obj, register_name);
    soft_iic_stop(soft_iic_obj);

    soft_iic_start(soft_iic_obj);
    soft_iic_send_data(soft_iic_obj, soft_iic_obj->addr << 1 | 0x01);
    temp = soft_iic_read_data(soft_iic_obj, 1);
    soft_iic_stop(soft_iic_obj);
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     软件 IIC 接口初始化 默认 MASTER 模式 不提供 SLAVE 模式
// 参数说明     *soft_iic_obj   软件 IIC 指定信息存放结构体的指针
// 参数说明     addr            软件 IIC 地址 这里需要注意 标准七位地址 最高位忽略 写入时请务必确认无误
// 参数说明     delay           软件 IIC 延时 就是时钟高电平时间 越短 IIC 速率越高
// 参数说明     scl_pin         软件 IIC 时钟引脚 
// 参数说明     sda_pin         软件 IIC 数据引脚 
// 返回参数     void            
// 使用示例     soft_iic_init(&soft_iic_obj, addr, 100, PB6, PB7);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void soft_iic_init (soft_iic_info_struct *soft_iic_obj, uint8 addr, uint32 delay, gpio_pin_enum scl_pin, gpio_pin_enum sda_pin)
{
    zf_assert(soft_iic_obj != NULL);
    zf_assert(scl_pin != sda_pin);                                 // 醒醒！ scl_pin 与 sda_pin 怎么能填同一个引脚？
    soft_iic_obj->scl_pin = scl_pin;
    soft_iic_obj->sda_pin = sda_pin;
    soft_iic_obj->addr = addr;
    soft_iic_obj->delay = delay;
    gpio_init(scl_pin, GPO_OPEN_DTAIN, 1);                          	// 提取对应IO索引 AF功能编码
    gpio_init(sda_pin, GPO_OPEN_DTAIN, 1);                         	// 提取对应IO索引 AF功能编码
}

