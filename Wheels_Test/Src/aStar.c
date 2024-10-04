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
char buffer[64] = {0}; // Buffer temporal para almacenar la fila sin corchetes
char buffer2[64] = {0}; // Buffer temporal para almacenar la fila sin corchetes
int sizeRows = 5;
int sizeCols = 5;
int prob = 3;

int currentRow = 0;

int enterMatrixFlag = 0;
int builtMatrixFlag = 0;


sNode nodesMatrix[MAX_ROWS][MAX_COLS];

int countNeighbors = 0;



int inputMatrix[MAX_ROWS][MAX_COLS] = {0};
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
void findPoints(void);
void print_grid(sNode mtrx[MAX_ROWS][MAX_COLS], sNode *start,sNode *goal);

int f_cost(sNode *point);
int g_cost(sNode *point);
int h_cost(sNode *point);
int manhattan_Dist(sNode *p1,sNode *p2);
void updateNeighbors(sNode *current, sNode matrx[MAX_ROWS][MAX_COLS], sNode neighbors[MAX_NEIGHBORS], int *count);
void print_neighbors(sNode neighbors[MAX_NEIGHBORS], int count);
void rmFromOpenL(sNode* nodeToRM);
void searchBestF(void);
void aStar(sNode matrx[MAX_ROWS][MAX_COLS]);




/* ===== Función principal del programa ===== */
int main(void){

	SCB->CPACR |= 0xf<<20;

	initSystem();




	// Se configura inicialmente el MPU
	config_SysTick_ms(HSI_CLOCK_CONFIGURED);
	delay_ms(1);

	sprintf(bufferMsg, "Algoritmo A* \n");
	sprintf(bufferMsg, "Elaborado por Jhony Aristizábal \n");
	usart_WriteMsg(&usart2Comm, bufferMsg);
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
	usart2Comm.USART_Config.baudrate						= USART_BAUDRATE_115200;
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
    char *token = NULL;

    strncpy(buffer, ptrbufferReception + 1, auxIndex - 3); // Copiar la fila sin '[' y ']'
    buffer[auxIndex - 3] = '\0'; // Agregar terminador de cadena
    strncpy(buffer2, buffer+1,auxIndex -2); // Copiar la fila sin '[' y ']'

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
        // Verificamos que tenga el tamaño adecuado
        if (tokenCount == sizeCols) {
            // Reiniciar strtok para recorrer nuevamente los tokens

			token = strtok(buffer2, ",");
        	for (int j = 0; j < sizeCols; j++) {
        	    // Usamos atoi en lugar de castear el buffer directamente
        	    sprintf(bufferMsg, "\n valor %d ingresado.\n", atoi(token));
        	    usart_WriteMsg(&usart2Comm, bufferMsg);

        	    // Almacenar el valor entero en nodesMatrix
        	    inputMatrix[currentRow][j] = atoi(token);

        	    // Obtener el siguiente token
        	    tokenCount++;
        	    token = strtok(NULL, ",");
        	}

            // Aumentamos el contador de filas
            sprintf(bufferMsg, "\nFila %d ingresada.\n", currentRow);
            usart_WriteMsg(&usart2Comm, bufferMsg);
            currentRow++; // Aumentamos la fila

            // Verifica si se han ingresado todas las filas
            if (currentRow >= sizeRows) {
                sprintf(bufferMsg, "\nSe ha completado la matriz con %d filas.\n", currentRow);
                usart_WriteMsg(&usart2Comm, bufferMsg);
                enterMatrixFlag = 0;  // Desactivar la bandera
                builtMatrixFlag = 1;	//la matriz está construida
                findPoints();
                print_grid(nodesMatrix, &startN,&goalN);
            }
        } else {
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
	// Comando para solicitar ayuda
	if (strcmp(cmd, "help") == 0) {
	    usart_WriteMsg(&usart2Comm, "\nHelp Menu CMDS: \n");
	    usart_WriteMsg(&usart2Comm, "1) Tamaño matriz: \"m # @\" para definir el número de filas.\n");
	    usart_WriteMsg(&usart2Comm, "2) Tamaño matriz: \"n # @\" para definir el número de columnas.\n");
	    usart_WriteMsg(&usart2Comm, "3) Ingreso de matriz: \"b\" para comenzar a ingresar la matriz.\n");
	    usart_WriteMsg(&usart2Comm, "   - La matriz debe ser ingresada en el formato: \"[0,0,0,0,1,3,5]@\"\n");
	    usart_WriteMsg(&usart2Comm, "   - Donde:\n");
	    usart_WriteMsg(&usart2Comm, "     * 1 representa un obstáculo.\n");
	    usart_WriteMsg(&usart2Comm, "     * 0 representa una casilla vacía.\n");
	    usart_WriteMsg(&usart2Comm, "     * 2 representa el inicio.\n");
	    usart_WriteMsg(&usart2Comm, "     * 3 representa el fin.\n");
	    usart_WriteMsg(&usart2Comm, "4) Imprimir matriz: \"printMatrix\" para mostrar la matriz construida.\n");
	    usart_WriteMsg(&usart2Comm, "   - Asegúrate de que la matriz haya sido construida antes de usar este comando.\n");
	    usart_WriteMsg(&usart2Comm, "5) Ejecutar A*: \"aStar\" para iniciar el algoritmo A* con los puntos de inicio y fin encontrados.\n");
	    usart_WriteMsg(&usart2Comm, "   - Busca los puntos de inicio (2) y fin (3) en la matriz, y visualiza el camino encontrado.\n");
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
    else if(strcmp(cmd, "build") == 0) {
        if (sizeRows > 0 && sizeCols > 0) {
            enterMatrixFlag = 1;  // Activamos la secuencia para ingresar la matriz
            usart_WriteMsg(&usart2Comm, "\nVamos a construir la matriz");
            sprintf(bufferMsg, "\nEscribe %d filas ingresando cada fila de la matriz:\n", sizeRows);
            usart_WriteMsg(&usart2Comm, "\nPor ejemplo:  \"[0,0,0,0,1,3,5]@ \" ");
            usart_WriteMsg(&usart2Comm, "\n 1 representa obstaculo  ");
            usart_WriteMsg(&usart2Comm, "\n 0 representa casilla vacia  ");
            usart_WriteMsg(&usart2Comm, "\n 2 representa inicio  ");
            usart_WriteMsg(&usart2Comm, "\n 3 representa fin  ");
            usart_WriteMsg(&usart2Comm, bufferMsg);
        } else {
            usart_WriteMsg(&usart2Comm, "\nPrimero define el tamaño de la matriz con los comandos 'm' y 'n'.\n");
        }
    }

    // Comando para imprimir la matriz
    else if (strcmp(cmd, "printMatrix") == 0 ) {
        if (builtMatrixFlag) {  // Verificamos si la matriz ha sido construida
            sprintf(bufferMsg, "\nMatriz construida:\n");
            usart_WriteMsg(&usart2Comm, bufferMsg);
            for (int i = 0; i < sizeRows; i++) {
                for (int j = 0; j < sizeCols; j++) {
                    sprintf(bufferMsg, "%d ", inputMatrix[i][j]);  // Imprimimos cada elemento de la matriz
                    usart_WriteMsg(&usart2Comm, bufferMsg);
                }
                usart_WriteMsg(&usart2Comm, "\n");  // Nueva línea al final de cada fila
            }
        } else {
            usart_WriteMsg(&usart2Comm, "La matriz no ha sido construida aún.\n");
        }
    }


    // Comando para ejecutar el algoritmo A*
    else if(strcmp(cmd, "aStar") == 0) {



    	aStar(nodesMatrix);

        for(int i = 0; i < auxPath; i++){
//        	clearScreen();
            nodesMatrix[pathMatrix[i].row][pathMatrix[i].col].bPath = true;
            print_grid(nodesMatrix,&startN,&goalN);
            delay_ms(1500);
        }
        print_grid(nodesMatrix,&startN,&goalN);

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

void findPoints(void) {
    bool startFound = false;
    bool goalFound = false;

    for (int i = 0; i < sizeRows; i++) {
        for (int j = 0; j < sizeCols; j++) {

            // Asignar coordenadas (i,j)
            nodesMatrix[i][j].row = i;
            nodesMatrix[i][j].col = j;
            nodesMatrix[i][j].xParent = 0;
            nodesMatrix[i][j].yParent = 0;
            nodesMatrix[i][j].bPath = false;
            nodesMatrix[i][j].fCost = 0;
            nodesMatrix[i][j].hCost = 0;

            if (inputMatrix[i][j] == 2 && !startFound) {
                startN.row = i;
                startN.col = j;
                startN.bObstacle = false; // No es un obstáculo
                startFound = true; // Marcamos que hemos encontrado el inicio
            } else if (inputMatrix[i][j] == 3 && !goalFound) {
                goalN.row = i;
                goalN.col = j;
                goalN.bObstacle = false; // No es un obstáculo
                goalFound = true; // Marcamos que hemos encontrado el objetivo
            } else if (inputMatrix[i][j] == 1) {
                nodesMatrix[i][j].bObstacle = true; // Es un obstáculo
            } else if (inputMatrix[i][j] == 0) {
                nodesMatrix[i][j].bObstacle = false; // No es un obstáculo
            }

            // Si ambos puntos han sido encontrados, no es necesario seguir buscando
            if (startFound && goalFound) {
                break;
            }
        }
        if (startFound && goalFound) {
            break;
        }
    }

    // Mensajes de confirmación
    if (startFound) {
        sprintf(bufferMsg, "Inicio encontrado en: (%d, %d)\n", startN.row, startN.col);
        usart_WriteMsg(&usart2Comm, bufferMsg);
    } else {
        usart_WriteMsg(&usart2Comm, "No se encontró el punto de inicio.\n");
    }

    if (goalFound) {
        sprintf(bufferMsg, "Objetivo encontrado en: (%d, %d)\n", goalN.row, goalN.col);
        usart_WriteMsg(&usart2Comm, bufferMsg);
    } else {
        usart_WriteMsg(&usart2Comm, "No se encontró el punto objetivo.\n");
    }
}




void print_grid(sNode mtrx[MAX_ROWS][MAX_COLS], sNode *start,sNode *goal){
    sprintf(bufferMsg,"Matrix %dx%d\t",sizeRows,sizeCols);
    usart_WriteMsg(&usart2Comm, bufferMsg);
    sprintf(bufferMsg,"Start= (%d,%d):\t",start->row,start->col);
    usart_WriteMsg(&usart2Comm, bufferMsg);
    sprintf(bufferMsg,"Goal= (%d,%d):\n",goal->row,goal->col);
    usart_WriteMsg(&usart2Comm, bufferMsg);

    for (int i = 0; i < sizeRows; i++){
        for (int j = 0; j < sizeCols; j++){

            if((mtrx[i][j].row == start->row) && (mtrx[i][j].col == start->col)){
                mtrx[i][j].bObstacle = false;

                usart_WriteMsg(&usart2Comm, "S ");
            }

            else if((mtrx[i][j].row == goal->row) && (mtrx[i][j].col == goal->col)){
                mtrx[i][j].bObstacle = false;

                usart_WriteMsg(&usart2Comm, "G ");

            }
            else if (mtrx[i][j].bPath == true){

                usart_WriteMsg(&usart2Comm, "@ ");
            }

            else if (mtrx[i][j].bObstacle == false){

                usart_WriteMsg(&usart2Comm, "\u00B7 ");
            }
            else if(mtrx[i][j].bObstacle == true){
                usart_WriteMsg(&usart2Comm, "X ");
            }

        }
        usart_WriteMsg(&usart2Comm,"\n" );
    }
}


void updateNeighbors(sNode *current, sNode matrx[MAX_ROWS][MAX_COLS], sNode neighbors[MAX_NEIGHBORS], int *count){
    int x = current->row;
    int y = current->col;

    *count = 0;

    sprintf(bufferMsg,"Nodo actual (%d,%d)-----------------------------------------------------\n",x,y);
    usart_WriteMsg(&usart2Comm, bufferMsg);

    if( x+1 < sizeRows && (matrx[x + 1][y].bObstacle == false)){//down
        neighbors[*count].row = x+1;
        neighbors[*count].col = y;
        (*count)++;
    }
    if( x > 0 && (matrx[x - 1][y].bObstacle == false)){//up
    neighbors[*count].row = x - 1;
    neighbors[*count].col = y;
    (*count)++;
    }

    if( y + 1 < sizeCols && (matrx[x][y + 1].bObstacle == false)){//right
    neighbors[*count].row = x;
    neighbors[*count].col = y + 1;
    (*count)++;
    }
    if( y > 0 && (matrx[x][y - 1].bObstacle == false)){//left
    neighbors[*count].row = x;
    neighbors[*count].col = y - 1;
    (*count)++;
    }
    // printf("Neighbors Count (%d)\n",*count);
}
// Función para imprimir la lista de vecinos
void print_neighbors(sNode neighbors[MAX_NEIGHBORS], int count) {
    printf("Neighbors (%d):\n", count);
    for (int i = 0; i < count; i++) {
        sprintf(bufferMsg,"(%d, %d)\n", neighbors[i].row, neighbors[i].col);
        usart_WriteMsg(&usart2Comm, bufferMsg);
        printf(bufferMsg,"gcost %d\n",g_cost(&neighbors[i]));
        usart_WriteMsg(&usart2Comm, bufferMsg);
        printf(bufferMsg,"hcost %d\n",h_cost(&neighbors[i]));
        usart_WriteMsg(&usart2Comm, bufferMsg);
        printf(bufferMsg,"fcost %d\n",f_cost(&neighbors[i]));
        usart_WriteMsg(&usart2Comm, bufferMsg);
    }
}

void aStar(sNode matrx[MAX_ROWS][MAX_COLS]){

	usart_WriteMsg(&usart2Comm, "Solving A* ");



    sNode currentN = startN; // Nodo actual

    openL[openLsize++] = startN; //agregamos el nodo inicial a la lista abierta

    int iteration = 0; // Variable de control para las iteraciones
    int maxIterations = 1000; // Establece el número máximo de iteraciones


    while (openLsize >0 && iteration < maxIterations){


    	sprintf(bufferMsg,"\niteracion de while: %d \t",iteration);
    	usart_WriteMsg(&usart2Comm, bufferMsg);

        updateNeighbors(&currentN, matrx, neighborsList, &countNeighbors);//revisemos los vecinos
        print_neighbors(neighborsList, countNeighbors);

        //agregamos el vecino a la lista abierta
        for (int i = 0; i < countNeighbors; i++){
            if (closedL[neighborsList[i].row][neighborsList[i].col] == false){
            	sprintf(bufferMsg,"agregando el vecino %d a la lista abierta: (%d,%d)\n",i,neighborsList[i].row,neighborsList[i].col);
            	usart_WriteMsg(&usart2Comm, bufferMsg);
                openL[openLsize++] = neighborsList[i];
            }

        }


        searchBestF();
        for (int i = 0; i < openLsize; i++){
        	sprintf(bufferMsg,"Recorriendo OpenL: en nodo (%d,%d)\n",openL[i].row,openL[i].col);
        	usart_WriteMsg(&usart2Comm, bufferMsg);
        }

        sprintf(bufferMsg,"nodo (%d,%d)removido\n",currentN.row,currentN.col);
        usart_WriteMsg(&usart2Comm, bufferMsg);
        sprintf(bufferMsg,"openLsize era : %d\n",openLsize);
        usart_WriteMsg(&usart2Comm, bufferMsg);

        rmFromOpenL(&currentN);

        sprintf(bufferMsg,"openLsize ahora es: %d\n",openLsize);
        usart_WriteMsg(&usart2Comm, bufferMsg);
        for (int i = 0; i < openLsize; i++){
        	sprintf(bufferMsg,"Recorriendo OpenL: en nodo (%d,%d)\n",openL[i].row,openL[i].col);
        	usart_WriteMsg(&usart2Comm, bufferMsg);
        }

        // Marcar el nodo actual en la matriz de camino
        sprintf(bufferMsg,"construimos el camino con a: (%d,%d)\n",currentN.row,currentN.col);
        usart_WriteMsg(&usart2Comm, bufferMsg);
        // matrx[currentN.row][currentN.col].bPath = true;
        pathMatrix[auxPath++] = currentN; // Marcar el nodo actual como parte del camino


        // recorremos pathmatrix e imprimimos
        for (int i = 0; i < auxPath; i++){
        	sprintf(bufferMsg,"PathMatrix (%d) en nodo (%d,%d)\n",i,pathMatrix[i].row,pathMatrix[i].col);
        	usart_WriteMsg(&usart2Comm, bufferMsg);
        }

        sprintf(bufferMsg,"aux path: %d\n",auxPath);
        usart_WriteMsg(&usart2Comm, bufferMsg);

        //nos desplazamos al nodo con costo mas bajo y asignamos la procedencia
        sNode *nextNode = &openL[bestIndex];
        nextNode->xParent = currentN.row;
        nextNode->yParent = currentN.col;
        currentN = *nextNode;
        sprintf(bufferMsg,"Nos desplazamos a: (%d,%d) desde (%d,%d)\n",currentN.row,currentN.col,currentN.xParent,currentN.yParent);
        usart_WriteMsg(&usart2Comm, bufferMsg);


        if (currentN.row == goalN.row && currentN.col == goalN.col) {
        	sprintf(bufferMsg,"Goal reached!\n");
        	usart_WriteMsg(&usart2Comm, bufferMsg);
        	sprintf(bufferMsg,"objetivo alcanzado en %d iteraciones\n",iteration);
        	usart_WriteMsg(&usart2Comm, bufferMsg);
            // Aquí deberías reconstruir y mostrar el camino
            return;
        }


        iteration++;//incrementar la variable de control
        }


        // Si el bucle termina por alcanzar maxIterations
        if (iteration >= maxIterations) {
        	sprintf(bufferMsg,"Max iterations reached: %d\n", maxIterations);
        	usart_WriteMsg(&usart2Comm, bufferMsg);
        }
}


void rmFromOpenL(sNode* nodeToRM){
    int indexToRm = -1;
    // printf("Vamos a eliminar (%d,%d) de la lista abierta\n",nodeToRM->row,nodeToRM->col);
    // printf("openLsize: %d\n",openLsize);
    closedL[nodeToRM->row][nodeToRM->col] = true;

    for (int i = 0; i < openLsize; i++){
        // printf("Recorriendo OpenL: en nodo (%d,%d)\n",openL[i].row,openL[i].col);
    }

    for (int i = 0; i < openLsize; i++){
        if (openL[i].row == nodeToRM->row && openL[i].col == nodeToRM->col){
            indexToRm = i;
            break;
        }
    }

    // printf("Nodo (%d,%d) Eliminado de la lista abierta\n",nodeToRM->row,nodeToRM->col);

    if (indexToRm != -1){
        if (indexToRm < openLsize -1){
            openL[indexToRm] = openL[--openLsize];
        }
        else{
            --openLsize;
        }
    }
    // printf("openLsize: %d\n",openLsize);

    for (int i = 0; i < openLsize; i++){
        // printf("Recorriendo OpenL: en nodo (%d,%d)\n",openL[i].row,openL[i].col);
    }

}

void searchBestF(void){

    bestIndex = 0;  //reiniciamos la variable para empezar la busqueda
    //recorremos OpenL
    for (int i = 1; i < openLsize; i++){
        //Comparamos los f cost para buscar el mejor
        if (f_cost(&openL[i]) < f_cost(&openL[bestIndex])){
            bestIndex = i;  //guardamos el valor del menor fcost
        }

        //si hay un empate en f usamos h para desempatar
        else if (f_cost(&openL[i]) == f_cost(&openL[bestIndex])){
            if (h_cost(&openL[i]) < h_cost(&openL[bestIndex])){
                bestIndex = i;
            }

        }

    }
    sprintf(bufferMsg,"El mejor fcost es %d con coordenada (%d,%d)\n",f_cost(&openL[bestIndex]),openL[bestIndex].row,openL[bestIndex].col);
    usart_WriteMsg(&usart2Comm, bufferMsg);
}





int manhattan_Dist(sNode *p1,sNode *p2){
    int x1 = p1->col;
    int y1 = p1->row;

    int x2 = p2->col;
    int y2 = p2->row;

    int dist = abs(x2 - x1) + abs(y2 - y1);

    return (dist);
}

int h_cost(sNode *point){
    // h(n): Es la funcion heuristica que estima el camino mas corto desde n hasta el FINAL

    return manhattan_Dist(point,&goalN);
}

int g_cost(sNode *point){
    // g(n): es el costo del camino desde el INICIO hasta n
    return manhattan_Dist(&startN,point);
}

int f_cost(sNode *point){
    return (g_cost(point) + h_cost(point));
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



