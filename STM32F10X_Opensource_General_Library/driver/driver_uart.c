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

/* 串口句柄 + 循环缓冲区定义 */
typedef struct
{
    USART_TypeDef       *uartx;
    DMA_Channel_TypeDef *tx_dma_ch;
    DMA_Channel_TypeDef *rx_dma_ch;
    uint8               rx_buf[UART_RX_BUF_SIZE];
    volatile uint16     rx_head;
    volatile uint16     rx_tail;
}uart_handle_t;

/* 静态句柄 */
static uart_handle_t g_uart[3] =
{
    {USART1, DMA1_Channel4, DMA1_Channel5},   /* UART1 */
    {USART2, DMA1_Channel7, DMA1_Channel6},   /* UART2 */
    {USART3, DMA1_Channel2, DMA1_Channel3},   /* UART3 */
};

/* 取串口号索引 */
static inline uint8 uart_idx(uart_index_enum uartn)
{
    return (uint8)uartn;   /* UART_1=0, UART_2=1, UART_3=2 */
}

// 获取USART对应的中断通道
static inline uint8_t uart_get_irq_channel(USART_TypeDef *u)
{
    if(u == USART1) return USART1_IRQn;
    if(u == USART2) return USART2_IRQn;
    return USART3_IRQn;
}

#if UART_TX_USE_DMA
  /* TX DMA通道标志 */
  static const uint32 g_dma_tx_tc_flag[3] = {
      DMA1_FLAG_TC4,   /* UART1 TX - Channel 4 */
      DMA1_FLAG_TC7,   /* UART2 TX - Channel 7 */
      DMA1_FLAG_TC2    /* UART3 TX - Channel 2 */
  };
#endif

#if UART_RX_USE_DMA
  /* RX DMA通道中断标志 */
  static const uint32 g_dma_rx_tc_flag[3] = {
      DMA1_IT_TC5,   /* UART1 RX - Channel 5 */
      DMA1_IT_TC6,   /* UART2 RX - Channel 6 */
      DMA1_IT_TC3    /* UART3 RX - Channel 3 */
  };

// 获取DMA通道对应的中断通道
static inline uint8_t uart_get_dma_irq_channel(uint8 idx)
{
    // DMA通道: UART1=CH5, UART2=CH6, UART3=CH3
    const uint8_t dma_irq[] = {DMA1_Channel5_IRQn, DMA1_Channel6_IRQn, DMA1_Channel3_IRQn};
    return dma_irq[idx];
}

#endif	
//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口发送写入
// 参数说明       uart_n          串口模块号 
// 参数说明       dat             需要发送的字节
// 返回参数       void
// 使用示例       uart_write_byte(UART_1, 0xA5);                    // 往串口1的发送缓冲区写入0xA5，写入后仍然会发送数据，但是会减少CPU在串口的执行时
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void uart_write_byte(uart_index_enum uartn, const uint8 dat)
{
#if UART_TX_USE_DMA 
    uint8 tmp = dat;
    uart_write_buffer(uartn, &tmp, 1);
#else
    USART_TypeDef *u = g_uart[uart_idx(uartn)].uartx;
    while(USART_GetFlagStatus(u, USART_FLAG_TXE) == RESET);
    USART_SendData(u, dat);	
#endif	
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口发送数组
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 参数说明       *buff           要发送的数组地址
// 参数说明       len             发送长度
// 返回参数       void
// 使用示例       uart_write_buffer(UART_1, &a[0], 5);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void uart_write_buffer(uart_index_enum uartn, const uint8 *buff, uint32 len)
{
	  if(len == 0 || buff == NULL) return;
#if UART_TX_USE_DMA 
    uint8 idx = uart_idx(uartn);
    uart_handle_t *hu = &g_uart[idx];

    while(hu->tx_dma_ch->CCR & DMA_CCR1_EN);          /* 等上一次完成 */
    hu->tx_dma_ch->CMAR  = (uint32)buff;              /* 源地址 */
    hu->tx_dma_ch->CNDTR = len;                       /* 长度 */
		DMA_ClearFlag(g_dma_tx_tc_flag[idx]);             /* 清 TC标志 - 使用查表方式 */       /* 清 TC */
    DMA_Cmd(hu->tx_dma_ch, ENABLE);                   /* 启动 */

    while(DMA_GetFlagStatus(g_dma_tx_tc_flag[idx]) == RESET); /* 阻塞等待完成 */
	  DMA_Cmd(hu->tx_dma_ch, DISABLE);
#else
    while(len--) uart_write_byte(uartn, *buff++);	
	
#endif	
	
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口发送字符串
// 参数说明       uart_n          串口模块号 
// 参数说明       *str            要发送的字符串地址
// 返回参数       void
// 使用示例       uart_write_string(UART_1, "Lihua");
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void uart_write_string(uart_index_enum uartn, const char *str)
{
    uart_write_buffer(uartn, (uint8*)str, strlen(str));
}

/*============================ 接收部分 ============================*/
/* 把收到的 1 字节压入 FIFO */
static void uart_rx_push(uart_index_enum uartn, uint8 dat)
{
    uint8 idx = uart_idx(uartn);
    uart_handle_t *hu = &g_uart[idx];
    uint16 next = (hu->rx_head + 1) % UART_RX_BUF_SIZE;
    if(next != hu->rx_tail)           /* 没满 */
    {
        hu->rx_buf[hu->rx_head] = dat;
        hu->rx_head = next;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       读取串口接收的数据（查询接收）
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 参数说明       *dat            接收数据的地址
// 返回参数       uint8           1：接收成功   0：未接收到数据
// 使用示例       uint8 dat; uart_query_byte(UART_1, &dat);       // 接收 UART_1 数据  存在在 dat 变量里
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 uart_query_byte(uart_index_enum uartn, uint8 *dat)
{
    uint8 idx = uart_idx(uartn);
    uart_handle_t *hu = &g_uart[idx];
    if(hu->rx_head == hu->rx_tail) return 0;   /* 空 */
    *dat = hu->rx_buf[hu->rx_tail];
    hu->rx_tail = (hu->rx_tail + 1) % UART_RX_BUF_SIZE;
    return 1;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       读取串口接收的数据（whlie等待）
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 参数说明       *dat            接收数据的地址
// 返回参数       uint8           接收的数据
// 使用示例       uint8 dat = uart_read_byte(UART_1);             // 接收 UART_1 数据  存在在 dat 变量里
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 uart_read_byte(uart_index_enum uartn)
{
    uint8 dat;
    while(!uart_query_byte(uartn, &dat));
    return dat;
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      串口初始化
//  参数说明      uartn           串口模块号(UART_0,UART_1,UART_2,UART_3)
//  参数说明      baud            串口波特率
//  参数说明      tx_pin          串口发送引脚
//  参数说明      rx_pin          串口接收引脚
//  返回参数      uint32          实际波特率
//  使用示例      uart_init(UART_0,115200,UART0_TX_P14_0,UART0_RX_P14_1);       // 初始化串口0 波特率115200 发送引脚使用P14_0 接收引脚使用P14_1
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void uart_init(uart_index_enum uartn, uint32 baud,
               uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin)
{
    uint8 idx = uart_idx(uartn);
    uart_handle_t *hu = &g_uart[idx];
    USART_TypeDef *u = hu->uartx;

    GPIO_InitTypeDef  gpio_init_struct;
    GPIO_TypeDef *tx_port, *rx_port;
    uint16 tx_pin_src, rx_pin_src;

	  uint8 preempt_pri;

#if UART_RX_USE_DMA
    uint8_t dma_preempt_pri;  // 仅在DMA模式下定义
#endif
	
    /* 1. 根据 uartn 取 TX/RX 端口及复用源 */
    if (uartn == UART_1)
    {
        tx_port = (tx_pin == UART1_TX_PA9)  ? GPIOA : GPIOB;
        rx_port = (rx_pin == UART1_RX_PA10) ? GPIOA : GPIOB;
        tx_pin_src = (tx_pin == UART1_TX_PA9)  ? GPIO_PinSource9  : GPIO_PinSource6;
        rx_pin_src = (rx_pin == UART1_RX_PA10) ? GPIO_PinSource10 : GPIO_PinSource7;
			  preempt_pri = UART1_NVIC_PREEMPT_PRIORITY;
#if UART_RX_USE_DMA			
        dma_preempt_pri = UART1_DMA_NVIC_PREEMPT_PRIORITY;
#endif			
    }
    else if (uartn == UART_2)
    {
        tx_port = GPIOA; rx_port = GPIOA;
        tx_pin_src = GPIO_PinSource2; rx_pin_src = GPIO_PinSource3;
        preempt_pri = UART2_NVIC_PREEMPT_PRIORITY;
#if UART_RX_USE_DMA				
        dma_preempt_pri = UART2_DMA_NVIC_PREEMPT_PRIORITY;
#endif				
    }
    else /* UART_3 */
    {
        tx_port = GPIOB; rx_port = GPIOB;
        tx_pin_src = GPIO_PinSource10; rx_pin_src = GPIO_PinSource11;
			  preempt_pri = UART3_NVIC_PREEMPT_PRIORITY;
#if UART_RX_USE_DMA						
        dma_preempt_pri = UART3_DMA_NVIC_PREEMPT_PRIORITY;
#endif			
    }

    /* 2. 开时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    if (uartn == UART_1) RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    else if (uartn == UART_2) RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    else                      RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

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

    /* 3. 配 USART 基本参数 */
    USART_InitTypeDef us;
    USART_StructInit(&us);
    us.USART_BaudRate            = baud;
    us.USART_WordLength          = USART_WordLength_8b;
    us.USART_StopBits            = USART_StopBits_1;
    us.USART_Parity              = USART_Parity_No;
    us.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    us.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(u, &us);

		/* 4. 配 NVIC */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // USART NVIC 配置
    NVIC_InitTypeDef nv;
    nv.NVIC_IRQChannel = uart_get_irq_channel(u);
    nv.NVIC_IRQChannelPreemptionPriority = preempt_pri;
    nv.NVIC_IRQChannelSubPriority = 0;
    nv.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nv);

#if UART_RX_USE_DMA
    /* 5. RX-DMA 配置：循环搬运到 rx_buf，半完成/完成中断里推 FIFO */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(hu->rx_dma_ch);
    DMA_InitTypeDef dma;
    dma.DMA_PeripheralBaseAddr = (uint32)&u->DR;
    dma.DMA_MemoryBaseAddr     = (uint32)hu->rx_buf;
    dma.DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize         = UART_RX_BUF_SIZE;
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode               = DMA_Mode_Circular;
    dma.DMA_Priority           = DMA_Priority_Medium;
    dma.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(hu->rx_dma_ch, &dma);

    USART_DMACmd(u, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(hu->rx_dma_ch, ENABLE);

    /* 开 DMA 中断，半/全完成都把“新数据”推 FIFO */
    DMA_ITConfig(hu->rx_dma_ch, DMA_IT_TC | DMA_IT_HT, ENABLE);
      /* 根据DMA通道配置中断向量 */
		nv.NVIC_IRQChannel = uart_get_dma_irq_channel(idx);
    nv.NVIC_IRQChannelPreemptionPriority = dma_preempt_pri;  // DMA优先级高于USART
    nv.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nv);
#else
    /* 普通中断接收 */
    USART_ITConfig(u, USART_IT_RXNE, ENABLE);
#endif

#if UART_TX_USE_DMA
      /* 7. TX-DMA 配置准备（只配置，不启动） */
      RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
      DMA_DeInit(hu->tx_dma_ch);
      DMA_InitTypeDef tx_dma;
      tx_dma.DMA_PeripheralBaseAddr = (uint32)&u->DR;
      tx_dma.DMA_MemoryBaseAddr     = 0;  /* 发送时再设置 */
      tx_dma.DMA_DIR                = DMA_DIR_PeripheralDST;
      tx_dma.DMA_BufferSize         = 0;  /* 发送时再设置 */
      tx_dma.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
      tx_dma.DMA_MemoryInc          = DMA_MemoryInc_Enable;
      tx_dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
      tx_dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
      tx_dma.DMA_Mode               = DMA_Mode_Normal;  /* 发送用正常模式 */
      tx_dma.DMA_Priority           = DMA_Priority_Medium;
      tx_dma.DMA_M2M                = DMA_M2M_Disable;
      DMA_Init(hu->tx_dma_ch, &tx_dma);
      
      USART_DMACmd(u, USART_DMAReq_Tx, ENABLE);
      /* TX DMA不开启中断，使用查询方式等待完成 */
#endif

    USART_Cmd(u, ENABLE);
}

/*-------------------- 中断服务 --------------------*/
  
#if !UART_RX_USE_DMA
  /* 非DMA接收模式：使用串口中断接收 */
  static void uart_rx_irq_handler(uart_index_enum uartn)
  {
      USART_TypeDef *u = g_uart[uart_idx(uartn)].uartx;
      if(USART_GetITStatus(u, USART_IT_RXNE) != RESET)
      {
          uint8 dat = USART_ReceiveData(u);
          uart_rx_push(uartn, dat);
      }
  }
  void USART1_IRQHandler(void){
	uart_rx_irq_handler(UART_1); 
#if DEBUG_UART_USE_INTERRUPT                        // 如果开启 debug 串口中断
        debug_interrupr_handler();                  // 调用 debug 串口接收处理函数 数据会被 debug 环形缓冲区读取
#endif                                              // 如果修改了 DEBUG_UART_INDEX 那这段代码需要放到对应的串口中断去		
	}
  void USART2_IRQHandler(void){ uart_rx_irq_handler(UART_2); }
  void USART3_IRQHandler(void){ uart_rx_irq_handler(UART_3); }
#else
  /* DMA接收模式：串口中断只处理错误 */
  void USART1_IRQHandler(void)
  {
      if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
          USART_ReceiveData(USART1); /* 清除中断标志 */
  }
  void USART2_IRQHandler(void)
  {
      if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
          USART_ReceiveData(USART2); /* 清除中断标志 */
  }
  void USART3_IRQHandler(void)
  {
      if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
          USART_ReceiveData(USART3); /* 清除中断标志 */
  }
#endif

  /*============================ DMA 中断服务（仅 RX-DMA 模式用到） ============================*/
#if UART_RX_USE_DMA
  static void uart_rx_dma_handler(uint8 idx)
  {
      uart_handle_t *hu = &g_uart[idx];
      uint16 tail = hu->rx_dma_ch->CNDTR;        /* 剩余未读数 */
      uint16 pos  = UART_RX_BUF_SIZE - tail;     /* 当前 DMA 写指针 */
      uint16 old  = (hu->rx_head) % UART_RX_BUF_SIZE;

      /* 把 [old, pos) 区间的新数据推入用户 FIFO */
      while(old != pos)
      {
          uart_rx_push((uart_index_enum)idx, hu->rx_buf[old]);
          old = (old + 1) % UART_RX_BUF_SIZE;
      }
      /* 清中断标志 - 使用查表方式 */
      DMA_ClearITPendingBit(g_dma_rx_tc_flag[idx]);
  }

  void DMA1_Channel5_IRQHandler(void){ uart_rx_dma_handler(0); }  /* UART1 RX - Channel 5 */
  void DMA1_Channel6_IRQHandler(void){ uart_rx_dma_handler(1); }  /* UART2 RX - Channel 6 */
  void DMA1_Channel3_IRQHandler(void){ uart_rx_dma_handler(2); }  /* UART3 RX - Channel 3 */
#endif
	
