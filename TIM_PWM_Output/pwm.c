/**
 * @file pwm.c
 * @author yuanluochen
 * @brief stm32f407,PWM�������ʱ������Ƚϣ�����TIM14���ͨ��1(TIM14_CH1)����GPIOF9���PWM����PWMģʽ1����Ч��ƽΪ�͵�ƽ
 * @version 0.1
 * @date 2022-09-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */


//PWM��� - ��ʱ������Ƚ�
/**
 * ��ʱ��PWM�������ԭ��
 * 
 * ������ѧϰ�궨ʱ���жϺ�����֪����ʱ��ʱ��Դ���붨ʱ��ʱ����Ԫ��
 * ����PSC��Ƶ,ÿ��һ�����ڣ�ʱ����Ԫ CNTֵ����һ�Σ�ֱ������ARRʱ��ʱ�����������жϣ�CNT��ֵ����
 * ��ʵ��PWM�����������Ҫ��CNT���ص�ARR֮ǰ����һ��CCR��ֵ������Ƚ����PWM��
 * 
 */

//PWM�Ĺ�������(ͨ��xΪ��)
//���ģʽ�������Ƚ�CNT��CCRx��ֵ������ CCMR1:OC1M[2:0]����PWMģʽ���������������жϺ�ʱ�����Ч��ƽ��
//����TIMx_CCER�ж����/��������Ϊ0����Ϊ�ߵ�ƽ��Ч����Ϊ1�������������źŽ���һ�����ţ��͵�ƽ��Ч
//�������CCER��CC1EΪ�������/�����Ƿ�ʹ�ܡ�

/**
 * �Ĵ���
 * CCRx(x = 1,2,3,4):����Ƚ�(ֵ)�Ĵ���(x = 1,2,3,4):���ñȽ�ֵ��
 * CCMRx:OCxM[2:0]λ������PWM��ʽ�£���������PWMģʽ1[110]��PWMģʽ2[111]
 * PWMģʽ1��CNT�Ĵ�������ʱ��TIMx_CNT < TIMx_CCR1ʱͨ��1Ϊ��Ч��ƽ��TIMx_CNT > TIMx_CCR1ͨ��1Ϊ��Ч��ƽ
 * PWMģʽ2��CNT�Ĵ�������ʱ��TIMx_CNT > TIMx_CCR1ʱͨ��1Ϊ��Ч��ƽ��TIMx_CNT > TIMx_CCR1ͨ��1Ϊ��Ч��ƽ
 * CCER :CC1Pλ:���/����1������ԡ�0���ߵ�ƽ��Ч��1���͵�ƽ��Ч��
 * CCER :CC1Eλ:���/����1���ʹ�ܡ�0���رգ�1����
 */

/**
 * PWMģʽ
 * 
 * �����ȵ���ģʽ���Բ���һ����TIMx_ARR�Ĵ���ȷ��Ƶ�ʡ���TIM_CCRx�Ĵ���ȷ��ռ�ձȵ��źš�
 * PWM�źŵ�Ƶ�ʻ�������ARR������ռ�ձ���CCRx����
 * 
 * PWMģʽ1����������ֵ < CCRx��ֵ ��Ч
 * PWMģʽ2����������ֵ > CCRx��ֵ ��Ч
 * 
 * ��������TIMx_CCMRx�Ĵ���OCxPEλ��ʹ����Ӧ��Ԥװ�ؼĴ���
 * ���Ҫ����TIMx_CR1�Ĵ�����ARPEλ��ʹ���Զ���װ�ص�Ԥװ�ؼĴ���
 * 
 * ARPE = 1��ARR������Ч��ARPE = 0��ARR�¸��Ƚ�������Ч
 * 
 */

#include "pwm.h"

//PWM���

/*************************************************************************************/

/**
 * �⺯������PWM���
 * 
 * 1.ʹ�ܶ�ʱ��14�����IO��ʱ��
 *   ʹ�ܶ�ʱ��14ʱ��
 *   ʹ��GPIOFʱ��
 * 2.��ʼ��IO��Ϊ���ù������
 * 3.GPIOF9����ӳ�䵽��ʱ��14
 * 4.��ʼ����ʱ����ARR,PSC��
 * 5.��ʼ������Ƚϲ���
 * 6.ʹ��Ԥװ�ؼĴ���
 * 7.ʹ���Զ���װ�ص�Ԥװ�ؼĴ���
 * 8.ʹ�ܶ�ʱ��
 * 9.���ϸı�Ƚ�ֵCCRx,�ﵽ��ͬ��ռ�ձ�Ч��
 * ���һ������ TIM_SetCompare1() ����CCRx������ռ�ձ�
 * 
 */

//�ú������� TIM14 TIM14_CH1 PF9���PWM��

//GPIO��һ��ʼ��AHB1���ߣ�����������ͨ�����ù��ܽӵ�����ʱ����

void TIM_PWM_Init(uint16_t arr, uint16_t psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    

    //ʹ�ܶ�ʱ��ʱ�ӣ�TIM14������APB1���ߣ�����ʹ��APB1TIM14��ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
    //ʹ�ܶ�ʱ�����IO��ʱ�ӣ�TIM14_CH1 IO��ΪPF9����ʹ��GPIOF��ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    //��ʼ��PF9�����临��ΪTIM14
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//����GPIOģʽΪ����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//GPIO������
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    //GPIOF9����Ϊ��ʱ��14
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);

    //���ö�ʱ�� ��ʼ����ʱ��14
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc; //��ʱ����Ƶ
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���
    TIM_TimeBaseInitStructure.TIM_Period = arr; //�Զ���װ��ֵ
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM14, &TIM_TimeBaseInitStructure);

    //��ʱ��ͨ��1��ʼ�������ģʽΪPWMģʽ1������Ϊ�ͣ���CNT < CCRxΪ��Ч��ƽ����Ч��ƽΪ�͵�ƽ��
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//ģʽΪPWM1���ڸ�ģʽ�¼���ֵCNT < CCRxΪ��Ч��ƽ
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//����Ϊ�ͣ�����Ч��ƽΪ�͵�ƽ
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputNState_Enable;//���ʹ��
    TIM_OC1Init(TIM14, &TIM_OCInitStructure);

    //ʹ��TIM14ͨ��1��Ԥװ��ʹ��
    TIM_OC1PreloadConfig(TIM14, TIM_OCPreload_Enable);

    //ʹ��ARR��Ԥװ��ʹ��
    TIM_ARRPreloadConfig(TIM14, ENABLE);
    
    //ʹ�ܶ�ʱ�� 14
    TIM_Cmd(TIM14, ENABLE);
}


/**************************************************************************************/
