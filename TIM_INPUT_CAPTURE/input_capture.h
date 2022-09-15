#ifndef INPUT_CAPTURE_H
#define INPUT_CAPTURE_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"

/**
 * @brief ��ʱ�����벶�񣬻���stm32f407,
 * 
 * @param psc ��ʱ��ʱ����ԪԤ��Ƶֵ
 * @param arr ��ʱ���Զ���װ��ֵ
 */
void TIM_Input_Capture(uint16_t psc, uint32_t arr);

/**
 * ע�⣺�����ʼ��ʱ����ARRΪ0xFFFFFFFF
 * 
 * ԭ�����������жϷ�����ʱ�ٶ�ARRΪ0xFFFFFFFF��ʹ�� 
 * 
 */

#endif
