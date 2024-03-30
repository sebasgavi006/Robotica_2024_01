/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria Valencia
 * @brief          : Código de prueba para el ADC
 ******************************************************************************
 **/

#include <stdint.h>
#include <math.h>
#include "stm32f4xx.h"
//#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "adc_driver_hal.h"
#include "usart_driver_hal.h"
#include "pwm_driver_hal.h"

// Se define un pin para el Blinky
GPIO_Handler_t stateLed = {0}; // PinA5

Timer_Handler_t blinkTimer = {0}; // Timer 2 para el blinky
Timer_Handler_t pwmTimer = {0};

PWM_Handler_t pwmHandler = {0};
GPIO_Handler_t pinPWM = {0};

// Comunicación Rs-232 con el PC, ya habilitada en la board Nucleo 32F411
USART_Handler_t commSerial = {0}; // Pin para la transmisión serial
GPIO_Handler_t pinTx = {0};	// Pin para la transmisión serial
GPIO_Handler_t pinRx = {0};	// Pin para la recepción serial
uint8_t receivedChar = '\0';
uint8_t sendMsg = 0;
char bufferData[64] = {0};


// Headers de las funciones
void configPeripherals(void);


/* ===== Funciión principal del programa ===== */
int main(void){

	/* Cargamos la configuración de los periféricos */
	configPeripherals();


	/* Loop forever */
	while(1){
		if(sendMsg){
			sendMsg = 0; // Bajamos la bandera
			usart_WriteMsg(&commSerial, "Hello World! \n\r");
		}
		if(receivedChar){ // Si no está vacía la variable que almacena los datos recibidos
			if(receivedChar == 's'){
				usart_WriteMsg(&commSerial, "Empieza PWM\n\r");
				startPwmSignal(&pwmHandler);
			}

			if(receivedChar == 'p'){
				usart_WriteMsg(&commSerial, "Parar PWM\n\r");
				stopPwmSignal(&pwmHandler);

			}

			if(receivedChar == '1'){
				usart_WriteMsg(&commSerial, "Cambiando al 10%\n\r");
				updateDutyCycle(&pwmHandler, pwmHandler.config.periodo * 0.1);
			}

			if(receivedChar == '2'){
				usart_WriteMsg(&commSerial, "Cambiando al 50%\n\r");
				updateDutyCycle(&pwmHandler, pwmHandler.config.periodo * 0.5);

			}

			if(receivedChar == '3'){
				usart_WriteMsg(&commSerial, "Cambiando al 90%\n\r");
				updateDutyCycle(&pwmHandler, pwmHandler.config.periodo * 0.9);
			}


			receivedChar = '\0';
		}

	}

}	// Fin del main


/*
 * ===== IMPORTANTE ADC =====
 * A continuación se muestran los pines que corresponden a
 * los diferentes canales del módulo ADC (Consersión Análogo Digital)
 *
 * 	CANAL ADC	PIN			USART		TX		RX
 *
 *	ADC1_0		PA0			USART2		PA2		PA3
 * 	ADC1_1		PA1			USART6		PC6		PC7
 * 	ADC1_2		PA2			USART1		PA9		PA10
 * 	ADC1_3		PA3			USART6		PA11	PA12
 * 	ADC1_4		PA4			USART1		PA15
 * 	ADC1_5		PA5			USART2		PD5		PD6
 * 	ADC1_6		PA6			USART1				PB3
 * 	ADC1_7		PA7			USART1		PB6		PB7
 * 	ADC1_8		PB0
 * 	ADC1_9		PB1
 * 	ADC1_10		PC0
 * 	ADC1_11		PC1
 * 	ADC1_12		PC2
 * 	ADC1_13		PC3
 * 	ADC1_14		PC4
 * 	ADC1_15		PC5
 *
 */

/*
 * Definimos una función para inicializar las configuraciones
 * de los diferentes periféricos
 */
void configPeripherals(void){

	// 1. ===== PUERTOS Y PINES =====
	/* Configurando el pin para el Blinky */
	stateLed.pGPIOx								= GPIOA;
	stateLed.pinConfig.GPIO_PinNumber			= PIN_5;	// PinA5 -> Led2 del STM32F411
	stateLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	stateLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

	/* Cargamos la configuración del pin */
	gpio_Config(&stateLed);
	gpio_WritePin(&stateLed, SET);

	/* Configurando los pines para el puerto serial
	 *  - Usamos el PinA2 para TX
	 */
	pinTx.pGPIOx								= GPIOA;
	pinTx.pinConfig.GPIO_PinNumber				= PIN_2;
	pinTx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinTx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinTx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinTx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinTx);


	/* - Usamos el PinA3 para RX */
	pinRx.pGPIOx								= GPIOA;
	pinRx.pinConfig.GPIO_PinNumber				= PIN_3;
	pinRx.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinRx.pinConfig.GPIO_PinAltFunMode			= AF7;
	pinRx.pinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	pinRx.pinConfig.GPIO_PinOutputSpeed			= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinRx);


	/* - Usamos el  para el Canal del Timer del PWM */
	pinPWM.pGPIOx								= GPIOA;
	pinPWM.pinConfig.GPIO_PinNumber				= PIN_7;
	pinPWM.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	pinPWM.pinConfig.GPIO_PinAltFunMode			= AF2;
	pinPWM.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	pinPWM.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;

	/* Cargamos la configuración */
	gpio_Config(&pinPWM);


	// 2. ===== TIMERS =====
	/* Configurando el Timer del Blinky*/
	blinkTimer.pTIMx								= TIM2;
	blinkTimer.TIMx_Config.TIMx_Prescaler			= 16000;	// Genera incrementos de 1 ms
	blinkTimer.TIMx_Config.TIMx_Period				= 500;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500 ms)
	blinkTimer.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	blinkTimer.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción

	/* Cargamos la configuración del Timer */
	timer_Config(&blinkTimer);

	/* Encendemos el Timer */
	timer_SetState(&blinkTimer, TIMER_ON);

	// 4. ===== USARTS =====
	/* Configurando el puerto serial USART2 */
	commSerial.ptrUSARTx					= USART2;
	commSerial.USART_Config.baudrate		= USART_BAUDRATE_115200;
	commSerial.USART_Config.datasize		= USART_DATASIZE_8BIT;
	commSerial.USART_Config.parity			= USART_PARITY_NONE;
	commSerial.USART_Config.stopbits		= USART_STOPBIT_1;
	commSerial.USART_Config.mode			= USART_MODE_RXTX;
	commSerial.USART_Config.enableIntRX		= USART_RX_INTERRUP_ENABLE;

	/* Cargamos la configuración de USART */
	usart_Config(&commSerial);

	/*
	 * Escribimos el caracter nulo para asegurarnos de empezar
	 * una transmisión "limpia"
	 */
	usart_WriteChar(&commSerial, '\0');

	// 5. ===== PWM =====

	/* Configurando el PWM */
	pwmHandler.ptrTIMx					= TIM3; // Timer3 usado para el PWM
	pwmHandler.config.channel			= PWM_CHANNEL_2;
	pwmHandler.config.prescaler			= 16000; 	// 1 ms
	pwmHandler.config.periodo			= 500;		// 500 ms
	pwmHandler.config.dutyCycle			= 100;		// Activo por 5 ms

	/* Cargar la configuración del PWM */
	pwm_Config(&pwmHandler);

}	// Fin de la configuración de los periféricos



/*
 * Overwiter function
 */

void Timer2_Callback(void){
	gpio_TooglePin(&stateLed);
	sendMsg = 1; // Levantamos una bandera para enviar mensajes en el Callback del USART
}

void usart2_RxCallback(void){
	receivedChar = usart2_getRxData();
}


/*
 * Función assert para detectar problemas de paŕametros incorrectos
 */
void assert_failed(uint8_t* file, uint32_t line){
	while(1){
		// problems...
	}
}
