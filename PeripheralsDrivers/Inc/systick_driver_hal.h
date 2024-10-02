/*
 * systick_driver_hal.h
 *
 *  Created on: Oct 20, 2023
 *      Author: sgaviriav
 */

#ifndef SYSTICK_DRIVER_HAL_H_
#define SYSTICK_DRIVER_HAL_H_


#define HSI_CLOCK_CONFIGURED		0
#define HSE_CLOCK_CONFIGURED		1
#define PLL_CLOCK_CONFIGURED		2
#define SYSTICK_LOAD_VALUE_16MHz_1ms			16000//numero de ciclos en 1ms
#define SYSTICK_LOAD_VALUE_80MHz_1ms			80000//numero de ciclos en 1ms
#define SYSTICK_LOAD_VALUE_100MHz_1ms			100000//numero de ciclos en 1ms


void config_SysTick_ms(uint8_t systemClock);
uint64_t getTicks_ms(void);
void delay_ms(uint32_t wait_time_ms);

#endif /* SYSTICK_DRIVER_HAL_H_ */
