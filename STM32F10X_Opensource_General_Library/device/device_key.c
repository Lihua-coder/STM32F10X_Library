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
#include "device_key.h"

static uint32           scanner_period;                     // 按键的扫描周期
static uint32           key_press_time[KEY_NUMBER];         // 按键信号持续时长
static key_state_enum   key_state[KEY_NUMBER];              // 按键状态
static const gpio_pin_enum key_index[KEY_NUMBER] = KEY_LIST;// 按键列表
static uint32 sys_tick_ms = 0;
static uint32 last_rel[KEY_NUMBER] = {
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF
};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     按键状态扫描
// 参数说明     void
// 返回参数     void
// 使用示例     key_scanner();
// 备注信息     这个函数放在中断中
//-------------------------------------------------------------------------------------------------------------------
void key_scanner(void)
{
    uint8 i;
    sys_tick_ms++;                                   
    for (i = 0; i < KEY_NUMBER; i++)
    {
        /* 1. 按键仍被按下 */
        if (KEY_RELEASE_LEVEL != gpio_get_level(key_index[i]))
        {
            key_press_time[i]++;

            if (key_press_time[i] >= KEY_LONG_PRESS_PERIOD / scanner_period)
                key_state[i] = KEY_LONG_PRESS;
        }
        else
        {
            /* 2. 按键已释放 */
            if (key_state[i] == KEY_LONG_PRESS)
            {
                /* 长按松开后直接清零，不再产生 SINGLE/DOUBLE */
                key_press_time[i] = 0;
                key_state[i]      = KEY_RELEASE;
                continue;
            }

            /* 有效短按区间：消抖结束 ~ 长按阈值 */
            if (key_press_time[i] >= KEY_MAX_SHOCK_PERIOD &&
                key_press_time[i] <  KEY_LONG_PRESS_PERIOD)
            {
                /* 与上一次松开时间差 ≤ 200 ms 判定为双击 */
                if ((sys_tick_ms - last_rel[i]) <= KEY_DOUBLE_PRESS_PERIOD / scanner_period)
                {
                    key_state[i] = KEY_DOUBLE_PRESS;
                    last_rel[i]  = 0;               // 清零，防止连续触发
                }
                else
                {
                    key_state[i] = KEY_SINGLE_PRESS;
                    last_rel[i]  = sys_tick_ms;
                }
            }
            else
            {
                key_state[i] = KEY_RELEASE;
            }

            key_press_time[i] = 0;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取按键状态
// 参数说明     key_n           按键索引
// 返回参数     key_state_enum  按键状态
// 使用示例     key_get_state(KEY_1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
key_state_enum key_get_state(key_index_enum key_n)
{
    return key_state[key_n];
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     清除指定按键状态
// 参数说明     key_n           按键索引
// 返回参数     void            无
// 使用示例     key_clear_state(KEY_1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void key_clear_state(key_index_enum key_n)
{
    key_state[key_n] = KEY_RELEASE;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     清除所有按键状态
// 参数说明     void            无
// 返回参数     void            无
// 使用示例     key_clear_all_state();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void key_clear_all_state(void)
{
    for (uint8 i = 0; i < KEY_NUMBER; i++)
        key_state[i] = KEY_RELEASE;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     按键初始化
// 参数说明     period          按键扫描周期 以毫秒为单位
// 返回参数     void
// 使用示例     key_init(10);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void key_init(uint32 period)
{
    zf_assert(period > 0);
    scanner_period = period;

    for (uint8 i = 0; i < KEY_NUMBER; i++)
    {
        gpio_init(key_index[i], GPI_PULL_UP, 1);
        key_state[i]      = KEY_RELEASE;
        key_press_time[i] = 0;
    }
}

