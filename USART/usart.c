/**
 * @file usart.c
 * @author yuanluochen
 * @brief 串口通信实验，基于stm32f407，配置PA9，PA10复用为USART1来实现串口通信
 * @version 0.1
 * @date 2022-09-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//USART通用同步异步收发器，与UART通用异步收发器，功能相差不大，USART相较于UART多了同步功能，但基本不用，所以两者差距不大

#include "usart.h"


/***********************************************************************************************************/

/**
 * 标准库配置串口
 * 
 * 1.串口时钟使能
 * .GPIO时钟使能
 * 2.引脚复用映射
 * 3.GPIO端口模式设置为复用
 * 4.串口参数初始化
 * 5.开启中断并且初始化NVIC(如果需要中断才需要这个步骤)
 * 6.使能串口
 * 7.编写中断处理函数
 * 8.串口数据收发
 * 9.串口传输状态获取 
 * 
 */

//抢占优先级设置位3
#define NVIC_PREEMPTIONPRIORITY 3
//响应优先级设置为3
#define NVIC_SUBPRIORITY 3

//接收缓冲数组，接收缓冲，最大USART_REC_LEN个字节，末字节为换行符
uint8_t USART_RX_BUF[USART_REC_LEN] = { 0 };
//接收状态标志
uint16_t USART_RX_STA = 0;
/* 
 * -------------------------------------------------------
 * |                   USART_RX_STA                     |
 * -------------------------------------------------------
 * |    bit 15    |    bit 14    |      bit 13 ~ 0      | 
 * -------------------------------------------------------
 * | 接收完成标志  |   接收到0X0D  | 接收到的有效数据个数   |
 * -------------------------------------------------------
*/

/**
 * 程序要求，发送的字符是以回车换行符结束 (0x0D, 0xOA)
 * 
 */

void Usart_Init(uint32_t bound)
{
    //GPIO初始化配置结构体
    GPIO_InitTypeDef GPIO_InitStructure;
    //USART1初始化结构体
    USART_InitTypeDef USART_InitStructure;
    //NVIC配置结构体
    NVIC_InitTypeDef NVIC_InitStructure;

    //使能USART1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    //使能PA9,PA10的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    //初始化GPIOA9、GPIO10为复用模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//配置为复用模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度为50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //复用PA9 PA10为USART1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    //USART1初始化设置
    USART_InitStructure.USART_BaudRate  = bound;//配置USART波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位的数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件流控制
    USART_Init(USART1, &USART_InitStructure);//初始化串口1

    USART_Cmd(USART1, ENABLE);//USART使能

    USART_ClearFlag(USART1, USART_FLAG_TC);//清除USART1的中断标志位

#if EN_USART_RX
    //开启定时器接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    //配置USART1 NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEMPTIONPRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SUBPRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

}
 
#if EN_USART_RX
 /* 
 * -------------------------------------------------------
 * |                   USART_RX_STA                     |
 * -------------------------------------------------------
 * |    bit 15    |    bit 14    |      bit 13 ~ 0      | 
 * -------------------------------------------------------
 * | 接收完成标志  |   接收到0X0D  | 接收到的有效数据个数   |
 * -------------------------------------------------------
*/


//USART1 接收中断服务函数
//程序要求，发送的字符是以回车换行结束(0x0D回车 0x0A换行)
void USART1_IRQHandler(void)
{
    //串口接收变量
    uint8_t Res = 0;

    //判断是否为接收中断
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)//如果为接收中断
    {
        //读取接收到的数据
        Res = USART_ReceiveData(USART1);
        //判读是否接收完成
        if(USART_RX_STA & 0x8000 == 0)//接收未完成
        {
            //判断是否接收到0x0D
            if(USART_RX_STA & 0x4000)//接收到0x0D
            {
                if(Res != 0x0A)//如果未接收到0x0A
                {
                    //接收失败
                    USART_RX_STA = 0;   
                }
                else
                {
                    //接收到0x0A,标志接收成功
                    USART_RX_STA |= 0x8000;
                }
            }
            else//标志位，标志未接收到0x0D
            {    
                if(Res == 0x0D)//接收到0x0D
                {
                    //标志接收到0X0D
                    USART_RX_STA |= 0x4000;
                }
                else//未接收到0x0D
                {
                    USART_RX_BUF[USART_RX_STA & 0x3FFF] = Res;
                    USART_RX_STA++;//数据自增
                    if(USART_RX_STA > (USART_REC_LEN - 1))
                    {
                        USART_RX_STA = 0;//接收数据错误，重新开始接收
                    }
                }
            }
        }
    }
}

#endif
/***********************************************************************************************************/
