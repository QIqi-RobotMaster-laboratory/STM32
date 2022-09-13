/**
 * @file tim.c
 * @author yuanluochen
 * @brief STM32f407��Ƭ����ͨ�ö�ʱ���жϣ��������ϼ���ģʽ
 * @version 0.1
 * @date 2022-09-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//���ö�ʱ��������ѡ��ʱ������Դ��һ��ѡ���ڲ�ʱ��(CK_INT),Ϊ��ʱ��ʱ��Դ��
/**
 * ��ʱ��ʱ�ӿ�ѡ��
 * 1.�ڲ�ʱ��CK_INT
 * 2.�ⲿʱ��ģʽ1:�ⲿ�����(TIx)
 * 3.�ⲿʱ��ģʽ2:�ⲿ��������(ETR)��(������TIM2,3,4)
 * 4.�ڲ���������(ITRx):ʹ��һ����ʱ����Ϊ��һ����ʱ����Ԥ��Ƶ�������������һ����ʱ��Timer1����Ϊ��һ����ʱ��Timer2��Ԥ��Ƶ��
*/
//���ö�ʱ��ʱ����Ԫ����������PSC(Ԥ��Ƶϵ��)����ʱ������Դ���з�Ƶ��
//ͨ�ö�ʱ���ڲ�ʱ��Դ����APB1���߾�����Ƶ(*1 / *2)����CK_INT��CK_INT������CK_PSC��CK_PSC������Ƶ��(PSC)������CK_CNT��
//���APB1���߷�Ƶϵ��Ϊ1����ôAPB1��Ϊ��ʱ��CK_INT�ı�Ƶϵ��Ϊ1�����APB1��Ƶϵ������1��Ϊ��������2��3��4��������ôAPB1��ΪCK_INT�ı�Ƶϵ��Ϊ2��
/**
 * stm32f407 ʱ��
 * SYSCLK = 168MHz
 * AHBʱ�� = 168MHz
 * APB1ʱ�� = SYSCLK/4 = 42MHz
 * APB2ʱ�� = SYSCLK/2 = 84MHz
 * APB1�ķ�Ƶϵ�� = AHB/APB1ʱ�� = 4 > 1
 * ����CK_INT = APB1 * 2 = 84MHz
 * ����CK_CNT = 84MHz / PSC
*/
/**
 * ������ģʽ
 * 1.���ϼ���ģʽ: ��������0�����Զ����ص�ARR������һ�������жϹ��㡣
 * 2.���¼���ģʽ����������ARR������0�����������жϣ����»ص�ARR
 * �������ģʽ����������0��ARR���ٴ�ARR��0���ظ����Ϲ���
*/
/**
 * ��ؼĴ���
 * 
 * 1. ��������ǰֵ�Ĵ��� TIMx_CNT 16λ �洢��ʱ����ǰֵ
 * 2. Ԥ��Ƶ�Ĵ���TIMx_PSC 16λ �Զ�ʱ��ʱ�ӽ��з�Ƶ
 * 3. �Զ���װ�ؼĴ��� TIMx_ARR 16λ
 * 
 * 4. ���ƼĴ���1 TIMx_CR1 16λ 
 * �Ĵ���λ4��DIR(Direction)λ���Ƽ�����������ʽ����0 ���ϼ�������1 ���¼���
 * �Ĵ���λ0��CENʹ�ܼ�ʱ������0 ��ֹ��ʱ������1 ʹ�ܼ�����
 * 5. DMA�ж�ʹ�ܼĴ��� TIMx_DIER 16λ
 * λ5��������ʼ��Ϊ0
 * λ4��CC4IE ������/�Ƚ�4�ж�(Capture/Compare4 interrupt enable)
 * λ3��CC4IE ������/�Ƚ�3�ж�(Capture/Compare3 interrupt enable)
 * λ2��CC4IE ������/�Ƚ�2�ж�(Capture/Compare2 interrupt enable)
 * λ1��CC4IE ������/�Ƚ�1�ж�(Capture/Compare1 interrupt enable)
 * λ0��UIE ��������жϣ�Update interrupt enable)
 */

/**
 * ���ö�ʱ���жϲ���
 * 
 * 1.ѡ��ʱ��
 * 2.����Ԥ��Ƶϵ�� PSC �Զ���װ��ֵ ARR
 * 3.ʹ�ܶ�ʱ��
 * 4.�����жϷ�����
 * 
 */

#include "tim.h"

/*��ʱ���ж�*/
/**************************************************************************/

/**
 * �⺯����ʱ���ж�ʵ�ֲ���
 * 
 * 1.ʹ�ܶ�ʱ��ʱ��
 * 2.��ʼ����ʱ��������ARR��PSC��ȷ����ʱʱ��
 * 3.������ʱ���жϣ�����NVIC
 * 4.ʹ�ܶ�ʱ��
 * 5.��д�жϷ����� 
 * 
 */

//��ʱ�����ʱ�� Tout(���ʱ��) = (ARR + 1)* ((PSC + 1) / Tclk)

#define NVIC_PREEMPTIONPRIORITY 0x01//NVIC��ռ���ȼ�
#define NVIC_SUBPRIORITY 0x03       //NVIC�����ȼ�

void TIM_init(uint16_t arr, uint16_t psc)
{
    //��ʼ���ṹ��
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;//��ʱ����ʼ���ṹ��
    NVIC_InitTypeDef NVIC_InitStructure;//��ʱ�������жϽṹ��

    //ʹ�ܶ�ʱ��ʱ�ӣ�TIM3��APB1���ߣ�������ʱ���鿴�ļ�stm32f4xx_rcc.h
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//ʹ�ܶ�ʱ��3��ʱ��
    
    //��ʼ����ʱ��,����ARR��PSC ȷ����ʱʱ��
    TIM_TimeBaseInitStructure.TIM_Period = arr; //�Զ���װ��ֵ
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc; //Ԥ��Ƶϵ��
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ���ģʽ
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //��Ƶ����Ҫ���ã����������
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    //������ʱ��3�����ж� TIM_IT_Update
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    //����NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //��ʱ��3��NVIC
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEMPTIONPRIORITY;//��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SUBPRIORITY;//�����ȼ�
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //ʹ�ܶ�ʱ��
    TIM_Cmd(TIM3, ENABLE);
}

//��ʱ���жϷ�����
void TIM3_IRQHandler(void)
{
    //�ж��Ƿ�ΪTIM3�����жϣ�����ǣ���ΪSET��������ǣ���ΪRESET
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        /*
         * �жϷ���������
         */

        //����жϱ�־λ��ֹһֱ�����ж�
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

/**************************************************************************/
