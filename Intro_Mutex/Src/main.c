/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Sebastian Gaviria
 * @brief          : Semaphore-Mutex
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
#include "main.h"

#define STACK_SIZE 200

// definicion de variables del sistema
uint32_t SystemCoreClock = 100E6;
PLL_Config_t pllHandler = {0};

//USART
GPIO_Handler_t led_State_Handle		= {0};
GPIO_Handler_t user_button_Handle	= {0};
EXTI_Config_t exti_button_Handle	= {0};

USART_Handler_t commTerm_Handler	= {0};
GPIO_Handler_t pinTX_Handle 		= {0};
GPIO_Handler_t pinRX_Handle 		= {0};

uint8_t usartData = 0;

/* Handlers del FreeRTOS */
BaseType_t xReturned;

/* Handlers de las tareas */
TaskHandle_t xHandleTask_Print1 = NULL;
TaskHandle_t xHandleTask_Print2 = NULL;
TaskHandle_t xHandleTask_Print3 = NULL;

/* Headers del FreeRTOS */
extern void vTask_Print(void* pvParameters);

/* Handler de una variable tipo Semaphore. El Mutex es un tipo de semáforo que es
 * usado para asegurar el acceso mutuo exclusivo al UART */
xSemaphoreHandle xMutex;

/* Timer para el led de estado */
TimerHandle_t handle_led_timer;

const char *msg_working = "\n -- Working -- \n";
char auxMsg[64] = {0};

volatile uint8_t printOneTime = 0;

volatile uint16_t randomNumber = 0;

/*  Cabeceras de las funciones del programa */
void initSystem(void);
uint16_t getRandomNumber(void);
void led_state_callback(TimerHandle_t xTimer);
void prv_PrintString(const portCHAR *pcString);


/*
 * FUNCIÓN PRINCIPAL DEL PROGRAMA
 */
int main(void)
{

	/* Activamos la unidad de punto flotante (FPU) */
	SCB->CPACR    |= (0xF << 20);

	/* Activamos del contador de Ticks */
	DWT->CTRL    |= (1 << 0);

	/* Inicializamos el sistema */
	initSystem();


    /* Creando la Tarea Print1 */
    xReturned = xTaskCreate(
    				vTask_Print,       			/* Function that implements the task. */
                    "Task-Print_1",        			/* Text name for the task. */
                    STACK_SIZE,      				/* Stack size in words, not bytes. */
                    "---- Task-Print_1 ---- \n",    /* Parameter passed into the task. */
                    2,								/* Priority at which the task is created. */
                    &xHandleTask_Print1);			/* Used to pass out the created task's handle. */

    configASSERT(xReturned == pdPASS);

    /* Creando la Tarea Print2 */
    xReturned = xTaskCreate(
    				vTask_Print,       			/* Function that implements the task. */
                    "Task-Print_2",        			/* Text name for the task. */
                    STACK_SIZE,      				/* Stack size in words, not bytes. */
                    "---- Task-Print_2 ---- \n",    /* Parameter passed into the task. */
                    2,								/* Priority at which the task is created. */
                    &xHandleTask_Print2);			/* Used to pass out the created task's handle. */

    configASSERT(xReturned == pdPASS);

    /* Creando la Tarea Print3 */
    xReturned = xTaskCreate(
    				vTask_Print,       			/* Function that implements the task. */
                    "Task-Print_3",        			/* Text name for the task. */
                    STACK_SIZE,      				/* Stack size in words, not bytes. */
                    "---- Task-Print_3 ---- \n",    /* Parameter passed into the task. */
                    2,								/* Priority at which the task is created. */
                    &xHandleTask_Print3);			/* Used to pass out the created task's handle. */

    configASSERT(xReturned == pdPASS);


    /* Creando el semáforo */
    xMutex = xSemaphoreCreateMutex();

    configASSERT(xMutex != NULL); 	// Verificamos que se ha creado el semáforo


    /* Creando el timer */
    handle_led_timer = xTimerCreate("led_timer",
    								pdMS_TO_TICKS(500),
									pdTRUE,
									(void* ) 1,
									*led_state_callback);

    xTimerStart(handle_led_timer, portMAX_DELAY);

    //Start the created tasks running
    vTaskStartScheduler();


    /* Loop forever */
	while(1){
		// Si llegamos aqui algo salio mal
	}
}


/* Funcion para inicializar el sistema*/
void initSystem(void){


	systemClock_100MHz(&pllHandler);
	RCC->CR &= ~(RCC_CR_HSITRIM); // Limpiamos el registro
	RCC->CR |= (11 << RCC_CR_HSITRIM_Pos); // Numero para calibrar POR DEFECTO ESTABA EN 15!!!!!

	/* Configuramos el led de estado */
	led_State_Handle.pGPIOx 						= GPIOA;
	led_State_Handle.pinConfig.GPIO_PinNumber		= PIN_5;
	led_State_Handle.pinConfig.GPIO_PinMode 		= GPIO_MODE_OUT;
	led_State_Handle.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_LOW;
	led_State_Handle.pinConfig.GPIO_PinOutputType 	= GPIO_OTYPE_PUSHPULL;
	led_State_Handle.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

	// Cargamos la configuracion del pin led_State_Handle
	gpio_Config(&led_State_Handle);

	// Apagamos el led
	gpio_WritePin(&led_State_Handle, RESET);

	/* Configuramos el User Button */
	user_button_Handle.pGPIOx							= GPIOC;
	user_button_Handle.pinConfig.GPIO_PinNumber			= PIN_13;
	user_button_Handle.pinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	user_button_Handle.pinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	gpio_Config(&user_button_Handle);

	/* ==================================== Configuramos el EXTI ================================================*/
	/* Configuramos el EXTI del User Button */
	exti_button_Handle.pGPIOHandler				= &user_button_Handle;
	exti_button_Handle.edgeType					= EXTERNAL_INTERRUPT_RISING_EDGE;
	exti_button_Handle.priority					= e_EXTI_PRIORITY_6;

	exti_Config_Int_Priority(&exti_button_Handle, e_EXTI_PRIORITY_6);
	exti_Config(&exti_button_Handle);

	/* ==================================== Configurando los USART =============================================*/
	pinTX_Handle.pGPIOx										= GPIOA;
	pinTX_Handle.pinConfig.GPIO_PinNumber					= PIN_2;
	pinTX_Handle.pinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	pinTX_Handle.pinConfig.GPIO_PinOutputSpeed				= GPIO_OSPEED_HIGH;
	pinTX_Handle.pinConfig.GPIO_PinAltFunMode				= AF7;
	gpio_Config(&pinTX_Handle);

	pinRX_Handle.pGPIOx										= GPIOA;
	pinRX_Handle.pinConfig.GPIO_PinNumber					= PIN_3;
	pinRX_Handle.pinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	pinRX_Handle.pinConfig.GPIO_PinAltFunMode				= AF7;
	gpio_Config(&pinRX_Handle);

	commTerm_Handler.ptrUSARTx									= USART2;
	commTerm_Handler.USART_Config.baudrate						= USART_BAUDRATE_19200_100MHz;
	commTerm_Handler.USART_Config.datasize						= USART_DATASIZE_8BIT;
	commTerm_Handler.USART_Config.parity						= USART_PARITY_NONE;
	commTerm_Handler.USART_Config.stopbits						= USART_STOPBIT_1;
	commTerm_Handler.USART_Config.mode							= USART_MODE_RXTX;
	commTerm_Handler.USART_Config.enableIntRX					= USART_RX_INTERRUPT_ENABLE;
	commTerm_Handler.USART_Config.enableIntTX					= USART_TX_INTERRUPT_DISABLE;
	usart_Config(&commTerm_Handler);
	usart_Config_Int_Priority(&commTerm_Handler, e_USART_PRIORITY_6);
}

/* Función que gobierna la tarea Print */
void vTask_Print(void* pvParameters){

	char *pcStringToPrint;

	/* Two instances of this task are created so the string the task
	 * will send to prvNewPrintString() is passed in the task parameter.
	 * Cast this to the required type.	 *
	 */
	pcStringToPrint = (char*) pvParameters;

	while(1){

		/* Print out the string using the newly definied function */
		prv_PrintString(pcStringToPrint);

		/* Wait a pseudo random time. Note that rand() is not necessarily
		 * re-entrant, but in this case it doesn't really matter as code
		 * doesn't care what value is returned. In a more secure application
		 * a version of rand() that is known to be re-entrant should be used
		 * or calls to rand() should be protected using a critical section
		 */
		vTaskDelay(getRandomNumber() & 0xFF);

	}//end loop

}

/* Función que imprime el String */
void prv_PrintString(const portCHAR *pcString){

	char cBuffer[60];

	/* The semaphore is created before the scheduler is started, so
	 * already exists by the time this task executesprv_PrintString
	 *
	 * Attempt to take the semaphore, blocking indefinitely if the mutex
	 * isn´t available immediately. The call to xSemaphoreTake() will only
	 * return when the semaphore has been successfully obtained, so there
	 * is no need to check the return value. If any other delay period was
	 * used, then the code must check the xSemaphoreTakke() returns pdTRUE
	 * before accessing the source (in this case standard out).
	 */

	xSemaphoreTake(xMutex, portMAX_DELAY);
	{

		/* The following line will only execute once the semaphore has been
		 * successfully obtained - so standard out can be accessed freely.
		 */
		sprintf(cBuffer, "%s", pcString);
		usart_WriteMsg(&commTerm_Handler, (char*) cBuffer);
	}
	xSemaphoreGive(xMutex);
}// Fin función para imprimir


/* Función para generar un número aleatorio entre 1 y 10 */
uint16_t getRandomNumber(void){

	int randomnumber;

	/* Generamos una semilla de forma que rand() genere siempre un número diferente.
	 * La función srand() requiere siempre un número diferente, por eso le pasamos
	 * el conteo de ticks del sistema */
	srand(xTaskGetTickCount());
	randomnumber = (rand()%10) + 1;

	/* Devolvemos el número pseudo aleatorio */
	return (uint16_t) randomnumber;
}


/* Callback del Timer del led_State_Handle */
void led_state_callback(TimerHandle_t xTimer){
	gpio_TooglePin(&led_State_Handle);
}


/*Interrupción debida al puerto serial */
void usart2_RxCallback(void){
	usartData = usart2_getRxData();

} // Fin Callback USART2


/* Callback del EXTI13 */
void callback_ExtInt13(void){

	printOneTime = 1;
	randomNumber = getRandomNumber();

	BaseType_t pxHigherPriorityTaskWoken;
	pxHigherPriorityTaskWoken = pdFALSE;
	(void)pxHigherPriorityTaskWoken;

}
