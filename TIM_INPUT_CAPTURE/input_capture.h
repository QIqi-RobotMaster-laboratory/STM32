#ifndef INPUT_CAPTURE_H
#define INPUT_CAPTURE_H

#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"

/**
 * @brief 定时器输入捕获，基于stm32f407,
 * 
 * @param psc 定时器时基单元预分频值
 * @param arr 定时器自动重装载值
 */
void TIM_Input_Capture(uint16_t psc, uint32_t arr);

/**
 * 注意：建议初始化时设置ARR为0xFFFFFFFF
 * 
 * 原因：由于配置中断服务函数时假定ARR为0xFFFFFFFF来使用 
 * 
 */

#endif
