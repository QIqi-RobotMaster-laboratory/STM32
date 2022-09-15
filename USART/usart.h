#ifndef USART_H
#define USART_H

#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"

//�����������ֽ���
#define USART_REC_LEN 200
//����USART�����ж�
#define EN_USART_RX 1



/**
 * @brief ����ͨ�ų�ʼ������
 * 
 * @param bound ���ڲ�����
 */
void Usart_Init(uint32_t bound);

#endif
