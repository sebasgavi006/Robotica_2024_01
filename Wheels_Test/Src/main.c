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
#include "usart_driver_hal.h"
#include "pwm_driver_hal.h"
#include "exti_driver_hal.h"
#include "systick_driver_hal.h"
#include "pll_driver_hal.h"

// Se define un pin para el Blinky
GPIO_Handler_t stateLed = {0}; // PinA5

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


// Funciones privadas
void initSystem(void);


/* ===== Funciión principal del programa ===== */
int main(void){

	// Configuramos el PLL para que el micro corra a 100MHz
	systemClock_100MHz(&pllHandler);
	systemClock_Output(); // el pin PC9 configurado como salida para leer la freq

	RCC->CR &= ~(RCC_CR_HSITRIM); // Limpiamos el registro
	RCC->CR |= (11 << RCC_CR_HSITRIM_Pos); // Numero para calibrar POR DEFECTO ESTABA EN 15!!!!!

	initSystem();

	/* Activar motor derecho */

	// Encendemos el puente H
	gpio_WritePin(&GPIO_Enb_R, RESET);	// Prende motor derecho
	gpio_WritePin(&GPIO_Enb_L, RESET);	// Apaga motor izquierdo

	// Seteamos la dirección del motor
	gpio_WritePin(&GPIO_Dir_R, SET);
	gpio_WritePin(&GPIO_Dir_L, RESET);


	PWM_R.config.polarity	= PWM_POLARITY_DIRECT;
	pwm_Config(&PWM_R);
	selectPolarity(&PWM_R);


	PWM_L.config.polarity	= PWM_POLARITY_INVERSE;
	pwm_Config(&PWM_L);
	selectPolarity(&PWM_L);

	stopPwmSignal(&PWM_R);
	stopPwmSignal(&PWM_L);


	// Encendemos el PWM para mover el motor derecho
	startPwmSignal(&PWM_R);
	startPwmSignal(&PWM_L);
	systick_Delay_ms(SYSTICK_10s);
	updateDutyCycle(&PWM_R, 90);


	// Ponemos un delay de 10s
	//systick_Delay_ms(SYSTICK_10s);

	// Detenemos el PWM
	//stopPwmSignal(&PWM_R);


	/* Loop forever */
	while(1){

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
	gpio_WritePin(&stateLed, SET);

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
	Tim_Blinky.TIMx_Config.TIMx_Prescaler			= 100E3;	// Genera incrementos de 1 ms. El micro está a 100MHz
	Tim_Blinky.TIMx_Config.TIMx_Period				= 2000;		// De la mano con el pre-scaler, determina cuando se dispara una interrupción (500 ms)
	Tim_Blinky.TIMx_Config.TIMx_mode				= TIMER_UP_COUNTER;	// El Timer cuante ascendente
	Tim_Blinky.TIMx_Config.TIMx_InterruptEnable		= TIMER_INT_ENABLE;	// Se activa la interrupción
	timer_Config(&Tim_Blinky);
	timer_SetState(&Tim_Blinky, TIMER_ON);


	// 3. ===== PWM =====

	/* Configurando el PWM para el motor DERECHO */
	PWM_R.ptrTIMx					= TIM5; // Timer5 usado para el PWM
	PWM_R.config.channel			= PWM_CHANNEL_1;
	PWM_R.config.prescaler			= 100E3; 	// 1 ms
	PWM_R.config.periodo			= 500;		// 500 ms
	PWM_R.config.percDuty			= 10;		// Activo por 5 ms
	pwm_Config(&PWM_R);

	/* Configurando el PWM para el motor IZQUIERDO */
	PWM_L.ptrTIMx					= TIM5; // Timer3 usado para el PWM
	PWM_L.config.channel			= PWM_CHANNEL_2;
	PWM_L.config.prescaler			= 100E3; 	// 1 ms
	PWM_L.config.periodo			= 500;		// 500 ms
	PWM_L.config.percDuty			= 10;		// Activo por 5 ms
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
	SystickHandler.Systick_Config.Systick_Reload	= 100000;
	SystickHandler.Systick_Config.Systick_IntState 	= SYSTICK_ON;
	systick_Config(&SystickHandler);


}



/* Función callback que atiende la interrupción del EXTI8, en este caso, corresponde
 * a la interrupcón externa del encoder
 * */
void callback_ExtInt1(void){
	counter_R ++;
}

/* Callback de Timer 2 para el Blinky */
void Timer2_Callback(void){
	gpio_TooglePin(&stateLed);
	counter_L++;
}


// Función para
