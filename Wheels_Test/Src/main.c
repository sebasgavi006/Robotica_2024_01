/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
#include <stdint.h>
#include "string.h"
#include <stdio.h>
#include <math.h>
#include "stm32f4xx.h"
//#include "stm32_assert.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "usart_driver_hal.h"
#include "pwm_driver_hal.h"
#include "exti_driver_hal.h"
#include "systick_driver_hal.h"
#include "pll_driver_hal.h"



// Se define un pin para el Blinky
GPIO_Handler_t stateLed = {0}; // PinA5
GPIO_Handler_t stateLedBoard = {0}; // PinA5

// Pines para el motor Derecho
GPIO_Handler_t GPIO_Enb_R = {0};
GPIO_Handler_t GPIO_Dir_R = {0};
GPIO_Handler_t GPIO_PWM_R = {0};

// Pines para el motor Izquierdo
GPIO_Handler_t GPIO_Enb_L = {0};
GPIO_Handler_t GPIO_Dir_L = {0};
GPIO_Handler_t GPIO_PWM_L = {0};

// Pines para interrupciones EXTI
GPIO_Handler_t GPIO_Exti_R = {0};
GPIO_Handler_t GPIO_Exti_L = {0};

// Handler Timers
Timer_Handler_t Tim_Blinky = {0};

// Handler PWM motores
PWM_Handler_t PWM_R = {0};
PWM_Handler_t PWM_L = {0};

// Estructura para la configuración de los EXTI
EXTI_Config_t Exti_R = {0};
EXTI_Config_t Exti_L = {0};

// Handler para el PLL
PLL_Config_t pllHandler = {0};

// Handler Systick
Systick_Handler_t SystickHandler= {0};

/*
 * Variables globales
 */
uint32_t SystemCoreClock = 100E6;
uint16_t counter_R = 0;
uint16_t counter_L = 0;

//USART
GPIO_Handler_t handlerPinTX		= {0};
GPIO_Handler_t handlerPinRX		= {0};
USART_Handler_t usart1Comm		= {0};


char bufferMsg[128] = {0};
char bufferReceiver[64] = {0};
uint8_t rxData = 0;
uint8_t counterReception = 0;
uint8_t stringComplete = 0;

// Variables para los comandos
char cmd[128] = {0};
unsigned int firstParameter = 0;
unsigned int secondParameter = 0;
char lastString[32] = {0};

// Variables globales para el funcionamiento del robot
uint8_t defaultSpeed = 0;
uint8_t counterPeriod = 0;
uint8_t flagPeriod = 0;
uint8_t counterPercDuty = 0;
uint8_t flagEncR = 0;
uint8_t flagEncL = 0;
uint8_t flagStop = 0;

// Funciones privadas
void initSystem(void);
void forwardMove(uint8_t dutyPercentage);
void backwardMove(uint8_t dutyPercentage);
void parseCommands(char  *ptrbufferReception);
void turnOff(void);
void turnOn(void);

/* ===== Función principal del programa ===== */
int main(void){


	// Configuramos el PLL para que el micro corra a 100MHz
	systemClock_100MHz(&pllHandler);
	systemClock_Output(); // el pin PC9 configurado como salida para leer la freq

	RCC->CR &= ~(RCC_CR_HSITRIM); // Limpiamos el registro
	RCC->CR |= (11 << RCC_CR_HSITRIM_Pos); // Numero para calibrar POR DEFECTO ESTABA EN 15!!!!!

	initSystem();
	turnOff();
//	systick_Delay_ms(SYSTICK_10s);
	sprintf(bufferMsg, "Saludos terrícolas, soy OPPY \n");
	usart_WriteMsg(&usart1Comm, bufferMsg);
//	forwardMove(1);
	counterPeriod = 0;
	counterPercDuty = 0;
	flagEncR = 0;
	flagEncL = 0;


	/* Loop forever */
	while(1){


		// Se revisa cual fue el dato recibido por la comunicacion serial
		if(rxData != '\0'){
			bufferReceiver[counterReception] = rxData;
			counterReception++;

			// Se verifica si el último dato es el caracter de finalizacion de un string
			if (rxData == '@'){
				// Se modifica el estado de una variable de control
				stringComplete = 1;

				// Configuramos las variables para guardar el string y esperar uno nuevo
				bufferReceiver[counterReception] = '\0';
				counterReception = 0;
			}

			// Limpiamos la variable que almacena los datos recibidos por el com. serial
			rxData = '\0';
		}
		else{
			// Aun no se han recibido caractéres a traves de la comunicacion serial
			stringComplete = 0;
		}


		if(stringComplete){
			parseCommands(bufferReceiver);
			stringComplete = 0;
		}

	}


}	// Fin del main



// Función para configurar los periféricos iniciales del sistemas
void initSystem(void){

	// 1. ===== PUERTOS Y PINES =====
	/* Configurando el pin para el Blinky */
	stateLed.pGPIOx								= GPIOA;
	stateLed.pinConfig.GPIO_PinNumber			= PIN_5;	// PinA5
	stateLed.pinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	stateLed.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLed.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLed.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	gpio_Config(&stateLed);

	/* Configurando el pin para el Blinky */
	stateLedBoard.pGPIOx							= GPIOC;
	stateLedBoard.pinConfig.GPIO_PinNumber			= PIN_5;	// PinA5
	stateLedBoard.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	stateLedBoard.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	stateLedBoard.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_MEDIUM;
	stateLedBoard.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	gpio_Config(&stateLedBoard);

	/* Configurando el pin para el Enable del puente H */
	GPIO_Enb_R.pGPIOx							= GPIOC;
	GPIO_Enb_R.pinConfig.GPIO_PinNumber			= PIN_10;
	GPIO_Enb_R.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	GPIO_Enb_R.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	GPIO_Enb_R.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_FAST;
	GPIO_Enb_R.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&GPIO_Enb_R);

	/* Configurando el pin para definir la dirección del motor DERECHO */
	GPIO_Dir_R.pGPIOx							= GPIOC;
	GPIO_Dir_R.pinConfig.GPIO_PinNumber			= PIN_12;
	GPIO_Dir_R.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	GPIO_Dir_R.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	GPIO_Dir_R.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_FAST;
	GPIO_Dir_R.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&GPIO_Dir_R);

	/* Pin de salida del PWM para el motor derecho. Usa el Timer5 */
	GPIO_PWM_R.pGPIOx								= GPIOA;
	GPIO_PWM_R.pinConfig.GPIO_PinNumber				= PIN_0;
	GPIO_PWM_R.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	GPIO_PWM_R.pinConfig.GPIO_PinAltFunMode			= AF2;
	GPIO_PWM_R.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	GPIO_PWM_R.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
	gpio_Config(&GPIO_PWM_R);

	/* Configurando el pin para el Enable del puente H */
	GPIO_Enb_L.pGPIOx							= GPIOC;
	GPIO_Enb_L.pinConfig.GPIO_PinNumber			= PIN_11;
	GPIO_Enb_L.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	GPIO_Enb_L.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	GPIO_Enb_L.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_FAST;
	GPIO_Enb_L.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&GPIO_Enb_L);

	/* Configurando el pin para definir la dirección del motor IZQUIERDO */
	GPIO_Dir_L.pGPIOx							= GPIOD;
	GPIO_Dir_L.pinConfig.GPIO_PinNumber			= PIN_2;
	GPIO_Dir_L.pinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	GPIO_Dir_L.pinConfig.GPIO_PinOutputType		= GPIO_OTYPE_PUSHPULL;
	GPIO_Dir_L.pinConfig.GPIO_PinOutputSpeed	= GPIO_OSPEED_FAST;
	GPIO_Dir_L.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&GPIO_Dir_L);

	/* Pin de salida del PWM para el motor izquierdo. Usa el Timer5 */
	GPIO_PWM_L.pGPIOx								= GPIOA;
	GPIO_PWM_L.pinConfig.GPIO_PinNumber				= PIN_1;
	GPIO_PWM_L.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	GPIO_PWM_L.pinConfig.GPIO_PinAltFunMode			= AF2;
	GPIO_PWM_L.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	GPIO_PWM_L.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
	gpio_Config(&GPIO_PWM_L);

	/* Pin del Encoder Derecho */
	GPIO_Exti_R.pGPIOx							= GPIOC;
	GPIO_Exti_R.pinConfig.GPIO_PinNumber		= PIN_1;
	GPIO_Exti_R.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	gpio_Config(&GPIO_Exti_R);

	/* Pin del Encoder Izquierdo */
	GPIO_Exti_L.pGPIOx							= GPIOC;
	GPIO_Exti_L.pinConfig.GPIO_PinNumber		= PIN_3;
	GPIO_Exti_L.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	gpio_Config(&GPIO_Exti_L);


	// 2. ===== TIMERS =====
	/* Configurando el Timer del Blinky*/
	Tim_Blinky.pTIMx								= TIM2;
	Tim_Blinky.TIMx_Config.TIMx_Prescaler			= 50E3;	// Genera incrementos de 1 ms. El micro está a 100MHz
	Tim_Blinky.TIMx_Config.TIMx_Period				= 2000;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500 ms)
	Tim_Blinky.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	Tim_Blinky.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción
	timer_Config(&Tim_Blinky);
	timer_SetState(&Tim_Blinky, TIMER_ON);


	// 3. ===== PWM =====

	/* Configurando el PWM para el motor DERECHO */
	PWM_R.ptrTIMx					= TIM5; // Timer5 usado para el PWM
	PWM_R.config.channel			= PWM_CHANNEL_1;
	PWM_R.config.prescaler			= 50E3; 	// 0.5 ms
	PWM_R.config.periodo			= 100;		// 50 ms
	PWM_R.config.percDuty			= 0;
	pwm_Config(&PWM_R);

	/* Configurando el PWM para el motor IZQUIERDO */
	PWM_L.ptrTIMx					= TIM5; // Timer5 usado para el PWM
	PWM_L.config.channel			= PWM_CHANNEL_2;
	PWM_L.config.prescaler			= 50E3; 	// 0.5 ms
	PWM_L.config.periodo			= 100;		// 50 ms
	PWM_L.config.percDuty			= 0;
	pwm_Config(&PWM_L);


	// 4. ====== EXTI =====
	/* Condigurando EXTI1 - Encoder Derecho */
	Exti_R.pGPIOHandler				= &GPIO_Exti_R;
	Exti_R.edgeType					= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_Config(&Exti_R);

	/* Condigurando EXTI3 - Encoder Izquierdo */
	Exti_L.pGPIOHandler				= &GPIO_Exti_L;
	Exti_L.edgeType					= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_Config(&Exti_L);

	// 5. ====== SYSTICK =====
	SystickHandler.pSystick 						= SYSTICK;
	SystickHandler.Systick_Config.Systick_Reload	= SYSTICK_PSC_1ms_100MHz;
	SystickHandler.Systick_Config.Systick_IntState 	= SYSTICK_INT_ENABLE;
	systick_Config(&SystickHandler);
	systick_SetState(&SystickHandler, SYSTICK_ON);


	/* ==================================== Configurando los USART =============================================*/
	handlerPinTX.pGPIOx										= GPIOA;
	handlerPinTX.pinConfig.GPIO_PinNumber					= PIN_9;
	handlerPinTX.pinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	handlerPinTX.pinConfig.GPIO_PinAltFunMode				= AF7;
	gpio_Config(&handlerPinTX);

	handlerPinRX.pGPIOx										= GPIOA;
	handlerPinRX.pinConfig.GPIO_PinNumber					= PIN_10;
	handlerPinRX.pinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	handlerPinRX.pinConfig.GPIO_PinAltFunMode				= AF7;
	gpio_Config(&handlerPinRX);

	usart1Comm.ptrUSARTx									= USART1;
	usart1Comm.USART_Config.baudrate						= USART_BAUDRATE_19200_100MHz;
	usart1Comm.USART_Config.datasize						= USART_DATASIZE_8BIT;
	usart1Comm.USART_Config.parity							= USART_PARITY_NONE;
	usart1Comm.USART_Config.stopbits						= USART_STOPBIT_1;
	usart1Comm.USART_Config.mode							= USART_MODE_RXTX;
	usart1Comm.USART_Config.enableIntRX						= USART_RX_INTERRUPT_ENABLE;
	usart1Comm.USART_Config.enableIntTX						= USART_TX_INTERRUPT_DISABLE;
	usart_Config(&usart1Comm);


}



// Función para mover hacia adelante

void forwardMove(uint8_t dutyPercentage){

	stopPwmSignal(&PWM_R);
	stopPwmSignal(&PWM_L);

	// Encendemos el puente H
	gpio_WritePin(&GPIO_Enb_R, RESET);	// Prende motor derecho
	gpio_WritePin(&GPIO_Enb_L, RESET);	// Apaga motor izquierdo

	// Seteamos la dirección del motor
	gpio_WritePin(&GPIO_Dir_R, SET);
	gpio_WritePin(&GPIO_Dir_L, RESET);


	PWM_R.config.polarity	= PWM_POLARITY_INVERSE;
	pwm_Config(&PWM_R);
	selectPolarity(&PWM_R);


	PWM_L.config.polarity	= PWM_POLARITY_DIRECT;
	pwm_Config(&PWM_L);
	selectPolarity(&PWM_L);

	updateDutyCycle(&PWM_R,dutyPercentage);
	updateDutyCycle(&PWM_L,dutyPercentage);

	// Encendemos el PWM para mover el motor derecho
	startPwmSignal(&PWM_R);
	startPwmSignal(&PWM_L);

}


void backwardMove(uint8_t dutyPercentage){

	stopPwmSignal(&PWM_R);
	stopPwmSignal(&PWM_L);

	// Encendemos el puente H
	gpio_WritePin(&GPIO_Enb_R, RESET);	// Prende motor derecho
	gpio_WritePin(&GPIO_Enb_L, RESET);	// Apaga motor izquierdo

	// Seteamos la dirección del motor
	gpio_WritePin(&GPIO_Dir_R, RESET);
	gpio_WritePin(&GPIO_Dir_L, SET);


	PWM_R.config.polarity	= PWM_POLARITY_DIRECT;
	pwm_Config(&PWM_R);
	selectPolarity(&PWM_R);


	PWM_L.config.polarity	= PWM_POLARITY_INVERSE;
	pwm_Config(&PWM_L);
	selectPolarity(&PWM_L);

	updateDutyCycle(&PWM_R,dutyPercentage);
	updateDutyCycle(&PWM_L,dutyPercentage);

	// Encendemos el PWM para mover el motor derecho
	startPwmSignal(&PWM_R);
	startPwmSignal(&PWM_L);

}

/*
 * Funcipon para apagar el sistema
 */
void turnOff(void){

	// Apaga el puente H para los motores
	gpio_WritePin(&GPIO_Enb_R, SET);
	gpio_WritePin(&GPIO_Enb_L, SET);

	// Apaga los PWM
	stopPwmSignal(&PWM_R);
	stopPwmSignal(&PWM_L);

}

/*
 * Funcipon para encender el sistema
 */
void turnOn(void){

	// Enciente el puente H de los motores
	gpio_WritePin(&GPIO_Enb_R, RESET);
	gpio_WritePin(&GPIO_Enb_L, RESET);

	// Enciente los PWM
	startPwmSignal(&PWM_R);
	startPwmSignal(&PWM_L);

}

void parseCommands(char  *ptrbufferReception){

	sscanf(ptrbufferReception,"%s %u %u %s",cmd,&firstParameter,&secondParameter,lastString);
	//Comando para solicitar ayuda
	if(strcmp(cmd, "help") == 0){
		usart_WriteMsg(&usart1Comm, "Help Menu CMDS: \n");
		usart_WriteMsg(&usart1Comm, "1) Dir 0:forw / 1:back ; dutty(\%) \" Dir # # @\" \n");
		usart_WriteMsg(&usart1Comm, "2) Spd \%leftM 		; \%rightM \" Spd # # @\" \n");
		usart_WriteMsg(&usart1Comm, "3) Rot 0:left 1:right  ; #turns  \" Rot # # @\" \n");
		usart_WriteMsg(&usart1Comm, "4) TestEncoders percDuttyCycle:left \" TestEncoders # @\" \n");

		usart_WriteMsg(&usart1Comm, "5) Test 0:left / 1:right; dutty   \" Test # # @\" \n");


		usart_WriteMsg(&usart1Comm, "6) Stop \" Stop @\" \n");
		usart_WriteMsg(&usart1Comm, "7) Resume \" Resume @\" \n");

	}

	else if (strcmp(cmd, "Dir") == 0) {

		// firstParameter indica la direccion, secondParameter es el dutyCycle
		if (firstParameter == 0 && secondParameter >= 0){
			if (defaultSpeed == 0){
				forwardMove(secondParameter);
			}
			else{
				forwardMove(10);
			}
			usart_WriteMsg(&usart1Comm, "Moviéndose hacia adelante \n");
		}
		else if (firstParameter == 1 && secondParameter >= 0){
			if (defaultSpeed == 0){
				backwardMove(secondParameter);
			}
			else{
				backwardMove(10);
			}
			usart_WriteMsg(&usart1Comm, "Moviéndose hacia atrás \n");
		}
		defaultSpeed = 0;
	}

	else if(strcmp(cmd, "Spd") == 0) {
			if (firstParameter > 0) {

				defaultSpeed = firstParameter;
				updateDutyCycle(&PWM_L,(uint16_t)firstParameter);
				updateDutyCycle(&PWM_R,(uint16_t)secondParameter);

				sprintf(bufferMsg,"Velocidad actualizada: %u, %u \n",firstParameter, secondParameter);
				usart_WriteMsg(&usart1Comm, bufferMsg);
			}
			else{
				usart_WriteMsg(&usart1Comm, "Porcentaje debe ser positivo.\n Ingresa \"help @\" para ver la lista de comandos.\n");
			}
	}

	else if(strcmp(cmd, "TestE") == 0) {

		usart_WriteMsg(&usart1Comm, "Iniciando Test Encoders\n");
		forwardMove(firstParameter);

		rxData = '\0';
		// Conteo y muestra de las interrupciones del encoder
		while(rxData == '\0'){
//			if(flagEncR){
//				sprintf(bufferMsg,"Right,%u\n", counter_R);
//				usart_WriteMsg(&usart1Comm, bufferMsg);
//				flagEncR = 0;
//				rxData = '\0';
//			}
//			if(flagEncL){
//				sprintf(bufferMsg,"Left,%u\n", counter_L);
//				usart_WriteMsg(&usart1Comm, bufferMsg);
//				flagEncL = 0;
//				rxData = '\0';
//			}
			if(flagEncR || flagEncL){
				sprintf(bufferMsg,"%.2f%% \t  %u \t %u \t \n",counterPercDuty, counter_R,counter_L);
				usart_WriteMsg(&usart1Comm, bufferMsg);
				flagEncR = 0;
				flagEncL = 0;
			}

		}
	}

	else if(strcmp(cmd, "Test") == 0){

		usart_WriteMsg(&usart1Comm, "Iniciando Test \n");
		forwardMove(0);
		counter_R = 0;
		counter_L = 0;
		counterPercDuty = 0;

		rxData = '\0';
		// Código para realizar el estudio del comportamiento de los motores y los encoders
		while(rxData == '\0'){

			// Conteo y muestra de las interrupciones del encoder
			if(flagEncR){
				sprintf(bufferMsg,"\nRight,%u,%u\n",counterPercDuty, counter_R);
				usart_WriteMsg(&usart1Comm, bufferMsg);
				flagEncR ^= 1;
			}
			if(flagEncL){
				sprintf(bufferMsg,"\nLeft,%u,%u\n",counterPercDuty, counter_L);
				usart_WriteMsg(&usart1Comm, bufferMsg);
				flagEncL ^= 1;
			}

			// Cada que pase un periodo determinado, el porcentaje del CutyCycle aumenta en 1%
			if(flagPeriod){
				counter_R = 0;
				counter_L = 0;
				counterPercDuty++;
				updateDutyCycle(&PWM_R,(uint16_t) counterPercDuty);
				updateDutyCycle(&PWM_L,(uint16_t) counterPercDuty);
				flagPeriod ^= 1;
			}
			if(counterPercDuty == 99){
				counterPercDuty = 0;
				turnOff();
				usart_WriteMsg(&usart1Comm, "Test finished \n");
			}
		}

	}


	else if(strcmp(cmd, "Period") == 0) {
			if (firstParameter > 0) {


				PWM_L.config.periodo			= firstParameter;		// 500 ms
				pwm_Config(&PWM_L);

				PWM_R.config.periodo			= firstParameter;		// 500 ms
				pwm_Config(&PWM_R);

				sprintf(bufferMsg,"Periodo actualizado: %u \n",firstParameter);
				usart_WriteMsg(&usart1Comm, bufferMsg);
			}
			else{
				usart_WriteMsg(&usart1Comm, "Periodo debe ser positivo.\n Ingresa \"help @\" para ver la lista de comandos.\n");
			}
	}




	else if (strcmp(cmd, "reset") == 0) {
		usart_WriteMsg(&usart1Comm, "PWR_MGMT_1 reset \n");

	}

	else if (strcmp(cmd, "Stop") == 0) {
		flagStop = 1;
		turnOff();
		usart_WriteMsg(&usart1Comm, "Detiene del sistema \n");
	}

	else if (strcmp(cmd, "Resume") == 0) {
		turnOn();
		usart_WriteMsg(&usart1Comm, "Reanuda del sistema \n");
	}

	else{
		usart_WriteMsg(&usart1Comm, "Comando erroneo.\n Ingresa \"help @\" para ver la lista de comandos.\n");
	}


}


/* Callback de Timer 2 para el Blinky */
void Timer2_Callback(void){
	gpio_TooglePin(&stateLed);
	gpio_TooglePin(&stateLedBoard);
	counterPeriod++;
	if(counterPeriod == 10){
		flagPeriod ^= 1;
		counterPeriod = 0;
	}
}


/* Interrupciones por recepcion a traves de transmision serial */
void usart1_RxCallback(void){
	rxData = usart1_getRxData();

}


/* Interrupciones para el encoder derecho */
void callback_ExtInt1(void){
	flagEncR = 1;
	counter_R++;
}


/* Interrupciones para el encoder izquierdo */
void callback_ExtInt3(void){
	flagEncL = 1;
	counter_L++;

}

