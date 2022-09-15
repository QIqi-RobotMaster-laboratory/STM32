/**
 * @file input_capture.c
 * @author yuanluochen
 * @brief 定时器输入捕获，基于STM32f407开发板，配置TIM5_CH1 引脚为PA0
 * @version 0.1
 * @date 2022-09-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//通用定时器输入捕获
//定时器各通道独立
//通过检测TIMx_CHx上相应通道上(对应的映射引脚)的的边沿信号，设置好要检测上升沿还是下降沿，在边沿信号发生跳变(比如上升沿/下降沿)的时候，
//检测到需要的电平跳变信号的时候，将当前定时器的值(TIMx_CNT)存放到对应的捕获比较寄存器(TIMx_CCRx)里面，完成了一次捕获，

/*
 * 由上侧说明可得，如果我们想捕获高电平信号宽度。
 * 那么就要首先设置上升沿捕获，捕获到上升沿将定时器计数值(TIMx_CNT)保存下来，
 * 然后设置下降沿捕获，捕获到下降沿时，将定时器的值(TIMx_CNT)保存下来。
 * 最后将保存下来的定时器数值相减，这样就可以得到高电平信号宽度
*/

/*
 * STM32输入捕获工作过程
 * 定时器通道对应的引脚，向定时器输入外部信号，经过一个滤波器，使信号变得更加平稳，
 * 经历过滤器过滤过的信号进入边沿检测器，检测设定好的边沿信号，
 * 检测到的边沿信号经历选择器，后进入管理定时器的通道选择器(该选择器负责管理，该定时器所有通道)，输出信号
 * 输出的信号，经历一个分频器输出数值，该分频器用于检测几个上升沿触发一次
*/

/**
 * 通用定时器设置输入捕获
 *  
 * 1.设置输入捕获滤波器
 * 定时器输入频率fCK_INT经过一个倍频器，倍频大小由寄存器TIMx_CRx的CKD[1:0]来设置，倍频系数为2^CKD，当CKD[1:0]为11时，倍频系数保留，产生fDTS，所有当CKD设置为0时fDTS = fCK_INT，
 * 假设IC1F[3:0] = 0011，并设置IC1映射到通道1上，且为上升沿触发，那么捕获到上升沿的时候，再以fCK_INT的频率，连续采样8次通道1的电平，如果都是高电平，则说明是有效触发，就会触发输入捕获中断。
 * 
 * 2.设置输入捕获极性
 * 相关寄存器
 * CC1P：输入/捕获1输出极性
 * CC1通道配置为输出
 * 0：OC1高电平有效
 * 1：OC1低电平有效
 * CC1通道配置为输入
 * 该位选择IC1还是IC1的反相信号作为触发或捕获信号
 * 0：不反相：捕获发生在IC1的上升沿：当用作外部触发器时，IC1不反相
 * 1：反向：捕获发生在IC1的下降沿：当用作外部触发器时，IC1反相
 * 
 * 3.设置捕获映射通道
 * 寄存器CC1S[1:0]:通道1，捕获/比较选择(Capture/Compare 1 selection)
 * 定义通道的方向(输入/输出),及输入脚选择
 * 00：CC1通道被配置为输出
 * 01：CC1通道被配置为输入，IC1映射到TI1上
 * 10：CC1通道被配置为输入，IC1映射到TI2上
 * 11：CC1通道被配置为输入，IC1映射到TRC上。此模式经工作在内部触发器输入被选中时
 * 
 * 4.设置输入捕获分频器
 * 
 * 5.捕获到有效的信号可以开启中断
 */


#include "input_capture.h"

//输入捕获

/*******************************************************************************************************/

/**
 * 检测高电平的持续时间
 * 
 * 1.定义一个8位的变量，详情如下
 * ---------------------------------------------------------
 * |   bit 7   |     bit 6     |         bit 5 ~ 0         |
 * ---------------------------------------------------------
 * |捕获完成标志|捕获到高电平标志|捕获到高电平后定时器溢出的时间|
 * ---------------------------------------------------------
 * 变量位7标志捕获完成，位6标志捕获到高电平，如果捕获到高电平bit6设置为1，如果捕获到低电平，就会检测bit6是否为1，为1，则捕获完成标志置1
 * 如果捕获到高电平后，定时器32为计数器溢出发生更新事件，则bit5~0数值加1
 * 所以高电平时间 = [5:0] * 计数器从0计数到最大值的时间  + 前后时间差值
 * 
 * 
 */

/**
 * 库函数配置输入捕获
 * 
 * 1.初始化定时器和通道对应的IO口时钟
 * 2.初始化IO口，模式为复用
 * 3.设置引脚复用映射
 * 4.初始化定时器ARR,PSC
 * 5.初始化输入捕获通道
 * 6.开启捕获中断
 * 7.使能定时器
 * 8.编写中断服务函数
 * 
 */

//抢占优先级
#define NVIC_PREEMPTIONPRIORITY 2
//子优先级
#define NVIC_SUBPRIORITY 0

void TIM_Input_Capture(uint16_t psc, uint32_t arr)
{
    //GPIO初始化结构体
    GPIO_InitTypeDef GPIO_InitStructure;
    //定时器初始化结构体
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    //定时器输入捕获结构体
    TIM_ICInitTypeDef TIM_ICInitStructure;
    //NVIC配置结构体
    NVIC_InitTypeDef NVIC_InitStructure;

    //使能TIM5定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    //初始化PA0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//初始化为复用模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//速度为100MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //PA0复用为TIM5
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM5);

    //配置TIM5的参数
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//配置定时器时结构体的这个成员就这个数
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStructure);

    //初始化 TIM5 通道1 输入捕获参数
    TIM_ICInitStructure.TIM_ICFilter = 0x00; //配置输入滤波器
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;//配置输入端IC1
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//令输入端IC1映射到TI1上
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;//配置滤波器，不分频
    TIM_ICInit(TIM5, &TIM_ICInitStructure);

    //配置定时器中断，允许更新中断，允许CC1TE捕获中断，允许更新中断原因，允许定时器数值溢出。
    TIM_ITConfig(TIM5, TIM_IT_Update | TIM_IT_CC1, ENABLE);

    //使能定时器5
    TIM_Cmd(TIM5, ENABLE);

    //配置中断NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEMPTIONPRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SUBPRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
    /*需要在主函数配置中断优先级分组*/
}


/**
 * 
 * 高电平时间为 = ((TIM5CH1_CAPTURE_STA & 0x3F) * ((PSC - 1) / CK_INT) * ARR) + TIM5CH1_CAPTURE_VAL * ((PSC -1) / CK_INT)
 * 
 */

static uint8_t TIM5CH1_CAPTURE_STA = 0;//输入捕获状态，八位，详解如下
static uint32_t TIM5CH1_CAPTURE_VAL = 0;//捕获完成后的定时器数值
 /*
 * ---------------------------------------------------------
 * |   bit 7   |     bit 6     |         bit 5 ~ 0         |
 * ---------------------------------------------------------
 * |捕获完成标志|捕获到高电平标志|捕获到高电平后定时器溢出的时间|
 * ---------------------------------------------------------
 */
//定时器5中断服务函数
void TIM5_IRQHandler(void)
{
   if(TIM_GetITStatus(TIM5, TIM_IT_CC1) == SET)//TIM5通道1发生了一次捕获事件
    {
        //如果上一次捕获到一个上升沿，则本次捕获到一个下降沿
        if(TIM5CH1_CAPTURE_STA & 0x40)//已经捕获到一次上升沿，则本次为下降沿
        {
            TIM5CH1_CAPTURE_STA |= 0x80; //标记捕获完成
            TIM5CH1_CAPTURE_VAL = TIM_GetCapture1(TIM5);//获取当前的捕获值
            //捕获完成后重新设置为上升沿捕获
            TIM_OC1PolarityConfig(TIM5, TIM_ICPolarity_Rising);
        }
        else//还未捕获到上升沿，则本次捕获定为上升沿
        { 
            //TIM5CH1_CAPTURE_VAL数值清空
            TIM5CH1_CAPTURE_VAL = 0;
            //TIM5CH1_CAPTURE_STA bit 5 ~ 0数值重置，
            TIM5CH1_CAPTURE_STA = 0;
            //标记捕获到高电平 bit6设置为1
            TIM5CH1_CAPTURE_STA |= 0x40;
            
            //将TIM5数值重置
            //如果要重置定时器5的计数值，那么需要先关闭定时器5，再重置定时器5的数值
            TIM_Cmd(TIM5, DISABLE);//关闭定时器5
            //计数值重置
            TIM_SetCounter(TIM5, 0);
            
            //配置定时器5为下降沿捕获
            TIM_OC1PolarityConfig(TIM5, TIM_ICPolarity_Falling);
            //重新使能定时器5
            TIM_Cmd(TIM5, ENABLE);
        }
    }
    //如果输入捕获状态为还未成功捕获,此时可能在捕获玩上升沿后，正在捕获下降沿
    if(TIM5CH1_CAPTURE_STA & 0x80 == 0)//即bit7为0，捕获未完成
    {
        //判读是否定时器溢出，如果溢出，bit 5 ~ 0 数值加一
        if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)//定时器发生更新中断数值溢出
        {
            //判断是否处在捕获低电平的过程中，如果处在捕获低电平的过程中， bit 5 ~ 0数值要加1
            if(TIM5CH1_CAPTURE_STA & 0x40)//已经捕获到高电平，定时器bit 5 ~ 0 位加1
            {
                //判断bit 5 ~ 0 是否已满，如果满了，为防止破坏变量bit6或bit7的数值，则标记成功捕获一次
                if((TIM5CH1_CAPTURE_STA & 0x3F) == 0x3F)//bit 5 ~ 0 已满
                {
                    TIM5CH1_CAPTURE_STA |= 0x80;//标记捕获完成
                    TIM5CH1_CAPTURE_VAL = 0xFFFFFFFF;//记录定时器数值为满
                    /**
                     * 
                     * 设置为 0XFFFFFFFF 是由于假定其ARR为0XFFFFFFFF
                     */
                }
                else
                {
                    TIM5CH1_CAPTURE_STA++;//数值溢出一次bit 5 ~ 0 自增；
                }
            }
        }
    }

    //清除中断标志位
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC1 | TIM_IT_Update);
}




/*******************************************************************************************************/
