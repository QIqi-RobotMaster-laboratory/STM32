/**
 * @file usart.c
 * @author yuanluochen
 * @brief ����ͨ��ʵ�飬����stm32f407������PA9��PA10����ΪUSART1��ʵ�ִ���ͨ��
 * @version 0.1
 * @date 2022-09-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//USARTͨ��ͬ���첽�շ�������UARTͨ���첽�շ�������������USART�����UART����ͬ�����ܣ����������ã��������߲�಻��

#include "usart.h"


/***********************************************************************************************************/

/**
 * ��׼�����ô���
 * 
 * 1.����ʱ��ʹ��
 * .GPIOʱ��ʹ��
 * 2.���Ÿ���ӳ��
 * 3.GPIO�˿�ģʽ����Ϊ����
 * 4.���ڲ�����ʼ��
 * 5.�����жϲ��ҳ�ʼ��NVIC(�����Ҫ�жϲ���Ҫ�������)
 * 6.ʹ�ܴ���
 * 7.��д�жϴ�����
 * 8.���������շ�
 * 9.���ڴ���״̬��ȡ 
 * 
 */

//��ռ���ȼ�����λ3
#define NVIC_PREEMPTIONPRIORITY 3
//��Ӧ���ȼ�����Ϊ3
#define NVIC_SUBPRIORITY 3

//���ջ������飬���ջ��壬���USART_REC_LEN���ֽڣ�ĩ�ֽ�Ϊ���з�
uint8_t USART_RX_BUF[USART_REC_LEN] = { 0 };
//����״̬��־
uint16_t USART_RX_STA = 0;
/* 
 * -------------------------------------------------------
 * |                   USART_RX_STA                     |
 * -------------------------------------------------------
 * |    bit 15    |    bit 14    |      bit 13 ~ 0      | 
 * -------------------------------------------------------
 * | ������ɱ�־  |   ���յ�0X0D  | ���յ�����Ч���ݸ���   |
 * -------------------------------------------------------
*/

/**
 * ����Ҫ�󣬷��͵��ַ����Իس����з����� (0x0D, 0xOA)
 * 
 */

void Usart_Init(uint32_t bound)
{
    //GPIO��ʼ�����ýṹ��
    GPIO_InitTypeDef GPIO_InitStructure;
    //USART1��ʼ���ṹ��
    USART_InitTypeDef USART_InitStructure;
    //NVIC���ýṹ��
    NVIC_InitTypeDef NVIC_InitStructure;

    //ʹ��USART1ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    //ʹ��PA9,PA10��ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    //��ʼ��GPIOA9��GPIO10Ϊ����ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//����Ϊ����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�Ϊ50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //����PA9 PA10ΪUSART1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    //USART1��ʼ������
    USART_InitStructure.USART_BaudRate  = bound;//����USART������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ�����ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ��������
    USART_Init(USART1, &USART_InitStructure);//��ʼ������1

    USART_Cmd(USART1, ENABLE);//USARTʹ��

    USART_ClearFlag(USART1, USART_FLAG_TC);//���USART1���жϱ�־λ

#if EN_USART_RX
    //������ʱ�������ж�
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    //����USART1 NVIC
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
 * | ������ɱ�־  |   ���յ�0X0D  | ���յ�����Ч���ݸ���   |
 * -------------------------------------------------------
*/


//USART1 �����жϷ�����
//����Ҫ�󣬷��͵��ַ����Իس����н���(0x0D�س� 0x0A����)
void USART1_IRQHandler(void)
{
    //���ڽ��ձ���
    uint8_t Res = 0;

    //�ж��Ƿ�Ϊ�����ж�
    if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)//���Ϊ�����ж�
    {
        //��ȡ���յ�������
        Res = USART_ReceiveData(USART1);
        //�ж��Ƿ�������
        if(USART_RX_STA & 0x8000 == 0)//����δ���
        {
            //�ж��Ƿ���յ�0x0D
            if(USART_RX_STA & 0x4000)//���յ�0x0D
            {
                if(Res != 0x0A)//���δ���յ�0x0A
                {
                    //����ʧ��
                    USART_RX_STA = 0;   
                }
                else
                {
                    //���յ�0x0A,��־���ճɹ�
                    USART_RX_STA |= 0x8000;
                }
            }
            else//��־λ����־δ���յ�0x0D
            {    
                if(Res == 0x0D)//���յ�0x0D
                {
                    //��־���յ�0X0D
                    USART_RX_STA |= 0x4000;
                }
                else//δ���յ�0x0D
                {
                    USART_RX_BUF[USART_RX_STA & 0x3FFF] = Res;
                    USART_RX_STA++;//��������
                    if(USART_RX_STA > (USART_REC_LEN - 1))
                    {
                        USART_RX_STA = 0;//�������ݴ������¿�ʼ����
                    }
                }
            }
        }
    }
}

#endif
/***********************************************************************************************************/
