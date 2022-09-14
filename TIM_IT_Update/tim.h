#ifndef TIM_H
#define TIM_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
/**
 * @brief 此函数为原为TIM3初始化函数，芯片为STM32f407
 * 
 * @param arr 自动重装载器的数值
 * @param psc 定时器时钟源的预分频系数
 */
void TIM_init(uint32_t arr, uint16_t psc);


#endif
