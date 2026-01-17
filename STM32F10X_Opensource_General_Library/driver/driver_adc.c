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
#include "driver_adc.h"

/*=============================  模块级静态变量  ==================================*/
static uint8 adc1_base_init_done = 0;        /* ADC1 基础部分只初始化一次 */
static uint8 adc1_init_flag[10]   = {0};       /* CH0~CH9 通道初始化标记，供 adc_init/adc_convert 共享 */
/*================================================================================*/
// 内部使用，用户无需关心
static uint8 get_adc1_idx(adc1_channel_enum ch)
{
    switch(ch)
    {
        case ADC1_CH0_PA0: return 0;
        case ADC1_CH1_PA1: return 1;
        case ADC1_CH2_PA2: return 2;
        case ADC1_CH3_PA3: return 3;
        case ADC1_CH4_PA4: return 4;
        case ADC1_CH5_PA5: return 5;
        case ADC1_CH6_PA6: return 6;
        case ADC1_CH7_PA7: return 7;
        case ADC1_CH8_PB0: return 8;
        case ADC1_CH9_PB1: return 9;
        default:           return 0xFF;   /* 非法通道 */
    }
}
//--------------------------------------------------------------------------------------------------------------------
// 函数名     adc1_init     
// 功能说明   初始化指定 ADC1 通道，并设定分辨率
// 参数说明   vadc_chn      选择 ADC1 通道（如 ADC1_CH0_PA0）
// 参数说明   resolution    选择分辨率（ADC_8BIT 或 ADC_12BIT）
// 返回参数   void
// 使用示例   adc_init(ADC1_CH0_PA0, ADC_12BIT);
// 备注信息   1. 通道级标记，保证同一通道只初始化一次
//            2. 分辨率/对齐方式每次调用都可刷新
//--------------------------------------------------------------------------------------------------------------------
void adc1_init(adc1_channel_enum vadc_chn, adc_resolution_enum resolution)
{
    uint8 idx = get_adc1_idx(vadc_chn);
    if(idx == 0xFF)  return;                 /* 非法通道，直接返回 */
    /*------------------ 1. 打开对应 GPIO 时钟并配置为模拟输入 ------------------*/
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_TypeDef*   gpio_port       = 0;
    uint16          gpio_pin        = 0;

    switch(vadc_chn)
    {
        case ADC1_CH0_PA0: gpio_port = GPIOA; gpio_pin = GPIO_Pin_0; break;
        case ADC1_CH1_PA1: gpio_port = GPIOA; gpio_pin = GPIO_Pin_1; break;
        case ADC1_CH2_PA2: gpio_port = GPIOA; gpio_pin = GPIO_Pin_2; break;
        case ADC1_CH3_PA3: gpio_port = GPIOA; gpio_pin = GPIO_Pin_3; break;
        case ADC1_CH4_PA4: gpio_port = GPIOA; gpio_pin = GPIO_Pin_4; break;
        case ADC1_CH5_PA5: gpio_port = GPIOA; gpio_pin = GPIO_Pin_5; break;
        case ADC1_CH6_PA6: gpio_port = GPIOA; gpio_pin = GPIO_Pin_6; break;
        case ADC1_CH7_PA7: gpio_port = GPIOA; gpio_pin = GPIO_Pin_7; break;
        case ADC1_CH8_PB0: gpio_port = GPIOB; gpio_pin = GPIO_Pin_0; break;
        case ADC1_CH9_PB1: gpio_port = GPIOB; gpio_pin = GPIO_Pin_1; break;
        default: return;
    }
    GPIO_InitStructure.GPIO_Pin  = gpio_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(gpio_port, &GPIO_InitStructure);

    /*------------------ 2. ADC1 基础部分（仅第一次） ------------------*/
    if(!adc1_base_init_done)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

        ADC_InitTypeDef ADC_InitStructure;
        ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
        ADC_InitStructure.ADC_ScanConvMode       = DISABLE;
        ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
        ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
        ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;
        ADC_InitStructure.ADC_NbrOfChannel       = 1;
        ADC_Init(ADC1, &ADC_InitStructure);

        RCC_ADCCLKConfig(RCC_PCLK2_Div6);   /* 12MHz */

        /* 自校准 */
        ADC_ResetCalibration(ADC1);
        while(ADC_GetResetCalibrationStatus(ADC1));
        ADC_StartCalibration(ADC1);
        while(ADC_GetCalibrationStatus(ADC1));

        adc1_base_init_done = 1;
    }

    /*------------------ 3. 分辨率/对齐方式（每次调用都可刷新） ------------------*/
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode       = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_NbrOfChannel       = 1;

    if(resolution == ADC_8BIT)
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;   /* 8 位取高 */
    else
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  /* 12 位右对齐 */

    ADC_Init(ADC1, &ADC_InitStructure);

    /*------------------ 4. 打开 ADC ------------------*/
    ADC_Cmd(ADC1, ENABLE);

    /*------------------ 5. 打标记，防止重复初始化 ------------------*/
    adc1_init_flag[idx] = 1;
}
//--------------------------------------------------------------------------------------------------------------------
// 函数名     adc1_convert  
// 功能说明   执行一次 ADC 转换（软件触发，单次模式）
// 参数说明   vadc_chn      要采样的 ADC1 通道（如 ADC1_CH0_PA0）
// 返回参数   uint16        转换结果
//                         - 若初始化时分辨率=ADC_12BIT，则返回 0~4095
//                         - 若分辨率=ADC_8BIT，则返回高 8 位（0~255）
// 使用示例   uint16 val = adc_convert(ADC1_CH0_PA0);
// 备注信息   如果该通道尚未初始化，则自动调用 adc_init(ch, ADC_12BIT)
//--------------------------------------------------------------------------------------------------------------------
uint16 adc1_convert(adc1_channel_enum vadc_chn)
{
    /* 如果还没初始化过，默认给 12-bit 初始化一次 */
    uint8 idx = get_adc1_idx(vadc_chn);
    if(idx == 0xFF)  return 0;                 /* 非法通道，直接返回 */

    if(idx != 0xFF && !adc1_init_flag[idx])
    {
        adc1_init(vadc_chn, ADC_12BIT);   /* 默认 12-bit */
        adc1_init_flag[idx] = 1;
    }
    /* 采样时间（默认 55.5 周期）*/      
    ADC_RegularChannelConfig(ADC1, idx, 1, ADC_SampleTime_55Cycles5);
    /* 软件触发转换 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    /* 等待转换结束，简单超时保护 */
    uint32_t timeout = 0xFFFF;
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
    {
        if(--timeout == 0) return 0;     /* 超时容错 */
    }

    /* 读取转换结果 */
    uint16_t result = ADC_GetConversionValue(ADC1);

    /* 如果之前设的是 8-bit（左对齐），则返回高 8 位 */
    if((ADC1->CR2 & ADC_CR2_ALIGN) == 0)   /* 0=右对齐(12bit)，非0=左对齐(8bit) */
        return result;                     /* 12-bit 右对齐，直接返回 */
    else
        return (result >> 8) & 0xFF;       /* 8-bit 模式，取高 8 位 */
}
//--------------------------------------------------------------------------------------------------------------------
// 函数名     adc1_mean_filter_convert		
// 功能说明   均值滤波 ADC 采样（去掉最大/最小后平均）
// 参数说明   vadc_chn  要采样的 ADC1 通道（如 ADC1_CH0_PA0）
// 参数说明   count     采样次数（建议 4~30，用户自行权衡速度与平稳度）
// 返回参数   uint16    滤波后的结果
// 使用示例   uint16 val = adc1_mean_filter_convert(ADC1_CH0_PA0, 16);
// 备注信息   若 count<3 则直接求平均；内部自动调用 adc_convert()
//--------------------------------------------------------------------------------------------------------------------
uint16 adc1_mean_filter_convert(adc1_channel_enum vadc_chn, uint8 count)
{
    if(count == 0) return 0;

    uint32 sum = 0;
    uint16 min_val = 0xFFFF;
    uint16 max_val = 0;
    for(uint8 i = 0; i < count; i++)
    {
        uint16 val = adc1_convert(vadc_chn);   /* 单次转换 */
        sum += val;
        if(val < min_val) min_val = val;
        if(val > max_val) max_val = val;
    }

    if(count >= 3)
        sum -= (min_val + max_val);           /* 去掉最大、最小 */

    return (uint16)(sum / (count >= 3 ? (count - 2) : count));
}
//--------------------------------------------------------------------------------------------------------------------
// 函数名     adc1_dma_init     
// 功能说明   一次性初始化 1~16 路 ADC1 通道，并配置 DMA1-CH1 循环搬运
// 参数说明   vadc_chn[]      		通道号数组，长度 = count
// 参数说明   resolution    			选择分辨率（ADC_8BIT 或 ADC_12BIT）
// 参数说明   rank[]   						选择adc1排名(1~16)，长度 = count
// 参数说明   count    						设置adc1数量	from 1 to 16
// 参数说明   destination_addr    设置目的地址
// 返回参数   void
// 使用示例   adc1_dma1_init(ch_list, ADC_12BIT, rank, 3, (uint32_t)adc_destination);
// 备注信息   adc1_channel_enum  ch_list[3] = { ADC1_CH0_PA0, ADC1_CH1_PA1, ADC1_CH2_PA2 };
//						uint8  rank[3] = { 1,2,3 };
//						uint16 adc_destination[3];
//--------------------------------------------------------------------------------------------------------------------
void adc1_dma1_init(const adc1_channel_enum  vadc_chn[], adc_resolution_enum resolution,
	const uint8 rank[], uint8 count, uint32 destination_addr){
			
    GPIO_TypeDef *port;
    uint16 pin;
    GPIO_InitTypeDef gpio;

    if(!count || count>16) return;

    /* 1. GPIO 时钟 + 模拟输入 */
    for(uint8 i=0;i<count;i++)
    {
        port=0; pin=0;
        switch(vadc_chn[i])
        {
            case ADC1_CH0_PA0: port=GPIOA; pin=GPIO_Pin_0; break;
            case ADC1_CH1_PA1: port=GPIOA; pin=GPIO_Pin_1; break;
            case ADC1_CH2_PA2: port=GPIOA; pin=GPIO_Pin_2; break;
            case ADC1_CH3_PA3: port=GPIOA; pin=GPIO_Pin_3; break;
            case ADC1_CH4_PA4: port=GPIOA; pin=GPIO_Pin_4; break;
            case ADC1_CH5_PA5: port=GPIOA; pin=GPIO_Pin_5; break;
            case ADC1_CH6_PA6: port=GPIOA; pin=GPIO_Pin_6; break;
            case ADC1_CH7_PA7: port=GPIOA; pin=GPIO_Pin_7; break;
            case ADC1_CH8_PB0: port=GPIOB; pin=GPIO_Pin_0; break;
            case ADC1_CH9_PB1: port=GPIOB; pin=GPIO_Pin_1; break;
            default: return;
        }
        RCC_APB2PeriphClockCmd(port==GPIOA ? RCC_APB2Periph_GPIOA : RCC_APB2Periph_GPIOB, ENABLE);

        gpio.GPIO_Pin  = pin;        
        gpio.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(port,&gpio);
    }

    /* 2. ADC1 基础部分（仅一次） */
    if(!adc1_base_init_done)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
        RCC_ADCCLKConfig(RCC_PCLK2_Div6);
        ADC_DeInit(ADC1);

        ADC_InitTypeDef adc;
        ADC_StructInit(&adc);
        adc.ADC_Mode               = ADC_Mode_Independent;
        adc.ADC_ScanConvMode       = ENABLE;
        adc.ADC_ContinuousConvMode = ENABLE;//连续扫描
        adc.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
        adc.ADC_DataAlign          = ADC_DataAlign_Right;
        adc.ADC_NbrOfChannel       = count;
        ADC_Init(ADC1,&adc);

        ADC_ResetCalibration(ADC1);
        while(ADC_GetResetCalibrationStatus(ADC1));
        ADC_StartCalibration(ADC1);
        while(ADC_GetCalibrationStatus(ADC1));
        adc1_base_init_done=1;
    }

    /* 3. 写入转换序列 */
    for(uint8 i=0;i<count;i++)
    {
        uint8 ch_idx = get_adc1_idx(vadc_chn[i]);
        if(ch_idx==0xFF) return;
        ADC_RegularChannelConfig(ADC1,ch_idx,rank[i],ADC_SampleTime_55Cycles5);
    }

    /* 4. 分辨率 */
    if(resolution==ADC_8BIT)
        ADC1->CR2 |=  ADC_CR2_ALIGN;   /* 左对齐 */
    else
        ADC1->CR2 &=~ADC_CR2_ALIGN;   /* 右对齐 */

    /* 5. DMA1-CH1 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
    DMA_DeInit(DMA1_Channel1);

    DMA_InitTypeDef dma;
    DMA_StructInit(&dma);
    dma.DMA_PeripheralBaseAddr = (uint32)&ADC1->DR;
    dma.DMA_MemoryBaseAddr     = destination_addr;
    dma.DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize         = (uint16)count;
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;//不自增
    dma.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dma.DMA_MemoryDataSize     = DMA_PeripheralDataSize_HalfWord;
    dma.DMA_Mode               = DMA_Mode_Circular;//循环模式
    dma.DMA_Priority           = DMA_Priority_High;//优先级高
    dma.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1,&dma);

    /* 6. 启动 */
    ADC_DMACmd(ADC1,ENABLE);
    DMA_Cmd(DMA1_Channel1,ENABLE);
    ADC_Cmd(ADC1,ENABLE);
    ADC_SoftwareStartConvCmd(ADC1,ENABLE);
}

