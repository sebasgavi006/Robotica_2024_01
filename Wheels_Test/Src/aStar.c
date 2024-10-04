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
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "stm32f4xx.h"
#include "gpio_driver_hal.h"
#include "timer_driver_hal.h"
#include "usart_driver_hal.h"
#include "systick_driver_hal.h"
#include "pll_driver_hal.h"



#define MAX_ROWS 60
#define MAX_COLS 60
#define MAX_NEIGHBORS 4


typedef struct {
    int row;
    int col;
    int xParent;
    int yParent;
    bool bObstacle;
    bool bPath;
    int fCost;
    int hCost;
} sNode;

//------------------------------------------VARIABLES GLOBALES------------------------------------------
sNode neighborsList[MAX_NEIGHBORS];
sNode startN = {-1,-1};
sNode goalN = {-1,-1};

int bestIndex = 0;



int openLsize = 0;
int fCosts[MAX_ROWS] = {0};
sNode openL[MAX_ROWS] = {0};
bool closedL[MAX_ROWS][MAX_COLS] = {false};//Lista de nodos cerrados
sNode pathMatrix[MAX_ROWS] = {0}; // Matriz para almacenar el camino
int auxPath = 0;

int sizeRows = 10;
int sizeCols = 10;
int prob = 3;

int currentRow = 0;
int enterMatrixFlag = 0;

sNode nodesMatrix[MAX_ROWS][MAX_COLS];

int countNeighbors = 0;



int matrix[MAX_ROWS][MAX_COLS] = {0};
//------------------------------------------------------------------------------------



// Se define un pin para el Blinky
GPIO_Handler_t stateLed = {0}; // PinA5
GPIO_Handler_t stateLedBoard = {0}; // PinC5
// Handler del Timer
Timer_Handler_t Tim_Blinky = {0};
// Handler para el PLL
PLL_Handler_t pllHandler = {0};
//USART
GPIO_Handler_t handlerPinTX		= {0};
GPIO_Handler_t handlerPinRX		= {0};
USART_Handler_t usart2Comm		= {0};


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






// Contadores
uint16_t counterBlinky = 0;
uint32_t counterMicros = 0;
uint16_t counterPeriodTest = 0;
uint16_t LimitBlinky = 50E3;

uint8_t flagTimer = 0;

uint8_t periodBlinky = 0;



char bufferData[64] = "Algoritmo...";

// Funciones privadas
void initSystem(void);
void parseCommands(char  *ptrbufferReception);
void manageCounters(void);
void clearScreen(void);

void buildMatrixUSART(char  *ptrbufferReception);




/* ===== Función principal del programa ===== */
int main(void){

	SCB->CPACR |= 0xf<<20;

	initSystem();



	// Se configura inicialmente el MPU
	config_SysTick_ms(HSI_CLOCK_CONFIGURED);
	delay_ms(1);

	sprintf(bufferMsg, "Algoritmo A* \n");
	usart_WriteMsg(&usart2Comm, bufferMsg);
	clearScreen();
	sprintf(bufferMsg, "Escribe \"help @\" ");
	usart_WriteMsg(&usart2Comm, bufferMsg);


	/* Loop forever */
	while (1) {





		// Se revisa cual fue el dato recibido por la comunicacion serial
		if(rxData != '\0'){
			bufferReceiver[counterReception] = rxData;
			counterReception++;

			// Se verifica si el último dato es el caracter de finalizacion de un string
			if (rxData == '@'){
				// Se modifica el estado de una variable de control
				stringComplete = 1;

				// Configuramos las variables para guardar el string y esperar uno nuevo
				bufferReceiver[counterReception] = '\0';//el caracter nulo \0 siempre esta al final de un string
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
			usart_WriteMsg(&usart2Comm, bufferReceiver);

			if (!enterMatrixFlag) {
				parseCommands(bufferReceiver);
				stringComplete = 0;
			}

			else if (enterMatrixFlag) {
				buildMatrixUSART(bufferReceiver);
				stringComplete = 0;
			}

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
	/*USART 1 -> Comunicación serial a través de la antena */
	handlerPinTX.pGPIOx										= GPIOA;
	handlerPinTX.pinConfig.GPIO_PinNumber					= PIN_2;
	handlerPinTX.pinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	handlerPinTX.pinConfig.GPIO_PinAltFunMode				= AF7;
	gpio_Config(&handlerPinTX);

	handlerPinRX.pGPIOx										= GPIOA;
	handlerPinRX.pinConfig.GPIO_PinNumber					= PIN_3;
	handlerPinRX.pinConfig.GPIO_PinMode						= GPIO_MODE_ALTFN;
	handlerPinRX.pinConfig.GPIO_PinAltFunMode				= AF7;
	gpio_Config(&handlerPinRX);

	usart2Comm.ptrUSARTx									= USART2;
	usart2Comm.USART_Config.baudrate						= USART_BAUDRATE_9600;
	usart2Comm.USART_Config.datasize						= USART_DATASIZE_8BIT;
	usart2Comm.USART_Config.parity							= USART_PARITY_NONE;
	usart2Comm.USART_Config.stopbits						= USART_STOPBIT_1;
	usart2Comm.USART_Config.mode							= USART_MODE_RXTX;
	usart2Comm.USART_Config.enableIntRX						= USART_RX_INTERRUPT_ENABLE;
	usart2Comm.USART_Config.enableIntTX						= USART_TX_INTERRUPT_DISABLE;
	usart_Config(&usart2Comm);
} // Fin initSystem()





//------------------------------------------------------ALGORITMO PRINCIPAL------------------------------------------------------





void clearScreen(void){
	for (int i = 0; i < 20; i++) {
		sprintf(bufferMsg, "\n");
		usart_WriteMsg(&usart2Comm, bufferMsg);
	}
}



void buildMatrixUSART(char *ptrbufferReception) {

	int auxIndex = strlen(ptrbufferReception);


    int tokenCount = 0;
    char *token = {0};
    char buffer[64] = {0}; // Buffer temporal para almacenar la fila sin corchetes
    strncpy(buffer, ptrbufferReception + 1, auxIndex - 3); // Copiar la fila sin '[' y ']'
    buffer[auxIndex - 3] = '\0'; // Agregar terminador de cadena
//	sprintf(bufferMsg, "\nNum de indices en el string %u",auxIndex);
//	usart_WriteMsg(&usart2Comm, bufferMsg);
//	sprintf(bufferMsg, "\nCantidad en el string %c \n",ptrbufferReception[auxIndex -3]);
//	usart_WriteMsg(&usart2Comm, bufferMsg);


    // Verificar si el mensaje comienza con '[' y termina con ']'
    if (ptrbufferReception[1] == '[' && ptrbufferReception[auxIndex - 3] == ']') {



        // Verifica si se han ingresado todas las filas

        // Usar strtok para separar los tokens
        token = strtok(buffer, ",");
        while (token != NULL) {
            tokenCount++;
            token = strtok(NULL, ",");
        }

        //verificamos que tenga el tamaño adecuado
        if (tokenCount == sizeCols) {
			//aumentamos el contador de filas
			sprintf(bufferMsg, "\nFila %d ingresada.\n", currentRow);
			usart_WriteMsg(&usart2Comm, bufferMsg);
			currentRow++;//aumentamos la fila

			if (currentRow >= sizeRows) {
				sprintf(bufferMsg, "\nSe ha completado la matriz con %d filas.\n", currentRow);
				usart_WriteMsg(&usart2Comm, bufferMsg);
				enterMatrixFlag = 0;  // Desactivar la bandera
			}
		}
        else {
            sprintf(bufferMsg, "\nNúmero de columnas no coincide. Se esperaban %d columnas pero se recibieron %d.\n", sizeCols, tokenCount);
            usart_WriteMsg(&usart2Comm, bufferMsg);

		}





    }//fin del if qu revisa la fila



    else {
        usart_WriteMsg(&usart2Comm, "\nFormato inválido. Usa el formato [x,x,x,...].\n");
    }
}












/* Función para los comandos */
void parseCommands(char  *ptrbufferReception){

	sscanf(ptrbufferReception,"%s %f %f %s",cmd,&firstParameter,&secondParameter,lastString);
	//Comando para solicitar ayuda
    // Comando para solicitar ayuda
    if(strcmp(cmd, "help") == 0){
        usart_WriteMsg(&usart2Comm, "\nHelp Menu CMDS: \n");
        usart_WriteMsg(&usart2Comm, "1) Tamaño matriz: \"m # @\" para filas, \"n # @\" para columnas\n");
    }

    // Comando para definir el tamaño de filas (m)
    else if(strcmp(cmd, "m") == 0) {
        if(firstParameter > 0 && firstParameter <= MAX_ROWS) {
            sizeRows = (int)firstParameter;  // Ajustamos el tamaño de las filas
            sprintf(bufferMsg, "El tamaño de filas m ahora es: %d\n", sizeRows);
            usart_WriteMsg(&usart2Comm, bufferMsg);
        } else {
            usart_WriteMsg(&usart2Comm, "Valor invalido para m. Debe ser un número positivo menor o igual a 100.\n");
        }
    }

    // Comando para definir el tamaño de columnas (n)
    else if(strcmp(cmd, "n") == 0) {
        if(firstParameter > 0 && firstParameter <= MAX_COLS) {
            sizeCols = (int)firstParameter;  // Ajustamos el tamaño de las columnas
            sprintf(bufferMsg, "El tamaño de columnas n ahora es: %d\n", sizeCols);
            usart_WriteMsg(&usart2Comm, bufferMsg);
        } else {
            usart_WriteMsg(&usart2Comm, "Valor invalido para n. Debe ser un número positivo menor o igual a 100.\n");
        }
    }


    // Comando para empezar a ingresar la matriz
    else if(strcmp(cmd, "matriz") == 0) {
        if (sizeRows > 0 && sizeCols > 0) {
            enterMatrixFlag = 1;  // Activamos la secuencia para ingresar la matriz
            currentRow = 0;  // Iniciamos en la primera fila
            sprintf(bufferMsg, "\nEscribe %d filas ingresando cada fila de la matriz:\n", sizeRows);
            usart_WriteMsg(&usart2Comm, bufferMsg);
        } else {
            usart_WriteMsg(&usart2Comm, "\nPrimero define el tamaño de la matriz con los comandos 'm' y 'n'.\n");
        }
    }


/* ==================== PID ==================== */




	// Opción para resetear en cualquier momento
	else if (strcmp(cmd, "reset") == 0) {
		usart_WriteMsg(&usart2Comm, "PWR_MGMT_1 reset \n");

	}

	else{
		usart_WriteMsg(&usart2Comm, "\nComando erroneo.\n Ingresa \"help @\" para ver la lista de comandos.\n");
	}


}

//----------------------------------------------------------------

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



/* Callback de Timer 2 */
void Timer2_Callback(void){
	counterBlinky++;
	counterMicros++;
	// Función que maneja todos los conteos de tiempo basados en interrupciones del Timer2
	manageCounters();
}


/* Interrupciones por recepcion a traves de transmision serial */
void usart2_RxCallback(void){
	rxData = usart2_getRxData();

}



