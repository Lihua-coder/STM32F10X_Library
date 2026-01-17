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
#include "driver_can.h"

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      波特率→分频系数转换（内部调用）
// 参数说明      baud                目标波特率，单位 bps
// 返回参数      uint8_t             计算出的分频系数 1~1024
// 使用示例      uint8_t prescaler = can_baud2prescaler(500000);
// 备注信息      基于 36 MHz 时钟、16 倍采样计算，结果自动限幅
//-------------------------------------------------------------------------------------------------------------------
static uint8 can_baud2prescaler(uint32 baud)
{
    uint32 p = (36000000UL / 16) / baud;
    if (p < 1) p = 1;
    if (p > 1024) p = 1024;
    return (uint8)p;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      CAN GPIO 初始化（内部调用）
// 参数说明      tx                  TX 引脚选择  CAN1_TX_PA12 / CAN1_TX_PB9
// 参数说明      rx                  RX 引脚选择  CAN1_RX_PA11 / CAN1_RX_PB8
// 返回参数      void
// 使用示例      can_gpio_config(CAN1_TX_PA12, CAN1_RX_PA11);
// 备注信息      自动开启对应端口及 AFIO 时钟，PB8/PB9 自动重映射
//-------------------------------------------------------------------------------------------------------------------
static void can_gpio_config(can_tx_pin_enum tx, can_rx_pin_enum rx)
{
    GPIO_TypeDef *tx_port, *rx_port;
    uint16 tx_pin, rx_pin;

    /* 引脚 -> 端口/管脚 */
    if (tx == CAN1_TX_PA12) { tx_port = GPIOA; tx_pin = GPIO_Pin_12; }
    else                    { tx_port = GPIOB; tx_pin = GPIO_Pin_9;  }

    if (rx == CAN1_RX_PA11) { rx_port = GPIOA; rx_pin = GPIO_Pin_11; }
    else                    { rx_port = GPIOB; rx_pin = GPIO_Pin_8;  }

    /* 1. 重映射：PB8/PB9 需要 AFIO */
    if (tx == CAN1_TX_PB9 || rx == CAN1_RX_PB8)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
    }

    /* 2. TX 复用推挽 */
    GPIO_InitTypeDef gpio = {0};
    gpio.GPIO_Pin   = tx_pin;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(tx_port, &gpio);

    /* 3. RX 上拉输入 */
    gpio.GPIO_Pin  = rx_pin;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(rx_port, &gpio);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      NVIC 配置（CAN1 RX0 中断，内部调用）
// 参数说明      void
// 返回参数      void
// 使用示例      can_nvic_config();
// 备注信息      优先级组 2，RX0 中断抢占=1，响应优先级=0
//-------------------------------------------------------------------------------------------------------------------
static void can_nvic_config(void)
{
    NVIC_InitTypeDef nvic = {0};
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    nvic.NVIC_IRQChannel                   = USB_LP_CAN1_RX0_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      关闭全部硬件滤波器（FIFO0 关联，内部调用）
// 参数说明      void
// 返回参数      void
// 使用示例      can_filter_disable_all();
// 备注信息      循环关闭 0 号滤波器
//-------------------------------------------------------------------------------------------------------------------
static void can_filter_disable(void)
{
        CAN_FilterInitTypeDef f = {0};
        f.CAN_FilterNumber         = 0;
        f.CAN_FilterMode           = CAN_FilterMode_IdMask;
        f.CAN_FilterScale          = CAN_FilterScale_32bit;
        f.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
        f.CAN_FilterActivation     = DISABLE;
        CAN_FilterInit(&f);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      CAN 初始化（无滤波，FIFO0 接收）
// 参数说明      cann                选择 CAN 外设，仅支持 CAN_1
// 参数说明      can_mode            工作模式  CAN_Mode_Normal / CAN_Mode_LoopBack
// 参数说明      baud                目标波特率，单位 bps
// 参数说明      tx_pin              TX 引脚选择  CAN1_TX_PA12 / CAN1_TX_PB9
// 参数说明      rx_pin              RX 引脚选择  CAN1_RX_PA11 / CAN1_RX_PB8
// 返回参数      void
// 使用示例      can_init_no_filter(CAN_1, CAN_Mode_Normal, 500000,
//                                    CAN1_TX_PA12, CAN1_RX_PA11);
// 备注信息      接收用 FIFO0 + 中断，CPU 直接读
//-------------------------------------------------------------------------------------------------------------------
void can_init_no_filter(can_index_enum cann, uint8 can_mode, uint32 baud, can_tx_pin_enum tx_pin, can_rx_pin_enum rx_pin)
{
    if (cann != CAN_1) return;

    /* 1. 时钟 + GPIO */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    can_gpio_config(tx_pin, rx_pin);

    /* 2. CAN 工作参数 */
    CAN_DeInit(CAN1);
    CAN_InitTypeDef can = {0};
    CAN_StructInit(&can);
    can.CAN_TTCM  = DISABLE;
    can.CAN_ABOM  = ENABLE;
    can.CAN_AWUM  = DISABLE;
    can.CAN_NART  = DISABLE;
    can.CAN_RFLM  = DISABLE;
    can.CAN_TXFP  = ENABLE;
    can.CAN_Mode  = can_mode;
    can.CAN_SJW   = CAN_SJW_1tq;
    can.CAN_BS1   = CAN_BS1_8tq;
    can.CAN_BS2   = CAN_BS2_7tq;
    can.CAN_Prescaler = can_baud2prescaler(baud);
    CAN_Init(CAN1, &can);

    /* 3. 滤波 + 中断 */
    can_filter_disable();
    can_nvic_config();
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);

}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      发送帧（阻塞等待）
// 参数说明      cann                选择 CAN 外设，仅支持 CAN_1
// 参数说明      TxMessage           待发送帧结构体指针
// 返回参数      void             
// 使用示例      can_send(CAN_1, &tx_msg);
// 备注信息      
//-------------------------------------------------------------------------------------------------------------------
void can_send(can_index_enum cann, CanTxMsg *TxMessage)
{
    if (cann != CAN_1) return;

	uint8 TransmitMailbox = CAN_Transmit(CAN1, TxMessage);
	
	uint32 Timeout = 0;
	while (CAN_TransmitStatus(CAN1, TransmitMailbox) != CAN_TxStatus_Ok)
	{
		Timeout ++;
		if (Timeout > 100000)
		{
			break;
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      CAN1 FIFO0 接收中断
// 参数说明      void
// 返回参数      void
// 使用示例      中断入口，用户无需手动调用
// 备注信息      
//-------------------------------------------------------------------------------------------------------------------
CanRxMsg can_rxmsg;//接收信息组
uint8 can_rxflag;//接收完成标志位
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) == SET)
	{
		CAN_Receive(CAN1, CAN_FIFO0, &can_rxmsg);
		can_rxflag = 1;
	}
}
