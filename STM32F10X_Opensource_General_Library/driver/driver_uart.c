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
#include "driver_uart.h"
/* =========================  用户可改参数  ========================= */
#define UART_RX_DMA_BUF_SIZE   256        /* 接收 DMA 环形缓冲区大小 */
#define UART_TX_DMA_BUF_SIZE   256        /* 发送 DMA 缓冲区大小     */
/* ================================================================= */
/* 每路 UART 私有硬件信息 */
typedef struct
{
    USART_TypeDef       *uart;                              /* 串口寄存器基址           */
    DMA_Channel_TypeDef *rx_dma_ch;                         /* 接收 DMA 通道            */
    DMA_Channel_TypeDef *tx_dma_ch;                         /* 发送 DMA 通道            */
    uint8               rx_dma_buf[UART_RX_DMA_BUF_SIZE];   /* 接收原始 DMA 缓冲区      */
    fifo_struct         rx_fifo;                            /* 接收数据 FIFO            */
    uint8               tx_dma_buf[UART_TX_DMA_BUF_SIZE];   /* 发送原始 DMA 缓冲区      */
    fifo_struct         tx_fifo;                            /* 发送数据 FIFO            */
    uint8               tx_dma_running;                     /* 发送 DMA 忙标志          */
}uart_hw_t;
/* 硬件映射表 */
static uart_hw_t uart_hw[3] =
{
    [UART_1] = { .uart = USART1, .rx_dma_ch = DMA1_Channel5, .tx_dma_ch = DMA1_Channel4 },
    [UART_2] = { .uart = USART2, .rx_dma_ch = DMA1_Channel6, .tx_dma_ch = DMA1_Channel7 },
    [UART_3] = { .uart = USART3, .rx_dma_ch = DMA1_Channel3, .tx_dma_ch = DMA1_Channel2 },
};

/* ----------------------------------------------------------
 * @brief  NVIC 配置（内部调用）
 * ---------------------------------------------------------- */
static void nvic_cfg(IRQn_Type irq, uint8 pre)
{
    NVIC_InitTypeDef nvic_structure =
    {
        .NVIC_IRQChannel                   = irq,
        .NVIC_IRQChannelPreemptionPriority = pre,//抢占优先级
        .NVIC_IRQChannelSubPriority        = 0,//响应优先级
        .NVIC_IRQChannelCmd                = ENABLE
    };
    NVIC_Init(&nvic_structure);
}

/* ----------------------------------------------------------
 * @brief  启动一次 DMA 发送（内部调用）
 * ---------------------------------------------------------- */
static void uart_start_tx_dma(uart_index_enum uartn)
{
    uart_hw_t *hw = &uart_hw[uartn];
    uint32 len = fifo_used(&hw->tx_fifo);
    if (!len) { hw->tx_dma_running = 0; return; }
    if (len > UART_TX_DMA_BUF_SIZE) len = UART_TX_DMA_BUF_SIZE;

    fifo_read_buffer(&hw->tx_fifo, hw->tx_dma_buf, &len, FIFO_READ_ONLY);

    DMA_Cmd(hw->tx_dma_ch, DISABLE);
    while (hw->tx_dma_ch->CCR & (1U << 0));          /* 等待 EN 位清零 */
    DMA_SetCurrDataCounter(hw->tx_dma_ch, len);
    DMA_Cmd(hw->tx_dma_ch, ENABLE);
    USART_DMACmd(hw->uart, USART_DMAReq_Tx, ENABLE);
    hw->tx_dma_running = 1;
}

/* ----------------------------------------------------------
 * @brief  DMA 发送完成中断回调（用户可定制）
 * ---------------------------------------------------------- */
void uart_tx_interrupt(uart_index_enum uartn, uint32 status)
{
    uart_hw_t *hw = &uart_hw[uartn];
    hw->tx_dma_running = 0;
    uart_start_tx_dma(uartn);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口单字节发送（阻塞）
// 参数说明     uartn           串口号，可选UART_1、UART_2、UART_3
// 参数说明     dat             待发送字节
// 返回参数     void
// 使用示例     uart_write_byte(UART_1, 0x55);
// 备注信息     内部等待TXE置位，确保发送完成
//-------------------------------------------------------------------------------------------------------------------
void uart_write_byte(uart_index_enum uartn, uint8 dat)
{
    USART_TypeDef *u = uart_hw[uartn].uart;
    while (!(u->SR & USART_SR_TXE));
    u->DR = dat;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口缓冲区发送（阻塞）
// 参数说明     uartn           串口号，可选UART_1、UART_2、UART_3
// 参数说明     buff            数据缓冲区首地址
// 参数说明     len             待发送长度
// 返回参数     void
// 使用示例     uart_write_buffer(UART_1, buf, 64);
// 备注信息     逐字节调用uart_write_byte，适合少量数据
//-------------------------------------------------------------------------------------------------------------------
void uart_write_buffer(uart_index_enum uartn, const uint8 *buff, uint32 len)
{ while (len--) uart_write_byte(uartn, *buff++); }

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口字符串发送（阻塞）
// 参数说明     uartn           串口号，可选UART_1、UART_2、UART_3
// 参数说明     str             以'\0'结尾的字符串
// 返回参数     void
// 使用示例     uart_write_string(UART_1, "hello\r\n");
// 备注信息     内部调用uart_write_buffer
//-------------------------------------------------------------------------------------------------------------------
void uart_write_string(uart_index_enum uartn, const char *str)
{ uart_write_buffer(uartn, (uint8 *)str, strlen(str)); }

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口单字节接收（阻塞）
// 参数说明     uartn           串口号，可选UART_1、UART_2、UART_3
// 返回参数     uint8           收到的字节
// 使用示例     uint8 dat = uart_read_byte(UART_1);
// 备注信息     内部等待直到收到数据
//-------------------------------------------------------------------------------------------------------------------
uint8 uart_read_byte(uart_index_enum uartn)
{
    uint8 d;
    while (!uart_query_byte(uartn, &d));
    return d;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口非阻塞查询单字节
// 参数说明     uartn           串口号，可选UART_1、UART_2、UART_3
// 参数说明     dat             用于保存接收到的字节
// 返回参数     uint8           1表示成功，0表示无数据
// 使用示例     uint8 dat; if(uart_query_byte(UART_1, &dat)) { ... }
// 备注信息     不阻塞，立即返回
//-------------------------------------------------------------------------------------------------------------------
uint8 uart_query_byte(uart_index_enum uartn, uint8 *dat)
{
    return fifo_read_element(&uart_hw[uartn].rx_fifo, dat, FIFO_READ_AND_CLEAN) == FIFO_SUCCESS;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     串口初始化（FIFO+DMA+中断）
// 参数说明     uartn           串口号，可选UART_1、UART_2、UART_3
// 参数说明     baud            波特率，常用115200、9600等
// 参数说明     tx_pin          发送引脚，参考uart_tx_pin_enum枚举
// 参数说明     rx_pin          接收引脚，参考uart_rx_pin_enum枚举
// 返回参数     void
// 使用示例     uart_init(UART_1, 115200, UART1_TX_PA9, UART1_RX_PA10);
// 备注信息     内部已完成NVIC、DMA、GPIO、USART配置
//-------------------------------------------------------------------------------------------------------------------
void uart_init(uart_index_enum uartn, uint32 baud, uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin)
{
    uart_hw_t *hw = &uart_hw[uartn];
    USART_InitTypeDef usart_init_struct;
    GPIO_InitTypeDef  gpio_init_struct;
    uint32_t tx_port_clk;
    GPIO_TypeDef *tx_port, *rx_port;
    uint16_t tx_pin_src, rx_pin_src;

    /* 1. 根据 uartn 取 TX/RX 端口及复用源 */
    if (uartn == UART_1)
    {
        tx_port = (tx_pin == UART1_TX_PA9)  ? GPIOA : GPIOB;
        rx_port = (rx_pin == UART1_RX_PA10) ? GPIOA : GPIOB;
        tx_pin_src = (tx_pin == UART1_TX_PA9)  ? GPIO_PinSource9  : GPIO_PinSource6;
        rx_pin_src = (rx_pin == UART1_RX_PA10) ? GPIO_PinSource10 : GPIO_PinSource7;
        tx_port_clk = RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB;
    }
    else if (uartn == UART_2)
    {
        tx_port = GPIOA; rx_port = GPIOA;
        tx_pin_src = GPIO_PinSource2; rx_pin_src = GPIO_PinSource3;
        tx_port_clk = RCC_APB2Periph_GPIOA;
    }
    else /* UART_3 */
    {
        tx_port = GPIOB; rx_port = GPIOB;
        tx_pin_src = GPIO_PinSource10; rx_pin_src = GPIO_PinSource11;
        tx_port_clk = RCC_APB2Periph_GPIOB;
    }

    /* 2. 开时钟 */
    RCC_APB2PeriphClockCmd(tx_port_clk | RCC_APB2Periph_AFIO, ENABLE);
    if (uartn == UART_1) RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    else if (uartn == UART_2) RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    else                      RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* 3. GPIO 配置 */
    if (uartn == UART_1 && tx_port == GPIOB)
        GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);//重映射

    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init_struct.GPIO_Pin   = (1 << tx_pin_src);
    gpio_init_struct.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(tx_port, &gpio_init_struct);
    gpio_init_struct.GPIO_Pin   = (1 << rx_pin_src);
    gpio_init_struct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(rx_port, &gpio_init_struct);

    /* 4. USART 配置 */
    USART_StructInit(&usart_init_struct);
    usart_init_struct.USART_BaudRate            = baud;
    usart_init_struct.USART_WordLength        = USART_WordLength_8b;
    usart_init_struct.USART_StopBits          = USART_StopBits_1;
    usart_init_struct.USART_Parity            = USART_Parity_No;
    usart_init_struct.USART_Mode              = USART_Mode_Rx | USART_Mode_Tx;
    usart_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(hw->uart, &usart_init_struct);
		
    /* 5. NVIC 配置：串口接收 + DMA 发送中断 抢占优先级写死，响应优先级 0 */
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    if (uartn == UART_1)
    {
        nvic_cfg(USART1_IRQn, 1); // 串口接收抢占=1
        nvic_cfg(DMA1_Channel4_IRQn, 1);// DMA-TX 抢占=1
    }
    else if (uartn == UART_2)
    {
        nvic_cfg(USART2_IRQn, 2);
        nvic_cfg(DMA1_Channel7_IRQn, 2);
    }
    else
    {
        nvic_cfg(USART3_IRQn, 3);
        nvic_cfg(DMA1_Channel2_IRQn, 3);
    }

    /* 6. FIFO 初始化 */
    fifo_init(&hw->rx_fifo, FIFO_DATA_8BIT, hw->rx_dma_buf, UART_RX_DMA_BUF_SIZE);
    fifo_init(&hw->tx_fifo, FIFO_DATA_8BIT, hw->tx_dma_buf, UART_TX_DMA_BUF_SIZE);
    hw->tx_dma_running = 0;

    /* 7. 配置 DMA 循环模式用于接收 */
    DMA_DeInit(hw->rx_dma_ch);
    DMA_InitTypeDef dma;
    DMA_StructInit(&dma);
    dma.DMA_PeripheralBaseAddr = (uint32)&hw->uart->DR;
    dma.DMA_MemoryBaseAddr     = (uint32)hw->rx_dma_buf;
    dma.DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize         = UART_RX_DMA_BUF_SIZE;
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode               = DMA_Mode_Circular;
    dma.DMA_Priority           = DMA_Priority_VeryHigh;
    dma.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(hw->rx_dma_ch, &dma);
    DMA_Cmd(hw->rx_dma_ch, ENABLE);
    USART_DMACmd(hw->uart, USART_DMAReq_Rx, ENABLE);

    /* 8. 使能发送 DMA 传输完成中断 */
    DMA_ITConfig(hw->tx_dma_ch, DMA_IT_TC, ENABLE);

    /* 9. 使能串口 */
    USART_Cmd(hw->uart, ENABLE);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     DMA方式发送数据包（非阻塞）
// 参数说明     uartn           串口号，可选UART_1、UART_2、UART_3
// 参数说明     buff            待发送数据缓冲区
// 参数说明     len             待发送长度
// 返回参数     void
// 使用示例     uart_write_buffer_dma(UART_1, buf, 64);
// 备注信息     数据先写入FIFO，再由DMA后台发送
//-------------------------------------------------------------------------------------------------------------------
fifo_state_enum uart_write_buffer_dma(uart_index_enum uartn, const uint8 *buff, uint32 len)
{
    uart_hw_t *hw = &uart_hw[uartn];
    if(!len) return FIFO_SUCCESS;
    fifo_state_enum ret = fifo_write_buffer(&hw->tx_fifo, (void *)buff, len);
    if(ret == FIFO_SUCCESS && !hw->tx_dma_running)
        uart_start_tx_dma(uartn);
    return ret;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     发送固定长度ASCII数字，前面补0
// 参数说明     uartn           串口号，可选UART_1、UART_2、UART_3
// 参数说明     number          待发送数字
// 参数说明     length          固定长度，不足前面补0
// 返回参数     void
// 使用示例     uart_send_number(UART_1, 123, 5); //输出"00123"
// 备注信息     最大支持10位
//-------------------------------------------------------------------------------------------------------------------
void uart_send_number(uart_index_enum uartn, uint32 number, uint8 length)
{
    char buf[11];
    uint8_t i = length > 10 ? 10 : length;
    buf[i] = '\0';
    while (i--)
    {
        buf[i] = (number % 10) + '0';
        number /= 10;
    }
    uart_write_buffer_dma(uartn, (uint8 *)buf, length);
}





/* ========================= 串口中断服务函数 ========================= */

/**
 * @brief  USART1 接收中断，将数据压入 FIFO
 */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t dat = USART_ReceiveData(USART1);
        fifo_write_element(&uart_hw[UART_1].rx_fifo, dat);
    }
		
#if DEBUG_UART_USE_INTERRUPT                        // 如果开启 debug 串口中断
        debug_interrupr_handler();                  // 调用 debug 串口接收处理函数 数据会被 debug 环形缓冲区读取
#endif                                              // 如果修改了 DEBUG_UART_INDEX 那这段代码需要放到对应的串口中断去

}

/**
 * @brief  USART2 接收中断
 */
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uint8_t dat = USART_ReceiveData(USART2);
        fifo_write_element(&uart_hw[UART_2].rx_fifo, dat);
    }
}

/**
 * @brief  USART3 接收中断
 */
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uint8_t dat = USART_ReceiveData(USART3);
        fifo_write_element(&uart_hw[UART_3].rx_fifo, dat);
    }
}

/**
 * @brief  DMA1 通道 4 传输完成中断（USART1 TX）
 */
void DMA1_Channel4_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC4))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC4);
        uart_tx_interrupt(UART_1, 0);
    }
}

/**
 * @brief  DMA1 通道 7 传输完成中断（USART2 TX）
 */
void DMA1_Channel7_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC7))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC7);
        uart_tx_interrupt(UART_2, 0);
    }
}

/**
 * @brief  DMA1 通道 2 传输完成中断（USART3 TX）
 */
void DMA1_Channel2_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC2))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC2);
        uart_tx_interrupt(UART_3, 0);
    }
}

