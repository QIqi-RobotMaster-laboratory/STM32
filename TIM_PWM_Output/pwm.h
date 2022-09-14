#ifndef PWM_H
#define PWM_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"

/**
 * @brief ͨ����ʱ������ȽϹ���ʵ��PWM���
 * 
 * @param arr ��ʱ��ʱ����Ԫ�Զ���װ��ֵ
 * @param psc ��ʱ��ʱ����ԪԤ��Ƶϵ��
 */
void TIM_PWM_Init(uint32_t arr, uint16_t psc);

/*
 * ��ʱ����ʼ����ɺ����
 * TIM_SetCompare1() 
 * ����CCRx��ֵ����ռ�ձ�
*/

#endif
