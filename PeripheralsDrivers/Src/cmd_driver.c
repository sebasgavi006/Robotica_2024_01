/*
 * cmd_driver.c
 *
 *  Created on: Mar 22, 2024
 *      Author: sebas
 */

#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "cmd_driver.h"
#include "usart_driver_hal.h"


/*
 * Esta funcion organiza los datos recibido a traves del puerto serial en un string
 * y devuelve una variable de estado cuando se completa el string
 */
uint8_t bufferReception(uint8_t RxData, char *ptrBufferReception, uint8_t counterReception){

	uint8_t stringComplete = 0;

	// Se revisa cual fue el dato recibido por la comunicacion serial
	if(RxData != '\0'){
		ptrBufferReception[counterReception] = RxData;
		counterReception++;

		// Se verifica si el último dato es el caracter de finalizacion de un string
		if (RxData == '@'){
			// Se modifica el estado de una variable de control
			stringComplete = 1;

			// Configuramos las variables para guardar el string y esperar uno nuevo
			ptrBufferReception[counterReception] = '\0';
			counterReception = 0;
		}

		// Limpiamos la variable que almacena los datos recibidos por el com. serial
		RxData = '\0';
	}
	else{
		// Aun no se han recibido caractéres a traves de la comunicacion serial
		stringComplete = 0;
	}

	return stringComplete;
}


///*
// * Esta funcion setea el string recibido a traves de comunicacion serial,
// * para poder reconocer si es o no un comando para el CMD
//*/
//void parseCommands(char *prtBufferReception, char *cmd, unsigned int firstParameter, unsigned int secondParameter, char *userMsg){
//
//	/*
//	 * Esta funcion toma el buffer de entrada donde tenemos guardado el string
//	 * y los separa en:
//	 * cmd 				-> String inicial del comando
//	 * firstParameter 	-> Un integer
//	 * secondParameter 	-> Un integer
//	 */
//	sscanf(prtBufferReception, "%s %u %u %s", cmd, &firstParameter, &secondParameter, userMsg);
//
//
//}



/*
 * Esta funcion recibe un puntero al handler que envía datos a traves de la
 * comunicacion serial
 */
void helpMenu(USART_Handler_t handlerCommSerial){

	usart_WriteMsg(&handlerCommSerial, "Help Menu CMD: \n");
	usart_WriteMsg(&handlerCommSerial, "1) Mensaje del menu");



}

// ====== AQUI VAN LAS FUNCIONES QUE MODIFICAN PARAMETROS DEL ROBOT ======
void modSpeed(uint8_t percSpeed){

}


void modDir(uint8_t dir){

}

