/*
 * pll_driver_hal.h
 *
 *  Created on: Feb 29, 2024
 */

#ifndef PLL_DRIVER_HAL_H_
#define PLL_DRIVER_HAL_H_

#include <stm32f4xx.h>
#include <gpio_driver_hal.h>

enum{
	HSI_CLOCK = 0,
	HSE_CLOCK,
	LSE_CLOCK,
	PLL_CLOCK
};
enum{
	PLL_OFF = 0,
	PLL_ON
};


typedef struct
{
	uint8_t input_Clock; 	// Reloj que vamos a usar para configurar el PLL
	uint8_t m_Factor;		// Factor de division para VCO
	uint16_t n_Factor; 		// Factor de multiplicacion VCO
	uint8_t p_Factor; 		// Factor de division para salida PLL
	uint8_t final_Frequency; // Variable para uso futuro, no necesita modificacion
	uint8_t state; 			//Estado encendido o apagado del PLL
} PLL_Config_t;

// Estructura para guardar datos informativos sobre el reloj
typedef struct
{
	uint8_t clock_Frequency;
	uint8_t APB1_Frequency;
	uint8_t APB2_Frequency;
	uint8_t AHB_Frequency;
} system_Clock_data;


void PLL_config(PLL_Config_t *ptrPLL);
void PLL_DriverOnOff(PLL_Config_t *ptrPLL,uint8_t state);
void PLL_Frequency_Output(GPIO_Handler_t *ptrA8, uint8_t clockSource, uint8_t preScaler);
void PLL_ON_forSystem(PLL_Config_t *ptrPLL);
void systemClock_16MHz(PLL_Config_t *ptrPLL);
void systemClock_80MHz(PLL_Config_t *ptrPLL);
void systemClock_100MHz(PLL_Config_t *ptrPLL);
void systemClock_Output(void);
void systemClock_GetConfig(system_Clock_data *ptrClockData);
uint8_t getPLL(void);


#endif /* PLL_DRIVER_HAL_H_ */
