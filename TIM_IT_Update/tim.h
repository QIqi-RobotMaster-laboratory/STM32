#ifndef TIM_H
#define TIM_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
/**
 * @brief �˺���ΪԭΪTIM3��ʼ��������оƬΪSTM32f407
 * 
 * @param arr �Զ���װ��������ֵ
 * @param psc ��ʱ��ʱ��Դ��Ԥ��Ƶϵ��
 */
void TIM_init(uint32_t arr, uint16_t psc);


#endif
