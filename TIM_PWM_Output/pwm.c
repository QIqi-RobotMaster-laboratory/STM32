/**
 * @file pwm.c
 * @author yuanluochen
 * @brief stm32f407,PWM输出，定时器输出比较，采样TIM14输出通道1(TIM14_CH1)引脚GPIOF9输出PWM波，PWM模式1，有效电平为低电平
 * @version 0.1
 * @date 2022-09-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//PWM输出 - 定时器输出比较
/**
 * 定时器PWM输出工作原理
 * 
 * 在我们学习完定时器中断后，我们知道定时器时钟源进入定时器时基单元，
 * 经过PSC分频,每过一个周期，时基单元 CNT值更改一次，直到到达ARR时定时器触发更新中断，CNT数值重置
 * 而实现PWM波的输出则需要在CNT加载到ARR之前加入一个CCR数值，输出比较输出PWM波
 * 
 */

//PWM的工作过程(通道x为例)
//输出模式控制器比较CNT与CCRx的值，根据 CCMR1:OC1M[2:0]设置PWM模式，根据以上三者判断何时输出有效电平，
//进入TIMx_CCER判读输出/捕获极性若为0，则为高电平有效，若为1输出控制器输出信号进入一个非门，低电平有效
//最后配置CCER的CC1E为配置输出/捕获是否使能。

/**
 * 寄存器
 * CCRx(x = 1,2,3,4):捕获比较(值)寄存器(x = 1,2,3,4):设置比较值。
 * CCMRx:OCxM[2:0]位：对于PWM方式下，用于设置PWM模式1[110]或PWM模式2[111]
 * PWM模式1，CNT寄存器计数时，TIMx_CNT < TIMx_CCR1时通道1为有效电平，TIMx_CNT > TIMx_CCR1通道1为无效电平
 * PWM模式2，CNT寄存器计数时，TIMx_CNT > TIMx_CCR1时通道1为有效电平，TIMx_CNT > TIMx_CCR1通道1为无效电平
 * CCER :CC1P位:输出/捕获1输出极性。0：高电平有效，1：低电平有效。
 * CCER :CC1E位:输出/捕获1输出使能。0：关闭，1：打开
 */

/**
 * PWM模式
 * 
 * 脉冲宽度调制模式可以产生一个由TIMx_ARR寄存器确定频率、由TIM_CCRx寄存器确定占空比的信号。
 * PWM信号的频率或周期由ARR决定，占空比由CCRx决定
 * 
 * PWM模式1，计数器的值 < CCRx的值 有效
 * PWM模式2，计数器的值 > CCRx的值 有效
 * 
 * 必须设置TIMx_CCMRx寄存器OCxPE位以使能相应的预装载寄存器
 * 最后还要设置TIMx_CR1寄存器的ARPE位，使能自动重装载的预装载寄存器
 * 
 * ARPE = 1，ARR立即生效；ARPE = 0，ARR下个比较周期生效
 * 
 */

#include "pwm.h"

//PWM输出

/*************************************************************************************/

/**
 * 库函数配置PWM输出
 * 
 * 1.使能定时器14和相关IO口时钟
 *   使能定时器14时钟
 *   使能GPIOF时钟
 * 2.初始化IO口为复用功能输出
 * 3.GPIOF9复用映射到定时器14
 * 4.初始化定时器：ARR,PSC等
 * 5.初始化输出比较参数
 * 6.使能预装载寄存器
 * 7.使能自动重装载的预装载寄存器
 * 8.使能定时器
 * 9.不断改变比较值CCRx,达到不同的占空比效果
 * 最后一步调用 TIM_SetCompare1() 更改CCRx来调整占空比
 * 
 */

//该函数配置 TIM14 TIM14_CH1 PF9输出PWM波

//GPIO口一开始在AHB1总线，但是它可以通过复用功能接到其他时钟线

void TIM_PWM_Init(uint16_t arr, uint16_t psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    

    //使能定时器时钟，TIM14挂载在APB1总线，所以使能APB1TIM14的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
    //使能定时器相关IO口时钟，TIM14_CH1 IO口为PF9所以使能GPIOF的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    //初始化PF9并将其复用为TIM14
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//配置GPIO模式为复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//GPIO口满速
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    //GPIOF9复用为定时器14
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);

    //配置定时器 初始化定时器14
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc; //定时器分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInitStructure.TIM_Period = arr; //自动重装载值
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM14, &TIM_TimeBaseInitStructure);

    //定时器通道1初始化，输出模式为PWM模式1，极性为低，在CNT < CCRx为有效电平，有效电平为低电平。
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//模式为PWM1，在该模式下计数值CNT < CCRx为有效电平
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//极性为低，即有效电平为低电平
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputNState_Enable;//输出使能
    TIM_OC1Init(TIM14, &TIM_OCInitStructure);

    //使能TIM14通道1的预装载使能
    TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);

    //使能ARR的预装载使能
    TIM_ARRPreloadConfig(TIM14, ENABLE);
    
    //使能定时器 14
    TIM_Cmd(TIM14, ENABLE);
}


/**************************************************************************************/
