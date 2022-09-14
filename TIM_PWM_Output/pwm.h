#ifndef PWM_H
#define PWM_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"

/**
 * @brief 通过定时器输出比较功能实现PWM输出
 * 
 * @param arr 定时器时基单元自动重装载值
 * @param psc 定时器时基单元预分频系数
 */
void TIM_PWM_Init(uint32_t arr, uint16_t psc);

/*
 * 定时器初始化完成后调用
 * TIM_SetCompare1() 
 * 更改CCRx的值调整占空比
*/

#endif
