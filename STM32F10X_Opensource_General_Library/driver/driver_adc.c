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

static const adc1_channel_enum adc_index[ADC_NUMBER] = ADC_LIST;// ADC列表

#if ADC_USE_DMA
static volatile uint16 adc_dma_buf[ADC_NUMBER];   // DMA 循环缓冲区
#endif

static adc_resolution_enum adc_resolution = ADC_12BIT;//分辨率缓存


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     ADC 转换一次
// 参数说明     adc_n           选择 ADC 通道 (详见 driver_adc.h 中枚举 adc_index_enum 定义)
// 返回参数     uint16          转换的 ADC 值
// 使用示例     adc_convert(ADC_1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16 adc_convert(adc_index_enum adc_n)
{
    if(adc_n >= ADC_NUMBER) return 0;

#if ADC_USE_DMA
    /* DMA 模式：直接读循环缓冲区 */
    uint16 raw = adc_dma_buf[adc_n];
#else
    /* 普通模式：软件触发 → 等待 EOC → 读 DR */
    uint8  channel;
    switch(adc_index[adc_n])
    {
        case ADC1_CH0_PA0: channel = ADC_Channel_0; break;
        case ADC1_CH1_PA1: channel = ADC_Channel_1; break;
        case ADC1_CH2_PA2: channel = ADC_Channel_2; break;
        case ADC1_CH3_PA3: channel = ADC_Channel_3; break;
        default: channel = ADC_Channel_0;
    }
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    uint16 raw = ADC_GetConversionValue(ADC1);
#endif

    return (adc_resolution == ADC_8BIT) ? (raw >> 4) : raw;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     ADC 均值滤波转换
// 参数说明     adc_n           选择 ADC  (详见 driver_adc.h 中枚举 adc_index_enum 定义)
// 参数说明     count           均值滤波次数
// 返回参数     uint16          转换的 ADC 值
// 使用示例     adc_mean_filter_convert(ADC_1, 5);                        // 采集5次 然后返回平均值
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16 adc_mean_filter_convert(adc_index_enum adc_n, uint8 count)
{
    if(adc_n >= ADC_NUMBER || count == 0) return 0;
    uint32 sum = 0;
    for(uint8 i = 0; i < count; i++)
        sum += adc_convert(adc_n);
    uint16 ret = (uint16)(sum / count);
    return ret;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     ADC 初始化
// 参数说明     resolution      选择选择通道分辨率(如果同一个 ADC 模块初始化时设置了不同的分辨率 则最后一个初始化的分辨率生效)
// 返回参数     void
// 使用示例     adc_init(ADC_8BIT);                                // 初始化ADC 分辨率为8位
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void adc_init(adc_resolution_enum resolution)
{
	if (ADC_NUMBER == 0) return;
RCC_ADCCLKConfig(RCC_PCLK2_Div6);   // 把 ADCCLK 降到 12 MHz
    adc_resolution = resolution;

    /* 1. GPIO 模拟输入 */
    for(uint8 i = 0; i < ADC_NUMBER; i++)
    {
        gpio_pin_enum pin;
        switch(adc_index[i])
        {
            case ADC1_CH0_PA0: pin = PA0; break;
            case ADC1_CH1_PA1: pin = PA1; break;
            case ADC1_CH2_PA2: pin = PA2; break;
            case ADC1_CH3_PA3: pin = PA3; break;
            default: continue;
        }
        gpio_init(pin, GPI_AIN, 0);
    }

    /* 2. ADC 外设时钟 & 寄存器初始化 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    ADC_InitTypeDef adc;
    ADC_StructInit(&adc);
    adc.ADC_Mode               = ADC_Mode_Independent;
#if ADC_USE_DMA
    adc.ADC_ScanConvMode       = ENABLE;               /* 多通道扫描 */
    adc.ADC_ContinuousConvMode = ENABLE;               /* 连续转换 */
#else
    adc.ADC_ScanConvMode       = DISABLE;              /* 单通道，每次只转一路 */
    adc.ADC_ContinuousConvMode = DISABLE;              /* 单次转换，手动触发 */
#endif
    adc.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign          = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel       = ADC_NUMBER;
    ADC_Init(ADC1, &adc);

#if ADC_USE_DMA
    /* DMA 模式下，把 4 路通道按顺序排好 */
    for(uint8 i = 0; i < ADC_NUMBER; i++)
    {
        uint8 ch;
        switch(adc_index[i])
        {
            case ADC1_CH0_PA0: ch = ADC_Channel_0; break;
            case ADC1_CH1_PA1: ch = ADC_Channel_1; break;
            case ADC1_CH2_PA2: ch = ADC_Channel_2; break;
            case ADC1_CH3_PA3: ch = ADC_Channel_3; break;
            default: ch = ADC_Channel_0;
        }
        ADC_RegularChannelConfig(ADC1, ch, i + 1, ADC_SampleTime_55Cycles5);/* 采样时间（默认 55.5 周期）*/
    }
    ADC_DMACmd(ADC1, ENABLE);    /* 使能 ADC1 的 DMA 请求 */
		
    /* 3. DMA 初始化 */
    dma1_init(dma1_CH1,(uint32)&ADC1->DR,(uint32)adc_dma_buf,DMA_HalfWord,ADC_NUMBER,DMA_Priority_High, DMA_DIR_PeripheralSRC);
    DMA1_Channel1->CCR |= (1 << 5);   /* CIRC = 1，循环模式 */
    dma1_enable(dma1_CH1);
#endif

    /* 4. 启动 ADC */
    ADC_Cmd(ADC1, ENABLE);

    /* 5. 校准 */
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

#if ADC_USE_DMA
    /* 6. DMA 模式：启动一次即可，连续转换 + 循环 DMA 会自动刷新 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
#else
    /* 普通模式：不自动启动，每次 adc_convert() 里单独触发 */
#endif
}
