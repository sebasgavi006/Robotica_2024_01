/**
 ******************************************************************************
 * @file           : mpu.c
 * @author         : Jhony Aristizabal, Sebastian Gaviria
 * @brief          : Programa que implementa un PID basado en datos de velocidad
 * 					 y rotación del robot para el control de movimiento del mismo
 * 					 con el objetivo de lograr el recorrido automático de una
 * 					 trayectoria definida.
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

#include "mpu6050_driver.h"
#include "i2c_driver_hal.h"



// Se define un pin para el Blinky
GPIO_Handler_t stateLed = {0}; // PinA5
GPIO_Handler_t stateLedBoard = {0}; // PinC5

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

// Handler del Timer
Timer_Handler_t Tim_Blinky = {0};

// Handler para el PLL
PLL_Handler_t pllHandler = {0};

// Handler PWM motores
PWM_Handler_t PWM_R = {0};
PWM_Handler_t PWM_L = {0};

// Estructura para la configuración de los EXTI
EXTI_Config_t Exti_R = {0};
EXTI_Config_t Exti_L = {0};


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
char cmd[64] = {0};
float firstParameter = 0;
float secondParameter = 0;
char lastString[64] = {0};

// Contadores ruedas
// Variables globales para el funcionamiento del robot
uint8_t defaultSpeed = 0;
uint8_t counterPeriodTest = 0;
uint8_t counterPercDuty = 0;
uint8_t flagEncR = 0;
uint8_t flagEncL = 0;
uint8_t flagStop = 0;
uint8_t flagPeriod = 0;
uint8_t flagTimer = 0;


float diameterWheel = 23.5 * 2; // Diámetro en mm

float percDutyR = 0;
float percDutyL = 0;


// Constantes de Tuning del PID
float kp, ki, kd = 0;
uint32_t currTime, prevTime = 0;
float prevError, devError, integralError = 0;
float deltaError, deltaTime = 0;
float u_PID = 0;


// Contadores
uint16_t counterBlinky = 0;
uint16_t counterIMU = 0;
uint32_t counterMicros = 0;
uint16_t LimitBlinky = 0;
uint16_t LimitGyro = 0;
uint8_t flagPID = 0;

uint8_t periodBlinky = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////
I2C_Handler_t imuHandler = {0};
GPIO_Handler_t imuSDA = {0};
GPIO_Handler_t imuSCL = {0};

char bufferData[64] = "Accel MPU6050 testing...";

float accelData[3] = {0};

float gyroData[3] = {0};
float calibGyroData[3] = {0};

int nReadings = 300;
extern float offsetGyroData[3];

float temp = 0;

float previousGyroZ = 0.0;
float currentGyroZ = 0.0;

float yaw_gyro = 0.0;       // Ángulo calculado con el acelerómetro
float dt = 0.2;           // Intervalo de tiempo (20 ms)


///////////////////////////////////////////////////////////////////////////////////////////////////////

// Funciones privadas
void initSystem(void);
void parseCommands(char  *ptrbufferReception);
void forwardMove(float percDutyL, float percDutyR);
void backwardMove(float percDutyL, float percDutyR);
void setCounts(float *percDutyR, float *percDutyL, uint16_t counts , float deltaDuty);
void turnOff(void);
void turnOn(void);
void PID(PWM_Handler_t *PWM_handler, uint16_t target, uint16_t measure);

void manageCounters(void);
void yawIntegral(void);


/* ===== Función principal del programa ===== */
int main(void){

	SCB->CPACR |= 0xf<<20;

	// Configuramos el PLL para que el micro corra a 100MHz
//	systemClock_100MHz(&pllHandler);
//	systemClock_Output(); // el pin PC9 configurado como salida para leer la freq

//	RCC->CR &= ~(RCC_CR_HSITRIM); // Limpiamos el registro
//	RCC->CR |= (11 << RCC_CR_HSITRIM_Pos); // Numero para calibrar POR DEFECTO ESTABA EN 15!!!!!

	initSystem();
	turnOff();
	sprintf(bufferMsg, "Saludos terrícolas, soy OPPY \n");
	usart_WriteMsg(&usart1Comm, bufferMsg);
	counterPeriodTest = 0;
	counterPercDuty = 0;
	percDutyR = 0;
	percDutyL = 0;
	flagEncR = 0;
	flagEncL = 0;
	flagPeriod = 0;
	flagTimer = 0;
	flagPID = 0;

	// Periodos del Blinky y del Giroscopio
	LimitBlinky = 50E3;
	LimitGyro = 20E3;

	// Se configura inicialmente el MPU
	config_SysTick_ms(HSI_CLOCK_CONFIGURED);
	delay_ms(1);
	imuBegin(&imuHandler);
	delay_ms(1);
	imuBegin(&imuHandler);
	//verificamos WhoAmI
	uint8_t WhoAmI = imuWhoAmI(&imuHandler);
	sprintf(bufferData, "WHOAMI = %u \n", WhoAmI);
	usart_WriteMsg(&usart1Comm, bufferData);

	setAccelRange(&imuHandler, ACCEL_RANGE_2_G);
	setGyroRange(&imuHandler, GYRO_RANGE_500_DEG);
	setAccelDLPF(&imuHandler, DLPF_260HZ);

	sprintf(bufferMsg, "Saludos terricolas, soy OPPY \n");
	usart_WriteMsg(&usart1Comm, bufferMsg);

	usart_WriteMsg(&usart1Comm, "Please dont move the sensor. Calibration Starting...\n");
	sprintf(bufferMsg,"Offset values  %.2f,%.2f,%.2f\n",offsetGyroData[0],offsetGyroData[1],offsetGyroData[2]);
	usart_WriteMsg(&usart1Comm, bufferMsg);
	gyroStaticCalibration(&imuHandler,nReadings);
	sprintf(bufferMsg,"Offset values  %.2f,%.2f,%.2f\n",offsetGyroData[0],offsetGyroData[1],offsetGyroData[2]);
	usart_WriteMsg(&usart1Comm, bufferMsg);
	usart_WriteMsg(&usart1Comm, "Calibration finished\n");

	/* Loop forever */
	while (1) {

		// Función que maneja todos los conteos de tiempo basados en interrupciones del Timer2
		//manageCounters();

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

	} // Fin del while

	return 0;


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


	// 2. ===== GPIOs =====
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
	/* Configurando el Timer del Blinky */
	Tim_Blinky.pTIMx								= TIM2;
	Tim_Blinky.TIMx_Config.TIMx_Prescaler			= 100;	// Genera incrementos de 10 us. El micro está a 100MHz
	Tim_Blinky.TIMx_Config.TIMx_Period				= 10;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500ms)
	Tim_Blinky.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	Tim_Blinky.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción
	timer_Config(&Tim_Blinky);
	timer_SetState(&Tim_Blinky, TIMER_ON);

	/* ==================================== PLL =============================================*/

	pllHandler.clkSpeed = FREQUENCY_100MHz;
	//Calibramos el clock
	RCC->CR &= ~RCC_CR_HSITRIM;			//Limpiamos el registro
	RCC->CR |= (12<<RCC_CR_HSITRIM_Pos); // Numero para calibrar POR DEFECTO ESTABA EN 15!!!!!
	configPLL(&pllHandler);

	/* ==================================== Configurando los USART =============================================*/
//	handlerPinTX.pGPIOx										= GPIOA;
//	handlerPinTX.pinConfig.GPIO_PinNumber					= PIN_2;
//	handlerPinTX.pinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
//	handlerPinTX.pinConfig.GPIO_PinAltFunMode				= AF7;
//	gpio_Config(&handlerPinTX);
//
//	handlerPinRX.pGPIOx										= GPIOA;
//	handlerPinRX.pinConfig.GPIO_PinNumber					= PIN_3;
//	handlerPinRX.pinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
//	handlerPinRX.pinConfig.GPIO_PinAltFunMode				= AF7;
//	gpio_Config(&handlerPinRX);
//
//	usart1Comm.ptrUSARTx									= USART2;
//	usart1Comm.USART_Config.baudrate						= USART_BAUDRATE_9600;
//	usart1Comm.USART_Config.datasize						= USART_DATASIZE_8BIT;
//	usart1Comm.USART_Config.parity							= USART_PARITY_NONE;
//	usart1Comm.USART_Config.stopbits						= USART_STOPBIT_1;
//	usart1Comm.USART_Config.mode							= USART_MODE_RXTX;
//	usart1Comm.USART_Config.enableIntRX						= USART_RX_INTERRUPT_ENABLE;
//	usart1Comm.USART_Config.enableIntTX						= USART_TX_INTERRUPT_DISABLE;
//	usart_Config(&usart1Comm);

	/*USART 1 -> Comunicación serial a través de la antena */
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
	usart1Comm.USART_Config.baudrate						= USART_BAUDRATE_100MHz_19200;
	usart1Comm.USART_Config.datasize						= USART_DATASIZE_8BIT;
	usart1Comm.USART_Config.parity							= USART_PARITY_NONE;
	usart1Comm.USART_Config.stopbits						= USART_STOPBIT_1;
	usart1Comm.USART_Config.mode							= USART_MODE_RXTX;
	usart1Comm.USART_Config.enableIntRX						= USART_RX_INTERRUPT_ENABLE;
	usart1Comm.USART_Config.enableIntTX						= USART_TX_INTERRUPT_DISABLE;
	usart_Config(&usart1Comm);



	// 3. ===== PWM =====
	/* Configurando el PWM para el motor DERECHO */
	PWM_R.ptrTIMx					= TIM5; // Timer5 usado para el PWM
	PWM_R.config.channel			= PWM_CHANNEL_1;
	PWM_R.config.prescaler			= 50E2; 	// 0.05 ms
	PWM_R.config.periodo			= 1000;		// 50 ms -> Frec. de 20 Hz
	PWM_R.config.percDuty			= 0;
	pwm_Config(&PWM_R);

	/* Configurando el PWM para el motor IZQUIERDO */
	PWM_L.ptrTIMx					= TIM5; // Timer5 usado para el PWM
	PWM_L.config.channel			= PWM_CHANNEL_2;
	PWM_L.config.prescaler			= 50E2; 	// 0.05 ms
	PWM_L.config.periodo			= 1000;		// 50 ms -> Frec. de 20 Hz
	PWM_L.config.percDuty			= 0;
	pwm_Config(&PWM_L);


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	imuSDA.pGPIOx								= GPIOC;
	imuSDA.pinConfig.GPIO_PinNumber				= PIN_9;
	imuSDA.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	imuSDA.pinConfig.GPIO_PinOutputType			= GPIO_OTYPE_OPENDRAIN;
	imuSDA.pinConfig.GPIO_PinAltFunMode			= AF4;
	imuSDA.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	imuSDA.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
	gpio_Config(&imuSDA);

	imuSCL.pGPIOx								= GPIOA;
	imuSCL.pinConfig.GPIO_PinNumber				= PIN_8;
	imuSCL.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
	imuSCL.pinConfig.GPIO_PinOutputType			= GPIO_OTYPE_OPENDRAIN;
	imuSCL.pinConfig.GPIO_PinAltFunMode			= AF4;
	imuSCL.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_PULLUP;
	imuSCL.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
	gpio_Config(&imuSCL);


	imuHandler.slaveAddress = MPU6050_ADDRESS; //105;
	imuHandler.ptrI2Cx		= I2C3;
	imuHandler.modeI2C		= I2C_MODE_SM;
	imuHandler.mainClock	= MAIN_CLOCK_100_MHz_FOR_I2C;

	i2c_Config(&imuHandler);

//	imuSDA.pGPIOx								= GPIOB; // Lado izquierdo del micro
//	imuSDA.pinConfig.GPIO_PinNumber				= PIN_7;
//	imuSDA.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
//	imuSDA.pinConfig.GPIO_PinOutputType			= GPIO_OTYPE_OPENDRAIN;
//	imuSDA.pinConfig.GPIO_PinAltFunMode			= AF4;
//	imuSDA.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
//	imuSDA.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
//	gpio_Config(&imuSDA);
//
//	imuSCL.pGPIOx								= GPIOB; // Lado derecho del micro
//	imuSCL.pinConfig.GPIO_PinNumber				= PIN_6;
//	imuSCL.pinConfig.GPIO_PinMode				= GPIO_MODE_ALTFN;
//	imuSCL.pinConfig.GPIO_PinOutputType			= GPIO_OTYPE_OPENDRAIN;
//	imuSCL.pinConfig.GPIO_PinAltFunMode			= AF4;
//	imuSCL.pinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_PULLUP;
//	imuSCL.pinConfig.GPIO_PinOutputSpeed		= GPIO_OSPEED_FAST;
//	gpio_Config(&imuSCL);
//
//
//	imuHandler.slaveAddress = MPU6050_ADDRESS;
//	imuHandler.ptrI2Cx		= I2C1;
//	imuHandler.modeI2C		= I2C_MODE_SM;
//	imuHandler.mainClock	= MAIN_CLOCK_100_MHz_FOR_I2C;
//
//	i2c_Config(&imuHandler);


	///////////////////////////////////////////////////////////////////////////////////////////////////////

} // Fin initSystem()


/* Función para los comandos */
void parseCommands(char  *ptrbufferReception){

	sscanf(ptrbufferReception,"%s %f %f %s",cmd,&firstParameter,&secondParameter,lastString);
	//Comando para solicitar ayuda
	if(strcmp(cmd, "help") == 0){
		usart_WriteMsg(&usart1Comm, "Help Menu CMDS: \n");
		usart_WriteMsg(&usart1Comm, "1) Dir 0:forw / 1:back ; dutty(\%) \" Dir # # @\" \n");
		usart_WriteMsg(&usart1Comm, "2) Spd \%leftM 		; \%rightM \" Spd # # @\" \n");
		usart_WriteMsg(&usart1Comm, "3) Cuentas dutty(\%) \" Cuentas (#) @\" \n");

		usart_WriteMsg(&usart1Comm, "4) TestEncoders percDuttyCycle:left \" TestEncoders # @\" \n");
		usart_WriteMsg(&usart1Comm, "5) Test 0:left / 1:right; dutty   \" Test # # @\" \n");

		usart_WriteMsg(&usart1Comm, "6) RotLecture -> Datos de rotación actual \" RotLecture @\" \n");

		usart_WriteMsg(&usart1Comm, "7) Ajuste Cuentas (#) ; deltaDuty (float) @ \n");

		usart_WriteMsg(&usart1Comm, "8) PID_Count Target (#) @ \n");

		usart_WriteMsg(&usart1Comm, "9) PID_Rot Target (#) @ \n");

		usart_WriteMsg(&usart1Comm, "10) PID Target (#) @ \n");

		usart_WriteMsg(&usart1Comm, "11) Stop \" Stop @\" \n");
		usart_WriteMsg(&usart1Comm, "12) Resume \" Resume @\" \n");

	}

	// Opción 1) Dir
	else if (strcmp(cmd, "Dir") == 0) {

		// firstParameter indica la direccion, secondParameter es el dutyCycle
		if (firstParameter == 0 && secondParameter >= 0){
			if (defaultSpeed == 0){
				forwardMove(secondParameter, secondParameter);
			}
			else{
				forwardMove(10, 10);
			}
			usart_WriteMsg(&usart1Comm, "Moviéndose hacia adelante \n");
		}
		else if (firstParameter == 1 && secondParameter >= 0){
			if (defaultSpeed == 0){
				backwardMove(secondParameter, secondParameter);
			}
			else{
				backwardMove(10, 10);
			}
			usart_WriteMsg(&usart1Comm, "Moviéndose hacia atrás \n");
		}
		defaultSpeed = 0;
	}

	// Opción 2) Spd
	else if(strcmp(cmd, "Spd") == 0) {
			if (firstParameter > 0) {

				percDutyL = firstParameter;
				percDutyR = secondParameter;

				defaultSpeed = firstParameter;
				updateDutyCycle(&PWM_L,(uint16_t)firstParameter);
				updateDutyCycle(&PWM_R,(uint16_t)secondParameter);

				sprintf(bufferMsg,"Velocidad actualizada: %.2f, %.2f \n",firstParameter, secondParameter);
				usart_WriteMsg(&usart1Comm, bufferMsg);
			}
			else{
				usart_WriteMsg(&usart1Comm, "Porcentaje debe ser positivo.\n Ingresa \"help @\" para ver la lista de comandos.\n");
			}
	}

	// Opción 3) Cuentas
	else if (strcmp(cmd, "Cuentas") == 0) {

		if (firstParameter > 0 && secondParameter > 0) {

			sprintf(bufferMsg,"Iniciando conteo \n");
			usart_WriteMsg(&usart1Comm, bufferMsg);

			counter_R = 0;
			counter_L = 0;
			flagEncR = 1;
			flagEncR = 1;

			forwardMove(firstParameter, firstParameter);

			rxData = '\0';


			while(rxData == '\0'){

				if(counter_R >= secondParameter){

					// Apaga el puente H para los motores
					//gpio_WritePin(&GPIO_Enb_R, SET);

					// Apaga los PWM
					stopPwmSignal(&PWM_R);

					sprintf(bufferMsg,"Conteo Encoder Derecho: %u \n",counter_R);
					usart_WriteMsg(&usart1Comm, bufferMsg);
					counter_R = 0;
					flagEncR = 0;
				}

				if(counter_L >= secondParameter){

					// Apaga el puente H para los motores
					//gpio_WritePin(&GPIO_Enb_L, SET);

					// Apaga los PWM
					stopPwmSignal(&PWM_L);


					sprintf(bufferMsg,"Conteo Encoder Izquierdo: %u \n",counter_L);
					usart_WriteMsg(&usart1Comm, bufferMsg);
					counter_L = 0;
					flagEncL = 0;
				}
			}


			flagEncR = 1;
			flagEncL = 1;
			sprintf(bufferMsg,"Conteo realizado \n");
			usart_WriteMsg(&usart1Comm, bufferMsg);
		}
		else{
			usart_WriteMsg(&usart1Comm, "El valor debe ser positivo.\n Ingresa \"help @\" para ver la lista de comandos.\n");
		}
	}

	// Opción 4) TestEncoders
	else if(strcmp(cmd, "TestEncoders") == 0) {

		usart_WriteMsg(&usart1Comm, "Iniciando Test Encoders\n");
		forwardMove(firstParameter,firstParameter);

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
				sprintf(bufferMsg,"%u \t  %u \t %u \t \n",counterPercDuty, counter_R,counter_L);
				usart_WriteMsg(&usart1Comm, bufferMsg);
				flagEncR = 0;
				flagEncL = 0;
			}

		}
	}

	// Opción 5) Test
	else if(strcmp(cmd, "Test") == 0){

		usart_WriteMsg(&usart1Comm, "Iniciando Test \n");
		forwardMove(0,0);
		counter_R = 0;
		counter_L = 0;
		counterPercDuty = 0;

		rxData = '\0';
		// Código para realizar el estudio del comportamiento de los motores y los encoders
		while(rxData == '\0'){

			if(counterPeriodTest == 20){
				flagPeriod ^= 1;
				counterPeriodTest = 0;
			}

			// Cada que pase un periodo determinado, el porcentaje del CutyCycle aumenta en 1%
			if(flagPeriod){

				sprintf(bufferMsg,"%u \t %u \t %u \n",counter_L,counter_R, counterPercDuty);

				usart_WriteMsg(&usart1Comm, bufferMsg);

				counter_R = 0;
				counter_L = 0;
				counterPercDuty++;
				updateDutyCycle(&PWM_R, counterPercDuty);
				updateDutyCycle(&PWM_L, counterPercDuty);
				flagPeriod ^= 1;
			}
			if(counterPercDuty == 99){
				counterPercDuty = 0;
				turnOff();
				usart_WriteMsg(&usart1Comm, "Test finished \n");
			}
		}

	}

	// Opción 6) Rot
	else if(strcmp(cmd, "RotLecture") == 0){

		usart_WriteMsg(&usart1Comm, "Mostrando valores de rotación \n");

		rxData = '\0';
		while(rxData == '\0'){

			// Se generan las lecturas del giroscopio
			if (counterIMU > LimitGyro) {
				yawIntegral();
				counterIMU = 0;
			}
		}
	}

	// Opción 7) Ajuste
	else if(strcmp(cmd, "Ajuste") == 0){

		usart_WriteMsg(&usart1Comm, "Realizando Ajuste \n");

		if (firstParameter > 0 && secondParameter > 0){

			forwardMove(percDutyL, percDutyR);

			rxData = '\0';
			while(rxData == '\0'){
				setCounts(&percDutyR, &percDutyL, (uint16_t)firstParameter, secondParameter);
			}
		}
		else{
			usart_WriteMsg(&usart1Comm, "Los valores deben ser positivos.\n Ingresa \"help @\" para ver la lista de comandos.\n");
		}

	}


	else if(strcmp(cmd, "Freq") == 0) {
			if (firstParameter > 0) {

				updateFrequency(&PWM_L, firstParameter);
				updateFrequency(&PWM_R, firstParameter);



				sprintf(bufferMsg,"Frecuencia actualizado: %.2f \n",firstParameter);
				usart_WriteMsg(&usart1Comm, bufferMsg);
			}
			else{
				usart_WriteMsg(&usart1Comm, "La Frecuencia debe ser positiva.\n Ingresa \"help @\" para ver la lista de comandos.\n");
			}
	}


	else if(strcmp(cmd, "Period") == 0) {
			if (firstParameter > 0) {

				updatePeriod(&PWM_L, firstParameter);
				updatePeriod(&PWM_R, firstParameter);



				sprintf(bufferMsg,"Periodo actualizado: %.2f \n",firstParameter);
				usart_WriteMsg(&usart1Comm, bufferMsg);
			}
			else{
				usart_WriteMsg(&usart1Comm, "Periodo debe ser positivo.\n Ingresa \"help @\" para ver la lista de comandos.\n");
			}
	}

/* ==================== PID ==================== */

	// Opción ) PID Cuentas
	else if(strcmp(cmd, "PID_Count") == 0){

		usart_WriteMsg(&usart1Comm, "Iniciando PID \n");

		if (firstParameter > 0){

			// Fijamos los valores del dutycycle que estabilizan la velocidad de las ruedas
			percDutyL = 20;
			percDutyR = 32;

			forwardMove(percDutyL, percDutyR);

			// Establecemos el daltaTime con base a la frecuencia de muestreo del ángulo
			deltaTime = LimitBlinky;

			rxData = '\0';
			while(rxData == '\0'){
				//setCounts(&percDutyR, &percDutyL, (uint16_t)firstParameter, secondParameter);
				flagPID = 1;
				PID(&PWM_R, firstParameter, counter_R);
				flagPID = 1;
				PID(&PWM_L, firstParameter, counter_L);
				counter_R = 0;
				counter_L = 0;
			}
		}
		else{
			usart_WriteMsg(&usart1Comm, "Los valores deben ser positivos.\n Ingresa \"help @\" para ver la lista de comandos.\n");
		}

	}


	// Opción ) PID Angulo
	else if(strcmp(cmd, "PID_Rot") == 0){

		usart_WriteMsg(&usart1Comm, "Iniciando PID \n");

		if (firstParameter > 0){

			// Fijamos los valores del dutycycle que estabilizan la velocidad de las ruedas
			percDutyL = 20;
			percDutyR = 32;

			forwardMove(percDutyL, percDutyR);

			// Establecemos el daltaTime con base a la frecuencia de muestreo del ángulo
			deltaTime = LimitGyro;

			rxData = '\0';
			while(rxData == '\0'){

				// Se generan las lecturas del giroscopio
				if (counterIMU > LimitGyro) {
					yawIntegral();
					// Realiza el PID y ajuste los 	PWM de los motores
					flagPID = 1;
					PID(&PWM_R, firstParameter, yaw_gyro);
					flagPID = 1;
					PID(&PWM_L, firstParameter, yaw_gyro);
					counterIMU = 0;
				}


			}
		}
		else{
			usart_WriteMsg(&usart1Comm, "Los valores deben ser positivos.\n Ingresa \"help @\" para ver la lista de comandos.\n");
		}

	}


/* ==================== PID ==================== */

	// Opción 11) Stop
	else if (strcmp(cmd, "Stop") == 0) {
		flagStop = 1;
		turnOff();
		usart_WriteMsg(&usart1Comm, "Detiene del sistema \n");
	}

	// Opción 12) Resume
	else if (strcmp(cmd, "Resume") == 0) {
		turnOn();
		usart_WriteMsg(&usart1Comm, "Reanuda del sistema \n");
	}

	// Opción para resetear en cualquier momento
	else if (strcmp(cmd, "reset") == 0) {
		usart_WriteMsg(&usart1Comm, "PWR_MGMT_1 reset \n");

	}

	else{
		usart_WriteMsg(&usart1Comm, "Comando erroneo.\n Ingresa \"help @\" para ver la lista de comandos.\n");
	}


}


/* Función para ajustar automáticamente el movimiento de los motores */
void setCounts(float *percDutyR, float *percDutyL, uint16_t counts , float deltaDuty){

	while(flagTimer){

		if((counter_R != counts) || (counter_L != counts)){

			// Control del motor derecho
			if(counter_R < counts){

				*percDutyR = *percDutyR+deltaDuty;
				updateDutyCycle(&PWM_R, *percDutyR);

				sprintf(bufferMsg,"Aum. duty der.: %.2f \n", *percDutyR);
				usart_WriteMsg(&usart1Comm, bufferMsg);

				counter_R = 0;

			}
			else if(counter_R > counts){

				*percDutyR = *percDutyR-deltaDuty;
				updateDutyCycle(&PWM_R, *percDutyR);

				sprintf(bufferMsg,"Dism. duty der.: %.2f \n",*percDutyR);
				usart_WriteMsg(&usart1Comm, bufferMsg);

				counter_R = 0;

			}
			else{
				updateDutyCycle(&PWM_R, *percDutyR);

				counter_R = 0;

			}

			// Control del motor izquierdo
			if(counter_L < counts){

				*percDutyL = *percDutyL+deltaDuty;
				updateDutyCycle(&PWM_L, *percDutyL);

				sprintf(bufferMsg,"Aum. duty izq.: %.2f \n",*percDutyL);
				usart_WriteMsg(&usart1Comm, bufferMsg);

				counter_L = 0;

			}
			else if(counter_L > counts){

				*percDutyL = *percDutyL-deltaDuty;
				updateDutyCycle(&PWM_L, *percDutyL);

				sprintf(bufferMsg,"Dism. duty izq.: %.2f \n",*percDutyL);
				usart_WriteMsg(&usart1Comm, bufferMsg);

				counter_L = 0;

			}
			else{
				updateDutyCycle(&PWM_L, *percDutyL);

				counter_L = 0;

			}
		}

		flagTimer ^= 1;

	} // Fin del while
}


/* Función para mover hacia adelante */

void forwardMove(float percDutyL, float percDutyR){

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

	updateDutyCycle(&PWM_R,percDutyR);
	updateDutyCycle(&PWM_L,percDutyL);

	// Encendemos el PWM para mover el motor derecho
	startPwmSignal(&PWM_R);
	startPwmSignal(&PWM_L);

}

/*
 * Función para mover el robot hacia atrás
 */
void backwardMove(float percDutyL, float percDutyR){

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

	updateDutyCycle(&PWM_R,percDutyR);
	updateDutyCycle(&PWM_L,percDutyL);

	// Encendemos el PWM para mover el motor derecho
	startPwmSignal(&PWM_R);
	startPwmSignal(&PWM_L);

}


/* Funcipon para apagar el sistema */
void turnOff(void){

	// Apaga el puente H para los motores
	gpio_WritePin(&GPIO_Enb_R, SET);
	gpio_WritePin(&GPIO_Enb_L, SET);

	// Apaga los PWM
	stopPwmSignal(&PWM_R);
	stopPwmSignal(&PWM_L);

}


/* Funcipon para encender el sistema */
void turnOn(void){

	// Enciente el puente H de los motores
	gpio_WritePin(&GPIO_Enb_R, RESET);
	gpio_WritePin(&GPIO_Enb_L, RESET);

	// Enciente los PWM
	startPwmSignal(&PWM_R);
	startPwmSignal(&PWM_L);

}


/* Función para el PID */
void PID(PWM_Handler_t *PWM_handler, uint16_t target, uint16_t measure){


	while(flagPID){

		// Se calcula la diferencia de tiempo
		//deltaTime = (currTime - prevTime) / 1E5; 	// Se calcula al diferencia de tiempo y se deja en segundos (unidades)
		//prevTime = currTime;					// Actualizamos la variable del tiempo
		deltaTime = deltaTime;

		// Se calcula el error de medida
		deltaError = target - measure;			// Diferencia entre el valor deseado y el medido en la actual iteración

		// Se calcula la parte Derivativa del error
		devError = (deltaError - prevError) / deltaTime;

		// Se calcula la parte Integral del error
		integralError = integralError + (deltaError*deltaTime);

		/* La señal de control de PID, que representa la variable que modifica el actuador del sistema,
		 * en este caso el PWM para los motores */
		u_PID = kp*deltaError + ki*integralError + kd*devError;

		// Realizamos el ajuste en el motor

		/*
		 * PASARLO A VALOR ASBOLUTO (u_PID)
		 */
		if(u_PID > 100){
			u_PID = 100;
		}

		// Actualizamos el dutyCycle según el ajuste producto del PID
		updateDutyCycle(PWM_handler, PWM_handler->config.percDuty + u_PID);

		// Bajamos la bandera para salir del while
		flagPID = 0;
	}

}


/* Función para manejar los diferentes conteos de tiempo (Periodos) */
void manageCounters(void){
	if (counterBlinky > LimitBlinky){//cada 500 ms revisamos los contadores

		gpio_TogglePin(&stateLed);//cambiamos el estado del led
		gpio_TogglePin(&stateLedBoard);

		if (gpio_ReadPin(&stateLed)) {//si el pin esta en alto contamos
			if (periodBlinky > 10) {//contamos cada ciclo de encendido-apagado de led
				periodBlinky = 0;//reiniciamos la variable
			}
			periodBlinky++;//aumentamos el contador del periodo
		}

		flagTimer ^= 1;
		counterBlinky = 0;
		counterPeriodTest++;

	}
}

/* Función para calcular la rotación en grados del robot (yaw) */
void yawIntegral(void){
	readGyro(&imuHandler, gyroData);
	yaw_gyro += (gyroData[2] * dt)* (180.0/M_PI);

    // Corrección de los límites de yaw_gyro
    if (yaw_gyro > 360.0) {
        yaw_gyro -= 360.0;
    } else if (yaw_gyro < 0.0) {
        yaw_gyro += 360.0;
    }

//	sprintf(bufferMsg,"gyro values  %.2f,%.2f,%.2f\n",gyroData[0],gyroData[1],gyroData[2]);
	sprintf(bufferMsg,"rate is  %.2f \t Yaw  %.2f\n",gyroData[2],yaw_gyro);
	usart_WriteMsg(&usart1Comm, bufferMsg);
}


/* Callback de Timer 2 */
void Timer2_Callback(void){
	counterBlinky++;
	counterIMU++;
	counterMicros++;
	// Función que maneja todos los conteos de tiempo basados en interrupciones del Timer2
	manageCounters();

}


/* Interrupciones por recepcion a traves de transmision serial */
void usart1_RxCallback(void){
	rxData = usart1_getRxData();

}


/* Interrupciones para el encoder derecho */
void callback_ExtInt1(void){
	//flagEncR = 1;
	counter_R++;
}


/* Interrupciones para el encoder izquierdo */
void callback_ExtInt3(void){
	//flagEncL = 1;
	counter_L++;

}


