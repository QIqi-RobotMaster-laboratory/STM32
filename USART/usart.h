#ifndef USART_H
#define USART_H

#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"

//定义接受最大字节数
#define USART_REC_LEN 200
//开启USART接收中断
#define EN_USART_RX 1



/**
 * @brief 串口通信初始化函数
 * 
 * @param bound 串口波特率
 */
void Usart_Init(uint32_t bound);

#endif
