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
#include "driver_dma.h"
static DMA_Channel_TypeDef* const DMA1_ChannelTable[] = {
    DMA1_Channel1, 
		DMA1_Channel2, 
		DMA1_Channel3, 
		DMA1_Channel4,
    DMA1_Channel5, 
		DMA1_Channel6,
		DMA1_Channel7
};
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      dma1初始化					（存储器→存储器）
// 参数说明      dma1_ch             选择DMA1通道
// 参数说明      source_addr         设置源地址
// 参数说明      destination_addr    设置目的地址
// 参数说明      datasize				     数据宽度  支持 DMA_Byte/DMA_HalfWord/DMA_Word
// 参数说明      dma_count           设置dma搬移次数
// 参数说明      priority            设置dma优先级	DMA_Priority_Low/Medium/High/VeryHigh
// 参数说明      dir           			 设置dma方向	DMA_DIR_PeripheralSRC（外设→内存）/DMA_DIR_PeripheralDST（内存→外设）
// 返回参数      void
// 使用示例      dma1_init(dma1_CH1, DataA, DataB, DMA_Byte, 3, DMA_Priority_Medium);
// 备注信息			 初始化后不会进行转运，转运需使用void dma1_transfer(dma_channel_enum dma1_ch, uint16_t dma_count);
//-------------------------------------------------------------------------------------------------------------------
void dma1_init(dma_channel_enum dma1_ch, uint32 source_addr, uint32 destination_addr, 
	uint32 datasize, uint16 dma_count, uint32 priority, uint32 dir)
{
    if (dma1_ch > dma1_CH7) return;                 /* 越界保护 */

    DMA_Channel_TypeDef* ch = DMA1_ChannelTable[dma1_ch];

    /* 1. 开时钟 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* 2. 关闭通道 + 复位寄存器（官方例程均如此）*/
    DMA_Cmd(ch, DISABLE);
		while(ch->CCR & 1U); /* 等 EN 清零 */
		DMA_DeInit(ch);

    /* 3. 填结构体 */
    DMA_InitTypeDef dmaInit;
    DMA_StructInit(&dmaInit);
    dmaInit.DMA_PeripheralBaseAddr = source_addr;
    dmaInit.DMA_MemoryBaseAddr     = destination_addr;
    dmaInit.DMA_DIR                = dir;
    dmaInit.DMA_BufferSize         = dma_count;
    dmaInit.DMA_PeripheralInc      = DMA_PeripheralInc_Enable;
    dmaInit.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dmaInit.DMA_PeripheralDataSize = datasize;
    dmaInit.DMA_MemoryDataSize     = datasize;
    dmaInit.DMA_Mode               = DMA_Mode_Normal;
    dmaInit.DMA_Priority           = priority;
    dmaInit.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(ch, &dmaInit);

    /* 4. 启动传输关闭 */
    DMA_Cmd(ch, DISABLE);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     dma1 传输禁止
// 参数说明     ch              选择 dma1 通道
// 返回参数     void
// 使用示例     dma_disable(dma1_CH1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void dma1_disable(dma_channel_enum dma1_ch)
{
    if(dma1_ch > dma1_CH7) return;
    DMA_Cmd(DMA1_ChannelTable[dma1_ch], DISABLE);
    /* 一次清掉所有中断标志，防止以后开错误中断时反复进 IRQ */
    DMA_ClearITPendingBit(DMA1_IT_GL1 << dma1_ch);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     dma1 传输使能
// 参数说明     ch              选择 dma1 通道 
// 返回参数     void
// 使用示例     dma_enable(dma1_CH1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void dma1_enable(dma_channel_enum dma1_ch)
{
    if (dma1_ch > dma1_CH7) return;          /* 越界保护 */

    DMA_Cmd(DMA1_ChannelTable[dma1_ch], ENABLE);           /* 置 EN 位，立即启动 */
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      启动DMA1数据转运
// 参数说明      dma1_ch             选择DMA1通道
// 参数说明      dma_count           设置dma搬移次数
// 返回参数      void
// 使用示例      dma1_transfer(dma1_CH2, 128);   /* 再转运 128 个 word */
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void dma1_transfer(dma_channel_enum dma1_ch, uint16_t dma_count)
{
    if (dma1_ch > dma1_CH7) return;
    DMA_Channel_TypeDef* ch = DMA1_ChannelTable[dma1_ch];

    /* 1. 关闭通道 */
    DMA_Cmd(ch, DISABLE);
    while (ch->CCR & 1U);          /* 等 EN 位归零 */

    /* 2. 重新装载传输数目 */
    ch->CNDTR = dma_count;

    /* 3. 启动新一轮传输 */
    DMA_Cmd(ch, ENABLE);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      设置DMA传输长度并启动
// 参数说明      ch            			 选择DMA1通道
// 参数说明      len           			 设置长度
// 返回参数      void
// 使用示例      dma1_transfer(dma1_CH2, 128);   /* 再转运 128 个 word */
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void dma_start(DMA_Channel_TypeDef* ch, uint16 len)
{
    /* 确保通道关闭 */
    DMA_Cmd(ch, DISABLE);
    while(ch->CCR & 1);  /* 等待EN位清零 */
    
    /* 只设置长度 */
    ch->CNDTR = len;
    
    /* 启动 */
    DMA_Cmd(ch, ENABLE);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      等待DMA完成
// 参数说明      ch            			 选择DMA1通道
// 返回参数      void
// 使用示例      dma1_transfer(dma1_CH2, 128);   /* 再转运 128 个 word */
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void dma_wait_done(DMA_Channel_TypeDef* ch)
{
    while(ch->CNDTR);
}

