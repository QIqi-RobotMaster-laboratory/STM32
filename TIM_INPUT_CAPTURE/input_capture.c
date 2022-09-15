/**
 * @file input_capture.c
 * @author yuanluochen
 * @brief ��ʱ�����벶�񣬻���STM32f407�����壬����TIM5_CH1 ����ΪPA0
 * @version 0.1
 * @date 2022-09-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//ͨ�ö�ʱ�����벶��
//��ʱ����ͨ������
//ͨ�����TIMx_CHx����Ӧͨ����(��Ӧ��ӳ������)�ĵı����źţ����ú�Ҫ��������ػ����½��أ��ڱ����źŷ�������(����������/�½���)��ʱ��
//��⵽��Ҫ�ĵ�ƽ�����źŵ�ʱ�򣬽���ǰ��ʱ����ֵ(TIMx_CNT)��ŵ���Ӧ�Ĳ���ȽϼĴ���(TIMx_CCRx)���棬�����һ�β���

/*
 * ���ϲ�˵���ɵã���������벶��ߵ�ƽ�źſ�ȡ�
 * ��ô��Ҫ�������������ز��񣬲��������ؽ���ʱ������ֵ(TIMx_CNT)����������
 * Ȼ�������½��ز��񣬲����½���ʱ������ʱ����ֵ(TIMx_CNT)����������
 * ��󽫱��������Ķ�ʱ����ֵ����������Ϳ��Եõ��ߵ�ƽ�źſ��
*/

/*
 * STM32���벶��������
 * ��ʱ��ͨ����Ӧ�����ţ���ʱ�������ⲿ�źţ�����һ���˲�����ʹ�źű�ø���ƽ�ȣ�
 * �������������˹����źŽ�����ؼ����������趨�õı����źţ�
 * ��⵽�ı����źž���ѡ��������������ʱ����ͨ��ѡ����(��ѡ������������ö�ʱ������ͨ��)������ź�
 * ������źţ�����һ����Ƶ�������ֵ���÷�Ƶ�����ڼ�⼸�������ش���һ��
*/

/**
 * ͨ�ö�ʱ���������벶��
 *  
 * 1.�������벶���˲���
 * ��ʱ������Ƶ��fCK_INT����һ����Ƶ������Ƶ��С�ɼĴ���TIMx_CRx��CKD[1:0]�����ã���Ƶϵ��Ϊ2^CKD����CKD[1:0]Ϊ11ʱ����Ƶϵ������������fDTS�����е�CKD����Ϊ0ʱfDTS = fCK_INT��
 * ����IC1F[3:0] = 0011��������IC1ӳ�䵽ͨ��1�ϣ���Ϊ�����ش�������ô���������ص�ʱ������fCK_INT��Ƶ�ʣ���������8��ͨ��1�ĵ�ƽ��������Ǹߵ�ƽ����˵������Ч�������ͻᴥ�����벶���жϡ�
 * 
 * 2.�������벶����
 * ��ؼĴ���
 * CC1P������/����1�������
 * CC1ͨ������Ϊ���
 * 0��OC1�ߵ�ƽ��Ч
 * 1��OC1�͵�ƽ��Ч
 * CC1ͨ������Ϊ����
 * ��λѡ��IC1����IC1�ķ����ź���Ϊ�����򲶻��ź�
 * 0�������ࣺ��������IC1�������أ��������ⲿ������ʱ��IC1������
 * 1�����򣺲�������IC1���½��أ��������ⲿ������ʱ��IC1����
 * 
 * 3.���ò���ӳ��ͨ��
 * �Ĵ���CC1S[1:0]:ͨ��1������/�Ƚ�ѡ��(Capture/Compare 1 selection)
 * ����ͨ���ķ���(����/���),�������ѡ��
 * 00��CC1ͨ��������Ϊ���
 * 01��CC1ͨ��������Ϊ���룬IC1ӳ�䵽TI1��
 * 10��CC1ͨ��������Ϊ���룬IC1ӳ�䵽TI2��
 * 11��CC1ͨ��������Ϊ���룬IC1ӳ�䵽TRC�ϡ���ģʽ���������ڲ����������뱻ѡ��ʱ
 * 
 * 4.�������벶���Ƶ��
 * 
 * 5.������Ч���źſ��Կ����ж�
 */


#include "input_capture.h"

//���벶��

/*******************************************************************************************************/

/**
 * ���ߵ�ƽ�ĳ���ʱ��
 * 
 * 1.����һ��8λ�ı�������������
 * ---------------------------------------------------------
 * |   bit 7   |     bit 6     |         bit 5 ~ 0         |
 * ---------------------------------------------------------
 * |������ɱ�־|���񵽸ߵ�ƽ��־|���񵽸ߵ�ƽ��ʱ�������ʱ��|
 * ---------------------------------------------------------
 * ����λ7��־������ɣ�λ6��־���񵽸ߵ�ƽ��������񵽸ߵ�ƽbit6����Ϊ1��������񵽵͵�ƽ���ͻ���bit6�Ƿ�Ϊ1��Ϊ1���򲶻���ɱ�־��1
 * ������񵽸ߵ�ƽ�󣬶�ʱ��32Ϊ������������������¼�����bit5~0��ֵ��1
 * ���Ըߵ�ƽʱ�� = [5:0] * ��������0���������ֵ��ʱ��  + ǰ��ʱ���ֵ
 * 
 * 
 */

/**
 * �⺯���������벶��
 * 
 * 1.��ʼ����ʱ����ͨ����Ӧ��IO��ʱ��
 * 2.��ʼ��IO�ڣ�ģʽΪ����
 * 3.�������Ÿ���ӳ��
 * 4.��ʼ����ʱ��ARR,PSC
 * 5.��ʼ�����벶��ͨ��
 * 6.���������ж�
 * 7.ʹ�ܶ�ʱ��
 * 8.��д�жϷ�����
 * 
 */

//��ռ���ȼ�
#define NVIC_PREEMPTIONPRIORITY 2
//�����ȼ�
#define NVIC_SUBPRIORITY 0

void TIM_Input_Capture(uint16_t psc, uint32_t arr)
{
    //GPIO��ʼ���ṹ��
    GPIO_InitTypeDef GPIO_InitStructure;
    //��ʱ����ʼ���ṹ��
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    //��ʱ�����벶��ṹ��
    TIM_ICInitTypeDef TIM_ICInitStructure;
    //NVIC���ýṹ��
    NVIC_InitTypeDef NVIC_InitStructure;

    //ʹ��TIM5��ʱ��ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    //��ʼ��PA0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//��ʼ��Ϊ����ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//�ٶ�Ϊ100MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //PA0����ΪTIM5
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM5);

    //����TIM5�Ĳ���
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���ģʽ
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//���ö�ʱ��ʱ�ṹ��������Ա�������
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStructure);

    //��ʼ�� TIM5 ͨ��1 ���벶�����
    TIM_ICInitStructure.TIM_ICFilter = 0x00; //���������˲���
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;//�����ز���
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;//���������IC1
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//�������IC1ӳ�䵽TI1��
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;//�����˲���������Ƶ
    TIM_ICInit(TIM5, &TIM_ICInitStructure);

    //���ö�ʱ���жϣ���������жϣ�����CC1TE�����жϣ���������ж�ԭ������ʱ����ֵ�����
    TIM_ITConfig(TIM5, TIM_IT_Update | TIM_IT_CC1, ENABLE);

    //ʹ�ܶ�ʱ��5
    TIM_Cmd(TIM5, ENABLE);

    //�����ж�NVIC
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PREEMPTIONPRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SUBPRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
    /*��Ҫ�������������ж����ȼ�����*/
}


/**
 * 
 * �ߵ�ƽʱ��Ϊ = ((TIM5CH1_CAPTURE_STA & 0x3F) * ((PSC - 1) / CK_INT) * ARR) + TIM5CH1_CAPTURE_VAL * ((PSC -1) / CK_INT)
 * 
 */

static uint8_t TIM5CH1_CAPTURE_STA = 0;//���벶��״̬����λ���������
static uint32_t TIM5CH1_CAPTURE_VAL = 0;//������ɺ�Ķ�ʱ����ֵ
 /*
 * ---------------------------------------------------------
 * |   bit 7   |     bit 6     |         bit 5 ~ 0         |
 * ---------------------------------------------------------
 * |������ɱ�־|���񵽸ߵ�ƽ��־|���񵽸ߵ�ƽ��ʱ�������ʱ��|
 * ---------------------------------------------------------
 */
//��ʱ��5�жϷ�����
void TIM5_IRQHandler(void)
{
   if(TIM_GetITStatus(TIM5, TIM_IT_CC1) == SET)//TIM5ͨ��1������һ�β����¼�
    {
        //�����һ�β���һ�������أ��򱾴β���һ���½���
        if(TIM5CH1_CAPTURE_STA & 0x40)//�Ѿ�����һ�������أ��򱾴�Ϊ�½���
        {
            TIM5CH1_CAPTURE_STA |= 0x80; //��ǲ������
            TIM5CH1_CAPTURE_VAL = TIM_GetCapture1(TIM5);//��ȡ��ǰ�Ĳ���ֵ
            //������ɺ���������Ϊ�����ز���
            TIM_OC1PolarityConfig(TIM5, TIM_ICPolarity_Rising);
        }
        else//��δ���������أ��򱾴β���Ϊ������
        { 
            //TIM5CH1_CAPTURE_VAL��ֵ���
            TIM5CH1_CAPTURE_VAL = 0;
            //TIM5CH1_CAPTURE_STA bit 5 ~ 0��ֵ���ã�
            TIM5CH1_CAPTURE_STA = 0;
            //��ǲ��񵽸ߵ�ƽ bit6����Ϊ1
            TIM5CH1_CAPTURE_STA |= 0x40;
            
            //��TIM5��ֵ����
            //���Ҫ���ö�ʱ��5�ļ���ֵ����ô��Ҫ�ȹرն�ʱ��5�������ö�ʱ��5����ֵ
            TIM_Cmd(TIM5, DISABLE);//�رն�ʱ��5
            //����ֵ����
            TIM_SetCounter(TIM5, 0);
            
            //���ö�ʱ��5Ϊ�½��ز���
            TIM_OC1PolarityConfig(TIM5, TIM_ICPolarity_Falling);
            //����ʹ�ܶ�ʱ��5
            TIM_Cmd(TIM5, ENABLE);
        }
    }
    //������벶��״̬Ϊ��δ�ɹ�����,��ʱ�����ڲ����������غ����ڲ����½���
    if(TIM5CH1_CAPTURE_STA & 0x80 == 0)//��bit7Ϊ0������δ���
    {
        //�ж��Ƿ�ʱ���������������bit 5 ~ 0 ��ֵ��һ
        if(TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)//��ʱ�����������ж���ֵ���
        {
            //�ж��Ƿ��ڲ���͵�ƽ�Ĺ����У�������ڲ���͵�ƽ�Ĺ����У� bit 5 ~ 0��ֵҪ��1
            if(TIM5CH1_CAPTURE_STA & 0x40)//�Ѿ����񵽸ߵ�ƽ����ʱ��bit 5 ~ 0 λ��1
            {
                //�ж�bit 5 ~ 0 �Ƿ�������������ˣ�Ϊ��ֹ�ƻ�����bit6��bit7����ֵ�����ǳɹ�����һ��
                if((TIM5CH1_CAPTURE_STA & 0x3F) == 0x3F)//bit 5 ~ 0 ����
                {
                    TIM5CH1_CAPTURE_STA |= 0x80;//��ǲ������
                    TIM5CH1_CAPTURE_VAL = 0xFFFFFFFF;//��¼��ʱ����ֵΪ��
                    /**
                     * 
                     * ����Ϊ 0XFFFFFFFF �����ڼٶ���ARRΪ0XFFFFFFFF
                     */
                }
                else
                {
                    TIM5CH1_CAPTURE_STA++;//��ֵ���һ��bit 5 ~ 0 ������
                }
            }
        }
    }

    //����жϱ�־λ
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC1 | TIM_IT_Update);
}




/*******************************************************************************************************/
