/*
 * pll_driver_hal.c
 *
 *  Created on: Feb 29, 2023
 */

#include <stm32f4xx.h>
#include <string.h>
#include <pll_driver_hal.h>
#include <gpio_driver_hal.h>

void configPLL(PLL_Handler_t *ptrPLL_Handler){

	// HSI clock selected as PLL and PLLI2S clock entry
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC);

	//Se configura la frecuencia a 16MHz, la que es por defecto
	if(ptrPLL_Handler->clkSpeed == FREQUENCY_16MHz){
		//System clock switch
		/*
		   	00: HSI oscillator selected as system clock
			01: HSE oscillator selected as system clock
			10: PLL selected as system clock
		 */
		RCC->CFGR &= ~RCC_CFGR_SW;
	}

	//Configuramos 80MHz usando el PLL como entrada para el reloj HSI
	else if(ptrPLL_Handler->clkSpeed == FREQUENCY_80MHz){

		//Selecionamos HSI como la fuente para el PLL
		RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC);


		/*
		 * Debemos configurar el RCC_PLLCFGR
		 * F_VCO=16(PLLN / PLLM), tenemos PLLM = 8 , PLLN=80 y PLLP= 2
		 * F_VCO = 16(80/2) = 160 MHz
		 * F_pll = F_VCO/PLLP
		 * F_pll = 160/2 = 80MHz
		 */
		//Primero limpiamos
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;

		RCC->PLLCFGR |= RCC_PLLCFGR_PLLM_3;							// 000101: PLLM = 8
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_6 |RCC_PLLCFGR_PLLN_4;		// 1010000: PLLN = 80
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;							// 00: PLLP = 2


		// Cambio el valor VOS (regulador de voltaje) para que se tenga el
		// rendimiento adecuado Esto se debe hacer antes de encender el PLL
		PWR->CR &= ~PWR_CR_VOS_0;
		PWR->CR |= PWR_CR_VOS_1;//es lo mismo que PWR->CR |= PWR_CR_VOS_1;



		//Encendemos el PLL
		RCC->CR |= RCC_CR_PLLON;

		//Esperamos que el clk se estabilice
		while(!(RCC->CR & RCC_CR_PLLRDY)){
			__NOP();
		}




		/*
		 * Configuramos el prescaler de ser necesario
		 * En este caso ya esta listo entonces le damos valor de 1
		 */

		RCC->CFGR &= ~RCC_CFGR_HPRE;
		RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

		//Los perifericos que estan conectados al APB1 tienen una velocidad maxima de 50MHz
		//El APB2 tiene una velocidad maxima de 100MHz

		//Dividimos los 80MHz /2 para tener 40MHz en el APB1
		RCC->CFGR &= ~RCC_CFGR_PPRE1;
		RCC->CFGR |= RCC_CFGR_PPRE1_2;

		//El APB2 soporta los 80MHz
		RCC->CFGR &= ~RCC_CFGR_PPRE2;

		//Configurando el System Clock Switch para usar el clock con el PLL
		RCC->CFGR &= ~RCC_CFGR_SW;
		RCC->CFGR |= RCC_CFGR_SW_1;
	}

/*------------------------------------------------------ 100 MHz -----------------------------------------------------*/
	else if(ptrPLL_Handler->clkSpeed == FREQUENCY_100MHz){
		//Activamos la señal de reloj del periferico
		RCC->AHB1ENR |= RCC_APB1ENR_PWREN;

		/*
		 * Debemos configurar el RCC_PLLCFGR
		 * F_VCO=16(PLLN / PLLM), tenemos PLLM = 8 , PLLN=100 y PLLP= 2
		 * F_VCO = 16(100/8) = 200 MHz
		 *
		 * F_pll = F_VCO/PLLP
		 * F_pll = 200/2 = 100MHz
		 *
		 */

		//Primero limpiamos
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;
		//Asignamos los valores
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLM_3;											// 000101: PLLM = 8
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_2 |RCC_PLLCFGR_PLLN_5 |RCC_PLLCFGR_PLLN_6;	// 001100100: PLLN = 100 multiplica el valor resultante de la división
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;											// 00: PLLP = 2



		// Cambio el valor VOS (regulador de voltaje) para que se tenga el
		// rendimiento adecuado Esto se debe hacer antes de encender el PLL
		PWR->CR |= PWR_CR_VOS_0;
		PWR->CR |= (0x3<<PWR_CR_VOS_Pos);

		/*
		 * Programamos la latencia en el registro FLASH->ACR
		 * para estar acorde a la velocidad del procesador
		 * Programamos a 3WS de acuerdo al manual 90 =< HCLK =< 100
		 */

		FLASH->ACR &= ~FLASH_ACR_LATENCY;
		FLASH->ACR |= FLASH_ACR_LATENCY_3WS;

		//Encendemos el PLL
		RCC->CR |= RCC_CR_PLLON;

		//Esperamos que el clk se estabilice
		while(!(RCC->CR & RCC_CR_PLLRDY)){
			__NOP();
		}


		/*
		 * Configuramos el prescaler de ser necesario
		 * En este caso ya esta listo entonces le damos valor de 1
		 */
		RCC->CFGR &= ~(0xF << RCC_CFGR_HPRE_Pos);

		//Los perifericos que estan conectados al APB1 tienen una velocidad maxima de 50MHz
		//Dividimos los 100MHz/2 para tener 50MHz en el APB1
		RCC->CFGR &= ~RCC_CFGR_PPRE1;
		RCC->CFGR |= (0x4 << RCC_CFGR_PPRE1_Pos); //es lo mismo queRCC->CFGR |= RCC_CFGR_PPRE1_2;

		//El APB2 tiene una velocidad maxima de 100MHz
		//El APB2 soporta los 100MHz
		RCC->CFGR &= ~RCC_CFGR_PPRE2;

		//Hacer el cambio del sistema a que funcione en la nueva frecuencia
		RCC->CFGR &= ~RCC_CFGR_SW;
		RCC->CFGR |= RCC_CFGR_SW_1;
	}

}

uint8_t getConfigPLL(void){
	//revisamos el valor de los registros 2 y 3
	//Los desplazamos 2 unidades a la derecha para obtener sus valores 0 y 1
	uint32_t auxVar = (RCC->CFGR & 0b1100)>>2;

	switch (auxVar) {
		case HSI_CLOCK:
			return 0;
			break;

		case HSE_CLOCK:
			return 1;
			break;

		case PLL_CLOCK:
			return 2;
			break;

		default:
			return 0;
			break;
	}

}

uint16_t getFreqPLL(void){
	switch (getConfigPLL()) {
		case PLL_CLOCK:
			return (RCC->PLLCFGR & 0b111111111000000) >> RCC_PLLCFGR_PLLN_Pos;
			break;

		default:
		case HSI_CLOCK:
			return 16;
			break;
	}
}


void configMCO1(MCO1_Handler_t *ptrMCO1_Handler){


	//Seleccionamos el reloj HSI en el RCC_CFGR
	if (ptrMCO1_Handler->clk == HSI_CLOCK) {
		RCC->CFGR &= ~RCC_CFGR_MCO1;

		//SELECCIONAMOS EL PREESCALER PARA EL RELOJ ESCOGIDO
		if (ptrMCO1_Handler->psc == MCO1PRE_2) {
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_0;
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_1;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_2;
		}
		else if (ptrMCO1_Handler->psc == MCO1PRE_3) {
			RCC->CFGR |= RCC_CFGR_MCO1PRE_0;
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_1;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_2;

		}
		else if (ptrMCO1_Handler->psc == MCO1PRE_4) {
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_0;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_1;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_2;
		}
		else if (ptrMCO1_Handler->psc == MCO1PRE_5) {
			RCC->CFGR |= RCC_CFGR_MCO1PRE;
		}


	}
	//Seleccionamos el reloj LSE
	else if (ptrMCO1_Handler->clk == LSE_CLOCK) {
		RCC->CFGR |= RCC_CFGR_MCO1_0;
		RCC->CFGR &= ~RCC_CFGR_MCO1_1;

		//SELECCIONAMOS EL PREESCALER PARA EL RELOJ ESCOGIDO
		if (ptrMCO1_Handler->psc == MCO1PRE_2) {
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_0;
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_1;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_2;
		}
		else if (ptrMCO1_Handler->psc == MCO1PRE_3) {
			RCC->CFGR |= RCC_CFGR_MCO1PRE_0;
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_1;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_2;

		}
		else if (ptrMCO1_Handler->psc == MCO1PRE_4) {
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_0;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_1;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_2;
		}
		else if (ptrMCO1_Handler->psc == MCO1PRE_5) {
			RCC->CFGR |= RCC_CFGR_MCO1PRE;
		}

	}
	//Seleccionamos el reloj PLL
	else if (ptrMCO1_Handler->clk == PLL_CLOCK) {
		RCC->CFGR |= RCC_CFGR_MCO1;

		//SELECCIONAMOS EL PREESCALER PARA EL RELOJ ESCOGIDO
		if (ptrMCO1_Handler->psc == MCO1PRE_2) {
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_0;
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_1;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_2;
		}
		else if (ptrMCO1_Handler->psc == MCO1PRE_3) {
			RCC->CFGR |= RCC_CFGR_MCO1PRE_0;
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_1;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_2;

		}
		else if (ptrMCO1_Handler->psc == MCO1PRE_4) {
			RCC->CFGR &= ~RCC_CFGR_MCO1PRE_0;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_1;
			RCC->CFGR |= RCC_CFGR_MCO1PRE_2;
		}
		else if (ptrMCO1_Handler->psc == MCO1PRE_5) {
			RCC->CFGR |= RCC_CFGR_MCO1PRE;
		}

	}

}
