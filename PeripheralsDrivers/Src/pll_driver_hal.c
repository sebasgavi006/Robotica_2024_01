/*
 * pll_driver_hal.c
 *
 *  Created on: Feb 29, 2023
 */

#include <stm32f4xx.h>
#include <string.h>
#include <pll_driver_hal.h>
#include <gpio_driver_hal.h>

uint8_t actual_Frequency = 0;

void PLL_chooseClock(PLL_Config_t *ptrPLL);
void PLL_reFactorM(PLL_Config_t *ptrPLL);
void PLL_reFactorN(PLL_Config_t *ptrPLL);
void PLL_reFactorP(PLL_Config_t *ptrPLL);
void restartBits(PLL_Config_t *ptrPLL);

void PLL_config(PLL_Config_t *ptrPLL){

	actual_Frequency = 16;

	/* Asegurar que el PLL está apagado para configuración */
	PLL_DriverOnOff(ptrPLL,ptrPLL->state);

	/* Selección de reloj de entrada para PLL */
	PLL_chooseClock(ptrPLL);

	/* División de reloj principal para entrar a VCO */
	PLL_reFactorM(ptrPLL);

	/* Multiplicación de salida VCO (2MHz) a 200MHz */
	PLL_reFactorN(ptrPLL);

	/* División de salida para sistema principal (200MHz) a 100MHz */
	/* Reinicio de bits */
	restartBits(ptrPLL);

	/*Aplicamos el valor final para P*/
	PLL_reFactorP(ptrPLL);

	/* Guardar frecuencia final configurada */
	ptrPLL->final_Frequency = actual_Frequency;
}


/* Asegurar que el PLL está apagado para configuración */
void PLL_DriverOnOff(PLL_Config_t *ptrPLL,uint8_t state){
	if(state){
		RCC->CR |= RCC_CR_PLLON;
	}
	else{
		RCC->CR &= ~RCC_CR_PLLON;
	}
}


/* Selección de reloj de entrada para PLL */
void PLL_chooseClock(PLL_Config_t *ptrPLL){
	if(ptrPLL->input_Clock == HSI_CLOCK){
		RCC->PLLCFGR &= RCC_PLLCFGR_PLLSRC_HSI;
	} else {
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;
	}
}


/* División de reloj principal para entrar a VCO */
void PLL_reFactorM(PLL_Config_t *ptrPLL){

	/* Reinicio de bits en divisor /M*/
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;

	/* Divisón por M */
	actual_Frequency = actual_Frequency/(ptrPLL->m_Factor);

	// No configurar si supera los límites
	if(actual_Frequency < 1 || 2 < actual_Frequency){
		while(1){
			// Supera los límites
			__NOP();
		}
	}

	// Configurar factor M
	RCC->PLLCFGR |= (ptrPLL->m_Factor << RCC_PLLCFGR_PLLM_Pos);
}


/* Multiplicación de salida VCO (2MHz) a 200MHz */
void PLL_reFactorN(PLL_Config_t *ptrPLL){

	/* Reinicio de bits xN */
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;

	/* Multiplicación por N */
	actual_Frequency = actual_Frequency * (ptrPLL->n_Factor);

	// No configurar si supera los límites
	if(actual_Frequency < 100 || 432 < actual_Frequency){
		while(1){
			// Supera los límites
			__NOP();
		}
	}
	// Configurar factor N
	RCC->PLLCFGR |= (ptrPLL->n_Factor << RCC_PLLCFGR_PLLN_Pos);
}


/*Aplicamos el valor final para P*/
void PLL_reFactorP(PLL_Config_t *ptrPLL){

	/* División por P */
	actual_Frequency = actual_Frequency/(ptrPLL->p_Factor);
	// No configurar si supera los límites
	if(actual_Frequency > 100){
		while(1){
			// Supera los límites
			__NOP();
		}
	}

}


/* División de salida para sistema principal (200MHz) a 100MHz */
/* Reinicio de bits */
void restartBits(PLL_Config_t *ptrPLL){

	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;

	switch(ptrPLL->p_Factor){
	case 2:{
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;
		break;
	}
	case 4:{
		RCC->PLLCFGR |= 1 << RCC_PLLCFGR_PLLP_Pos;
		break;
	}
	case 6:{
		RCC->PLLCFGR |= 2 << RCC_PLLCFGR_PLLP_Pos;
		break;
	}
	case 8:{
		RCC->PLLCFGR |= 3 << RCC_PLLCFGR_PLLP_Pos;
		break;
	}
	default:{
		while(1){
			// Worong P factor
			__NOP();
		}
	}
	}
}


void PLL_Frequency_Output(GPIO_Handler_t *ptrA8, uint8_t clockSource, uint8_t preScaler){

	/* Encendido GPIO A8 */
	ptrA8->pGPIOx = GPIOA;
	ptrA8->pinConfig.GPIO_PinNumber = PIN_8;
	ptrA8->pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	ptrA8->pinConfig.GPIO_PinAltFunMode = AF0;
	gpio_Config(ptrA8);

	/* Configuración prescaler de MCO1 */
	RCC->CFGR &= ~RCC_CFGR_MCO1PRE;
	if(preScaler == 2){
		RCC->CFGR |= 0b100 << RCC_CFGR_MCO1PRE_Pos;
	}
	else if(preScaler == 3){
		RCC->CFGR |= 0b101 << RCC_CFGR_MCO1PRE_Pos;
	}
	else if(preScaler == 4){
		RCC->CFGR |= 0b110 << RCC_CFGR_MCO1PRE_Pos;
	}
	else if(preScaler == 5){
		RCC->CFGR |= 0b111 << RCC_CFGR_MCO1PRE_Pos;
	}
	else {
		RCC->CFGR &= ~RCC_CFGR_MCO1PRE;
	}

	/* Encendido señal para lectura MCO1 */
	if(clockSource == HSI_CLOCK){
		RCC->CFGR &= ~RCC_CFGR_MCO1;
	}
	else if(clockSource == LSE_CLOCK){
		RCC->CFGR |= (0b01 << RCC_CFGR_MCO1_Pos);
	}
	else if(clockSource == HSE_CLOCK){
		RCC->CFGR |= (0b10 << RCC_CFGR_MCO1_Pos);
	}
	else if(clockSource == PLL_CLOCK){
		RCC->CFGR |= RCC_CFGR_MCO1;
	} else {
		RCC->CFGR &= ~RCC_CFGR_MCO1;
	}
}


// Sistema para encender PLL a configuración seleccionada
void PLL_ON_forSystem(PLL_Config_t *ptrPLL){

	/* Configuración de memoria flash y buses para nueva frecuencia */
	FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk; // Reset flash
	RCC->CFGR &= ~RCC_CFGR_PPRE1; // Reset APB1
	RCC->CFGR &= ~RCC_CFGR_PPRE2; // Reset APB2
	RCC->CFGR &= ~RCC_CFGR_HPRE; // Reset AHB

	if(ptrPLL->final_Frequency <= 30){
		FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
	}
	else if (ptrPLL->final_Frequency <= 64){
		FLASH->ACR |= FLASH_ACR_LATENCY_1WS;

		if(ptrPLL->final_Frequency > 50){
			/* División mínima para no superar limites del bus */
			RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
		}
	}
	else if (ptrPLL->final_Frequency <= 90){
		FLASH->ACR |= FLASH_ACR_LATENCY_2WS;
		/* División mínima para no superar limites del bus */
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
	}
	else {
		FLASH->ACR |= FLASH_ACR_LATENCY_3WS;
		/* División mínima para no superar limites del bus */
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
	}

	// Endender señal de PLL
	RCC->CR |= RCC_CR_PLLON;


	/* Confirmación de PLL listo */
	while(!(RCC->CR & RCC_CR_PLLRDY)){
		__NOP();
	}

	/* Configuración multiplexor que envia reloj a todo el hardware */
	/* Enviar señal PLL */
	RCC->CFGR |= RCC_CFGR_SW_PLL;
}


void systemClock_16MHz(PLL_Config_t *ptrPLL){

	ptrPLL->input_Clock = HSI_CLOCK;
	ptrPLL->m_Factor = 8;
	ptrPLL->n_Factor = 64;
	ptrPLL->p_Factor = 8;
	PLL_config(ptrPLL); // Apaga y configura PLL

	/* Chequear HSI listo */
		while(!(RCC->CR & RCC_CR_HSIRDY)){
		__NOP();
	}

	/* Configuración de memoria flash para 16MHz */
	FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;

	/* Configuración multiplexor que envia reloj a todo el hardware */
	/* Enviar señal desde HSI */
	RCC->CFGR &= RCC_CFGR_SWS_HSI;
}


void systemClock_100MHz(PLL_Config_t *ptrPLL){

	ptrPLL->input_Clock = HSI_CLOCK;
	ptrPLL->m_Factor = 8;
	ptrPLL->n_Factor = 100;
	ptrPLL->p_Factor = 2;
	PLL_config(ptrPLL); // Apaga y configura PLL

	PLL_ON_forSystem(ptrPLL);
}


void systemClock_80MHz(PLL_Config_t *ptrPLL){

	ptrPLL->input_Clock = HSI_CLOCK;
	ptrPLL->m_Factor = 8;
	ptrPLL->n_Factor = 80;
	ptrPLL->p_Factor = 2;
	PLL_config(ptrPLL);

	PLL_ON_forSystem(ptrPLL);
}


void systemClock_Output(void){

	GPIO_Handler_t ptrC9;
	/* Encendido GPIO A8 */
	ptrC9.pGPIOx = GPIOC;
	ptrC9.pinConfig.GPIO_PinNumber = PIN_9;
	ptrC9.pinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	ptrC9.pinConfig.GPIO_PinAltFunMode = AF0;
	gpio_Config(&ptrC9);

	/* Configuración prescaler de MCO2 SysCLK/5 */
	RCC->CFGR &= ~RCC_CFGR_MCO2PRE_Msk;
	RCC->CFGR |= (0b111 << RCC_CFGR_MCO2PRE_Pos);

	/* Encendido señal para lectura MCO2 */
	RCC->CFGR |= RCC_CFGR_MCO2;

}


void systemClock_GetConfig(system_Clock_data *ptrClockData){
/* Fuente de reloj actual */
// if(!(RCC->CFGR & RCC_CFGR_SWS_HSI)){
// strcpy(ptrClockData->clock_Source, "HSI");
// }
// else if(RCC->CFGR & RCC_CFGR_SWS_HSE){
// strcpy(ptrClockData->clock_Source, "HSE");
// }
// else if(RCC->CFGR & RCC_CFGR_SWS_PLL){
// strcpy(ptrClockData->clock_Source, "PLL");
// }

	/* Frecuencia actual en reloj */
	uint8_t factor_aux = 0;

	// Definir y dividir Factor M
	factor_aux = (RCC->PLLCFGR & RCC_PLLCFGR_PLLM)>> RCC_PLLCFGR_PLLM_Pos; // Definir Factor M
	ptrClockData->clock_Frequency = 16/factor_aux;
	factor_aux = 0;

	// Definir y multiplicar Factor N
	factor_aux = (RCC->PLLCFGR & RCC_PLLCFGR_PLLN)>> RCC_PLLCFGR_PLLN_Pos;
	ptrClockData->clock_Frequency = (ptrClockData->clock_Frequency)*factor_aux;
	factor_aux = 0;

	// Definir y dividir Factor P
	factor_aux = (RCC->PLLCFGR & RCC_PLLCFGR_PLLP)>> RCC_PLLCFGR_PLLP_Pos;
	switch(factor_aux){
	case 0:{
		factor_aux = 2;
	break;
	}
	case 1:{
		factor_aux = 4;
	break;
	}
	case 2:{
		factor_aux = 6;
	break;
	}
	case 3:{
		factor_aux = 8;
	break;
	}
	default:{
		__NOP();
	break;
	}
	}
	ptrClockData->clock_Frequency = (ptrClockData->clock_Frequency)/factor_aux;


	/* Frecuencia APB1 */
	switch(RCC->CFGR & RCC_CFGR_PPRE1){
	case RCC_CFGR_PPRE1_DIV1:{
		ptrClockData->APB1_Frequency = ptrClockData->clock_Frequency;
	break;
	}
	case RCC_CFGR_PPRE1_DIV2:{
		ptrClockData->APB1_Frequency = ptrClockData->clock_Frequency/2;
	break;
	}
	case RCC_CFGR_PPRE1_DIV4:{
		ptrClockData->APB1_Frequency = ptrClockData->clock_Frequency/4;
	break;
	}
	case RCC_CFGR_PPRE1_DIV8:{
		ptrClockData->APB1_Frequency = ptrClockData->clock_Frequency/8;
	break;
	}
	case RCC_CFGR_PPRE1_DIV16:{
		ptrClockData->APB1_Frequency = ptrClockData->clock_Frequency/16;
	break;
	}
	default:{
		__NOP();
	break;
	}
	}

	/* Frecuencia APB2 */
	switch(RCC->CFGR & RCC_CFGR_PPRE2){
	case RCC_CFGR_PPRE2_DIV1:{
		ptrClockData->APB2_Frequency = ptrClockData->clock_Frequency;
	break;
	}
	case RCC_CFGR_PPRE2_DIV2:{
		ptrClockData->APB2_Frequency = ptrClockData->clock_Frequency/2;
	break;
	}
	case RCC_CFGR_PPRE2_DIV4:{
		ptrClockData->APB2_Frequency = ptrClockData->clock_Frequency/4;
	break;
	}
	case RCC_CFGR_PPRE2_DIV8:{
		ptrClockData->APB2_Frequency = ptrClockData->clock_Frequency/8;
	break;
	}
	case RCC_CFGR_PPRE2_DIV16:{
		ptrClockData->APB2_Frequency = ptrClockData->clock_Frequency/16;
	break;
	}
	default:{
		__NOP();
	break;
	}
	}

	/* Frecuencia AHB */
	switch(RCC->CFGR & RCC_CFGR_HPRE){
	case RCC_CFGR_HPRE_DIV1:{
		ptrClockData->AHB_Frequency = ptrClockData->clock_Frequency;
	break;
	}
	case RCC_CFGR_HPRE_DIV2:{
		ptrClockData->AHB_Frequency = ptrClockData->clock_Frequency/2;
	break;
	}
	case RCC_CFGR_HPRE_DIV4:{
		ptrClockData->AHB_Frequency = ptrClockData->clock_Frequency/4;
	break;
	}
	case RCC_CFGR_HPRE_DIV8:{
		ptrClockData->AHB_Frequency = ptrClockData->clock_Frequency/8;
	break;
	}
	case RCC_CFGR_HPRE_DIV16:{
		ptrClockData->AHB_Frequency = ptrClockData->clock_Frequency/16;
	break;
	}
	case RCC_CFGR_HPRE_DIV64:{
		ptrClockData->AHB_Frequency = ptrClockData->clock_Frequency/64;
	break;
	}
	case RCC_CFGR_HPRE_DIV128:{
		ptrClockData->AHB_Frequency = ptrClockData->clock_Frequency/128;
	break;
	}
	case RCC_CFGR_HPRE_DIV256:{
		ptrClockData->AHB_Frequency = ptrClockData->clock_Frequency/256;
	break;
	}
	case RCC_CFGR_HPRE_DIV512:{
		ptrClockData->AHB_Frequency = ptrClockData->clock_Frequency/512;
	break;
	}
	default:{
		__NOP();
	break;
	}
	}
	/* Latencia FLASH */
	/* Pendiente */
}


uint8_t getPLL(void){
	/* Frecuencia actual en reloj */
	uint8_t factor_aux = 0;
	uint8_t frecuencia = 16;

	// Definir y dividir Factor M
	factor_aux = (RCC->PLLCFGR & RCC_PLLCFGR_PLLM)>> RCC_PLLCFGR_PLLM_Pos; // Definir Factor M
	frecuencia = frecuencia/factor_aux;
	factor_aux = 0;

	// Definir y multiplicar Factor N
	factor_aux = (RCC->PLLCFGR & RCC_PLLCFGR_PLLN)>> RCC_PLLCFGR_PLLN_Pos;
	frecuencia = frecuencia*factor_aux;
	factor_aux = 0;

	// Definir y dividir Factor P
	factor_aux = (RCC->PLLCFGR & RCC_PLLCFGR_PLLP)>> RCC_PLLCFGR_PLLP_Pos;
	switch(factor_aux){
	case 0:{
		factor_aux = 2;
	break;
	}
	case 1:{
		factor_aux = 4;
	break;
	}
	case 2:{
		factor_aux = 6;
	break;
	}
	case 3:{
		factor_aux = 8;
	break;
	}
	default:{
		__NOP();
	break;
	}
	}

	frecuencia = frecuencia/factor_aux;

	return frecuencia;
}
