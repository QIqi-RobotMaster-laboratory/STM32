/**
 * @file tim.c
 * @author yuanluochen
 * @brief STM32f407单片机，通用定时器中断，采用向上计数模式
 * @version 0.1
 * @date 2022-09-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//配置定时器，首先选择时钟输入源，一般选择内部时钟(CK_INT),为定时器时钟源。
/**
 * 计时器时钟可选择
 * 1.内部时钟CK_INT
 * 2.外部时钟模式1:外部输入脚(TIx)
 * 3.外部时钟模式2:外部触发输入(ETR)，(仅适用TIM2,3,4)
 * 4.内部触发输入(ITRx):使用一个定时器作为另一个定时器的预分频器，如可以配置一个定时器Timer1而作为另一个定时器Timer2的预分频器
*/
//配置定时器时基单元，首先配置PSC(预分频系数)，对时钟输入源进行分频。
//通用定时器内部时钟源来自APB1总线经过倍频(*1 / *2)产生CK_INT，CK_INT又生成CK_PSC，CK_PSC经过分频器(PSC)，生成CK_CNT。
//如果APB1总线分频系数为1，那么APB1变为定时器CK_INT的倍频系数为1，如果APB1分频系数不是1，为其他例如2、3、4……。那么APB1变为CK_INT的倍频系数为2。
/**
 * stm32f407 时钟
 * SYSCLK = 168MHz
 * AHB时钟 = 168MHz
 * APB1时钟 = SYSCLK/4 = 42MHz
 * APB2时钟 = SYSCLK/2 = 84MHz
 * APB1的分频系数 = AHB/APB1时钟 = 4 > 1
 * 所以CK_INT = APB1 * 2 = 84MHz
 * 所以CK_CNT = 84MHz / PSC
*/
/**
 * 计数器模式
 * 1.向上计数模式: 计数器从0计数自动加载到ARR，产生一个更新中断归零。
 * 2.向下计数模式：计数器从ARR计数到0，产生更新中断，重新回到ARR
 * 中央对齐模式：计数器从0到ARR，再从ARR到0，重复以上过程
*/
/**
 * 相关寄存器
 * 
 * 1. 计数器当前值寄存器 TIMx_CNT 16位 存储计时器当前值
 * 2. 预分频寄存器TIMx_PSC 16位 对定时器时钟进行分频
 * 3. 自动重装载寄存器 TIMx_ARR 16位
 * 
 * 4. 控制寄存器1 TIMx_CR1 16位 
 * 寄存器位4，DIR(Direction)位控制计数器计数方式，置0 向上计数；置1 向下计数
 * 寄存器位0，CEN使能计时器，置0 禁止计时器；置1 使能计数器
 * 5. DMA中断使能寄存器 TIMx_DIER 16位
 * 位5，保留，始终为0
 * 位4，CC4IE 允许捕获/比较4中断(Capture/Compare4 interrupt enable)
 * 位3，CC4IE 允许捕获/比较3中断(Capture/Compare3 interrupt enable)
 * 位2，CC4IE 允许捕获/比较2中断(Capture/Compare2 interrupt enable)
 * 位1，CC4IE 允许捕获/比较1中断(Capture/Compare1 interrupt enable)
 * 位0，UIE 允许更新中断（Update interrupt enable)
 */

/**
 * 配置定时器中断步骤
 * 
 * 1.选择时钟
 * 2.设置预分频系数 PSC 自动重装载值 ARR
 * 3.使能定时器
 * 4.配置中断服务函数
 * 
 */

#include "tim.h"

/*定时器中断*/
/**************************************************************************/

/**
 * 库函数定时器中断实现步骤
 * 
 * 1.使能定时器时钟
 * 2.初始化定时器，配置ARR，PSC，确定定时时长
 * 3.开启定时器中断，配置NVIC
 * 4.使能定时器
 * 5.编写中断服务函数 
 * 
 */

//定时器溢出时间 Tout(溢出时间) = (ARR + 1)* ((PSC + 1) / Tclk)

#define NVIC_PREEMPTIONPRIORITY 0x01//NVIC抢占优先级
#define NVIC_SUBPRIORITY 0x03       //NVIC子优先级

void TIM_init(uint16_t arr, uint16_t psc)
{
    //初始化结构体
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//定时器初始化结构体
    NVIC_InitTypeDef NVIC_InitStructure;//定时器更新中断结构体

    //使能定时器时钟，TIM3在APB1总线，其他定时器查看文件stm32f4xx_rcc.h
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//使能定时器3的时钟
    
    //初始化定时器,配置ARR，PSC 确定定时时长
    TIM_TimeBaseInitStructure.TIM_Period = arr; //自动重装载值
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc; //预分频系数
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //分频不需要配置，保存该配置
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    //开启定时器3更新中断 TIM_IT_Update
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    //配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //定时器3的NVIC
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEMPTIONPRIORITY;//抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SUBPRIORITY;//子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //使能定时器
    TIM_Cmd(TIM3, ENABLE);
}

//定时器中断服务函数
void TIM3_IRQHandler(void)
{
    //判断是否为TIM3更新中断，如果是，则为SET，如果不是，则为RESET
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        /*
         * 中断服务函数内容
         */

        //清楚中断标志位防止一直进入中断
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

/**************************************************************************/
