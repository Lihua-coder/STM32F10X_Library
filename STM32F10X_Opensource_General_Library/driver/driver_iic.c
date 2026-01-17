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
#include "driver_iic.h"
/* ========================= 内部辅助函数/宏 ========================= */
#define I2C_TIMEOUT  ((uint32_t)0x0000FFFF)   /* 足够长的超时值 */

static I2C_TypeDef *I2C_NUM[2] = {I2C1, I2C2}; /* 仅用到 I2C1 也可 */

static uint32_t I2C_WaitEvent(I2C_TypeDef *I2Cx, uint32_t event)
{
    uint32_t timeout = I2C_TIMEOUT;
    while (!I2C_CheckEvent(I2Cx, event))
    {
        if (--timeout == 0) return 1; /* 超时返回非 0 */
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC START 信号
// 参数说明     iic_n           IIC 指定
// 返回参数     void
// 使用示例     iic_start(IIC_1);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
void iic_start(iic_index_enum iic_n)
{
    I2C_GenerateSTART(I2C_NUM[iic_n], ENABLE);
    I2C_WaitEvent(I2C_NUM[iic_n], I2C_EVENT_MASTER_MODE_SELECT);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC STOP 信号
// 参数说明     iic_n           IIC 指定
// 返回参数     void
// 使用示例     iic_stop(IIC_1);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
void iic_stop(iic_index_enum iic_n)
{
    I2C_GenerateSTOP(I2C_NUM[iic_n], ENABLE);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 发送 ACK/NAKC 信号 内部调用
// 参数说明     iic_n           IIC 指定
// 参数说明     ack             ACK 电平
// 返回参数     void
// 使用示例     iic_send_ack(IIC_1, 1);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
void iic_send_ack(iic_index_enum iic_n, uint8 ack)
{
    /* 接收模式下，CPU 要回 ACK/NACK 时调用 */
    if (ack)
        I2C_AcknowledgeConfig(I2C_NUM[iic_n], ENABLE);
    else
        I2C_AcknowledgeConfig(I2C_NUM[iic_n], DISABLE);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 获取 ACK/NAKC 信号
// 参数说明     iic_n           IIC 指定
// 返回参数     uint8           ACK 状态
// 使用示例     iic_wait_ack(IIC_1);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
uint8 iic_wait_ack(iic_index_enum iic_n)
{
    /* 主机发送完一个字节后，等待从机回 ACK */
    uint32_t timeout = I2C_TIMEOUT;
    I2C_ClearFlag(I2C_NUM[iic_n], I2C_FLAG_AF);
    while (I2C_GetFlagStatus(I2C_NUM[iic_n], I2C_FLAG_AF) == RESET)
    {
        if (--timeout == 0) return 1; /* 无应答返回 1 */
    }
    return 0; /* 收到 ACK 返回 0 */
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 发送 8bit 数据
// 参数说明     iic_n           IIC 指定
// 参数说明     dat            数据
// 返回参数     uint8           ACK 状态
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
uint8 iic_send_data(iic_index_enum iic_n, const uint8 dat)
{
    I2C_SendData(I2C_NUM[iic_n], dat);
    if (I2C_WaitEvent(I2C_NUM[iic_n], I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        return 1;
    if (I2C_WaitEvent(I2C_NUM[iic_n], I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        return 1;
    return 0;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 读取 8bit 数据
// 参数说明     iic_n           IIC 指定
// 参数说明     ack             ACK 或 NACK
// 返回参数     uint8           数据
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
uint8 iic_read_data(iic_index_enum iic_n, uint8 ack)
{
    iic_send_ack(iic_n, ack);
    while (!I2C_CheckEvent(I2C_NUM[iic_n], I2C_EVENT_MASTER_BYTE_RECEIVED));
    return I2C_ReceiveData(I2C_NUM[iic_n]);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口写 8bit 数据
// 参数说明     iic_n           IIC 指定
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     iic_write_8bit(IIC_1, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_write_8bit(iic_index_enum iic_n, const uint8 dat)
{
    iic_start(iic_n);
    iic_send_data(iic_n, dat);
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口写 8bit 数组
// 参数说明     iic_n           IIC 指定
// 参数说明     *dat           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void            
// 使用示例     iic_write_8bit_array(IIC_1, dat, 6);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_write_8bit_array(iic_index_enum iic_n, const uint8 *dat, uint32 len)
{
    iic_start(iic_n);
    for(uint32 i = 0; i < len; i++)
        iic_send_data(iic_n, dat[i]);
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口器写 16bit 数据
// 参数说明     iic_n           IIC 指定
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     iic_write_16bit(IIC_1, 0x0101);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_write_16bit(iic_index_enum iic_n, const uint16 dat)
{
    iic_start(iic_n);
    iic_send_data(iic_n, (uint8)(dat >> 8));
    iic_send_data(iic_n, (uint8)(dat & 0xFF));
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口写 16bit 数组
// 参数说明     iic_n           IIC 指定
// 参数说明     *dat           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void            
// 使用示例     iic_write_16bit_array(IIC_1, dat, 6);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_write_16bit_array(iic_index_enum iic_n, const uint16 *dat, uint32 len)
{
    iic_start(iic_n);
    for(uint32 i = 0; i < len; i++)
    {
        iic_send_data(iic_n, (uint8)(dat[i] >> 8));
        iic_send_data(iic_n, (uint8)(dat[i] & 0xFF));
    }
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口向传感器寄存器写 8bit 数据
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     iic_write_8bit_register(IIC_1, 0x01, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_write_8bit_register(iic_index_enum iic_n, const uint8 reg, const uint8 dat)
{
    iic_start(iic_n);
    iic_send_data(iic_n, reg);
    iic_send_data(iic_n, dat);
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口向传感器寄存器写 8bit 数组
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void            
// 使用示例     iic_write_8bit_registers(IIC_1, 0x01, dat, 6);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_write_8bit_registers(iic_index_enum iic_n, const uint8 reg, const uint8 *dat, uint32 len)
{
    iic_start(iic_n);
    iic_send_data(iic_n, reg);
    for(uint32 i = 0; i < len; i++)
        iic_send_data(iic_n, dat[i]);
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口向传感器寄存器写 16bit 数据
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     iic_write_16bit_register(IIC_1, 0x0101, 0x0101);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_write_16bit_register(iic_index_enum iic_n, const uint16 reg, const uint16 dat)
{
    iic_start(iic_n);
    iic_send_data(iic_n, (uint8)(reg >> 8));
    iic_send_data(iic_n, (uint8)(reg & 0xFF));
    iic_send_data(iic_n, (uint8)(dat >> 8));
    iic_send_data(iic_n, (uint8)(dat & 0xFF));
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口向传感器寄存器写 16bit 数组
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           数据存放缓冲区
// 参数说明     len             缓冲区长度
// 返回参数     void            
// 使用示例     iic_write_16bit_registers(IIC_1, 0x0101, dat, 6);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_write_16bit_registers(iic_index_enum iic_n, const uint16 reg, const uint16 *dat, uint32 len)
{
    iic_start(iic_n);
    iic_send_data(iic_n, (uint8)(reg >> 8));
    iic_send_data(iic_n, (uint8)(reg & 0xFF));
    for(uint32 i = 0; i < len; i++)
    {
        iic_send_data(iic_n, (uint8)(dat[i] >> 8));
        iic_send_data(iic_n, (uint8)(dat[i] & 0xFF));
    }
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口读取 8bit 数据
// 参数说明     iic_n           IIC 指定
// 返回参数     uint8           返回读取的 8bit 数据
// 使用示例     iic_read_8bit(IIC_1);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 iic_read_8bit(iic_index_enum iic_n)
{
    uint8 dat;
    iic_start(iic_n);
    dat = iic_read_data(iic_n, 0); /* NACK 结束 */
    iic_stop(iic_n);
    return dat;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口从传感器寄存器读取 8bit 数组
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void            
// 使用示例     iic_read_8bit_array(IIC_1, dat, 8);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_read_8bit_array(iic_index_enum iic_n, uint8 *dat, uint32 len)
{
    iic_start(iic_n);
    for(uint32 i = 0; i < len; i++)
        dat[i] = iic_read_data(iic_n, (i == len - 1) ? 0 : 1);
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口读取 16bit 数据
// 参数说明     iic_n           IIC 指定
// 返回参数     uint16          返回读取的 16bit 数据
// 使用示例     iic_read_16bit(IIC_1);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint16 iic_read_16bit(iic_index_enum iic_n)
{
    uint16 dat;
    iic_start(iic_n);
    dat  = (uint16)iic_read_data(iic_n, 1) << 8;
    dat |= iic_read_data(iic_n, 0);
    iic_stop(iic_n);
    return dat;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口读取 16bit 数组
// 参数说明     iic_n           IIC 指定
// 参数说明     *dat           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void            
// 使用示例     iic_read_16bit_array(IIC_1, dat, 8);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_read_16bit_array(iic_index_enum iic_n, uint16 *dat, uint32 len)
{
    iic_start(iic_n);
    for(uint32 i = 0; i < len; i++)
    {
        dat[i]  = (uint16)iic_read_data(iic_n, (i == len - 1) ? 0 : 1) << 8;
        dat[i] |= iic_read_data(iic_n, (i == len - 1) ? 0 : 1);
    }
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口从传感器寄存器读取 8bit 数据
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint8           返回读取的 8bit 数据
// 使用示例     iic_read_8bit_register(IIC_1, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 iic_read_8bit_register(iic_index_enum iic_n, const uint8 reg)
{
    uint8 dat;
    iic_start(iic_n);
    iic_send_data(iic_n, reg);
    /* 重启总线进入读模式 */
    iic_start(iic_n);
    dat = iic_read_data(iic_n, 0);
    iic_stop(iic_n);
    return dat;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口从传感器寄存器读取 8bit 数组
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void            
// 使用示例     iic_read_8bit_registers(IIC_1, 0x01, dat, 8);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_read_8bit_registers(iic_index_enum iic_n, const uint8 reg, uint8 *dat, uint32 len)
{
    iic_start(iic_n);
    iic_send_data(iic_n, reg);
    /* 重启总线进入读模式 */
    iic_start(iic_n);
    for(uint32 i = 0; i < len; i++)
        dat[i] = iic_read_data(iic_n, (i == len - 1) ? 0 : 1);
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口从传感器寄存器读取 16bit 数据
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint16          返回读取的 16bit 数据
// 使用示例     iic_read_16bit_register(IIC_1, 0x0101);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint16 iic_read_16bit_register(iic_index_enum iic_n, const uint16 reg)
{
    uint16 dat;
    iic_start(iic_n);
    iic_send_data(iic_n, (uint8)(reg >> 8));
    iic_send_data(iic_n, (uint8)(reg & 0xFF));
    /* 重启总线进入读模式 */
    iic_start(iic_n);
    dat  = (uint16)iic_read_data(iic_n, 1) << 8;
    dat |= iic_read_data(iic_n, 0);
    iic_stop(iic_n);
    return dat;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口从传感器寄存器读取 16bit 数组
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     *dat           要读取的数据的缓冲区指针
// 参数说明     len             要读取的数据长度
// 返回参数     void            
// 使用示例     iic_read_16bit_registers(IIC_1, 0x0101, dat, 8);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_read_16bit_registers(iic_index_enum iic_n, const uint16 reg, uint16 *dat, uint32 len)
{
    iic_start(iic_n);
    iic_send_data(iic_n, (uint8)(reg >> 8));
    iic_send_data(iic_n, (uint8)(reg & 0xFF));
    /* 重启总线进入读模式 */
    iic_start(iic_n);
    for(uint32 i = 0; i < len; i++)
    {
        dat[i]  = (uint16)iic_read_data(iic_n, (i == len - 1) ? 0 : 1) << 8;
        dat[i] |= iic_read_data(iic_n, (i == len - 1) ? 0 : 1);
    }
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口传输 8bit 数组 先写后读取
// 参数说明     iic_n           IIC 指定
// 参数说明     *write_data     发送数据存放缓冲区
// 参数说明     write_len       发送缓冲区长度
// 参数说明     *read_data      读取数据存放缓冲区
// 参数说明     read_len        读取缓冲区长度
// 返回参数     void            
// 使用示例     iic_transfer_8bit_array(IIC_1, addr, dat, 64, dat, 64);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_transfer_8bit_array(iic_index_enum iic_n, const uint8 *wdat, uint32 wlen, uint8 *rdat, uint32 rlen)
{
    iic_start(iic_n);
    for(uint32 i = 0; i < wlen; i++)
        iic_send_data(iic_n, wdat[i]);
    /* 重启总线进入读模式 */
    iic_start(iic_n);
    for(uint32 i = 0; i < rlen; i++)
        rdat[i] = iic_read_data(iic_n, (i == rlen - 1) ? 0 : 1);
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口传输 16bit 数组 先写后读取
// 参数说明     iic_n   			  IIC 指定
// 参数说明     *write_data     发送数据存放缓冲区
// 参数说明     write_len       发送缓冲区长度
// 参数说明     *read_data      读取数据存放缓冲区
// 参数说明     read_len        读取缓冲区长度
// 返回参数     void            
// 使用示例     iic_transfer_16bit_array(IIC_1, addr, dat, 64, dat, 64);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_transfer_16bit_array(iic_index_enum iic_n, const uint16 *wdat, uint32 wlen, uint16 *rdat, uint32 rlen)
{
    iic_start(iic_n);
    for(uint32 i = 0; i < wlen; i++)
    {
        iic_send_data(iic_n, (uint8)(wdat[i] >> 8));
        iic_send_data(iic_n, (uint8)(wdat[i] & 0xFF));
    }
    /* 重启总线进入读模式 */
    iic_start(iic_n);
    for(uint32 i = 0; i < rlen; i++)
    {
        rdat[i]  = (uint16)iic_read_data(iic_n, (i == rlen - 1) ? 0 : 1) << 8;
        rdat[i] |= iic_read_data(iic_n, (i == rlen - 1) ? 0 : 1);
    }
    iic_stop(iic_n);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口 SCCB 模式向传感器寄存器写 8bit 数据
// 参数说明     *soft_iic_obj   软件 IIC 指定信息 
// 参数说明     register_name   传感器的寄存器地址
// 参数说明     dat            要写入的数据
// 返回参数     void            
// 使用示例     iic_sccb_write_register(IIC_1, 0x01, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_sccb_write_register(iic_index_enum iic_n, const uint8 reg, uint8 dat)
{
    iic_write_8bit_register(iic_n, reg, dat);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口 SCCB 模式从传感器寄存器读取 8bit 数据
// 参数说明     iic_n           IIC 指定
// 参数说明     register_name   传感器的寄存器地址
// 返回参数     uint8           返回读取的 8bit 数据
// 使用示例     iic_sccb_read_register(IIC_1, 0x01);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 iic_sccb_read_register(iic_index_enum iic_n, const uint8 reg)
{
    return iic_read_8bit_register(iic_n, reg);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     硬件 IIC 接口初始化 默认 MASTER 模式 不提供 SLAVE 模式
// 参数说明     iic_n   			  IIC 指定
// 参数说明     addr            IIC 地址 这里需要注意 标准七位地址 最高位忽略 写入时请务必确认无误
// 参数说明     speed_khz       SCL时钟频率，数值越大SCL频率越高，数据传输越快
// 参数说明     scl_pin         IIC 时钟引脚 
// 参数说明     sda_pin         IIC 数据引脚 
// 返回参数     void            
// 使用示例     iic_init(IIC_1, addr, 100, PB6, PB7);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void iic_init(iic_index_enum iic_n, uint8 addr, uint32 speed_khz, iic_pin_enum scl_pin, iic_pin_enum sda_pin)
{
    I2C_InitTypeDef  iic;
    GPIO_InitTypeDef gpio;
    GPIO_TypeDef*    gpio_port;
    uint16_t         gpio_pin_scl;
    uint16_t         gpio_pin_sda;
    I2C_TypeDef*     i2c_x;

    if (iic_n == IIC_1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

        bool need_remap = (scl_pin == IIC1_SCL_PB8) || (sda_pin == IIC1_SDA_PB9);
        if (need_remap)
        {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
            GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);
            gpio_port    = GPIOB;
            gpio_pin_scl = GPIO_Pin_8;
            gpio_pin_sda = GPIO_Pin_9;
        }
        else
        {
            gpio_port    = GPIOB;
            gpio_pin_scl = GPIO_Pin_6;
            gpio_pin_sda = GPIO_Pin_7;
        }
        i2c_x = I2C1;
    }
    else /* IIC_2 */
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
        gpio_port    = GPIOB;
        gpio_pin_scl = GPIO_Pin_10;
        gpio_pin_sda = GPIO_Pin_11;
        i2c_x = I2C2;
    }

    gpio.GPIO_Pin   = gpio_pin_scl | gpio_pin_sda;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode  = GPIO_Mode_AF_OD;
    GPIO_Init(gpio_port, &gpio);

    I2C_DeInit(i2c_x);
    iic.I2C_Mode                = I2C_Mode_I2C;
    iic.I2C_DutyCycle           = I2C_DutyCycle_2;
    iic.I2C_OwnAddress1         = addr << 1;
    iic.I2C_Ack                 = I2C_Ack_Enable;
    iic.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    iic.I2C_ClockSpeed          = speed_khz * 1000;
    I2C_Init(i2c_x, &iic);
    I2C_Cmd(i2c_x, ENABLE);
}

