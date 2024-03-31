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
#include "pwm_driver_hal.h"
#include "usart_driver_hal.h"
#include "string.h"



/*
 * Esta funcion organiza los datos recibido a traves del puerto serial en un string
 * y devuelve una variable de estado cuando se completa el string
 */
void bufferReception(USART_Handler_t *ptrHandlerCommSerial,uint8_t rxData, char *ptrBufferReception, uint8_t counterReception, cmd_Handler_t *ptrcmdHandler){

	uint8_t stringComplete = 0;

	// Se revisa cual fue el dato recibido por la comunicacion serial
	if(rxData != '\0'){
		ptrBufferReception[counterReception] = rxData;
		counterReception++;

		// Se verifica si el último dato es el caracter de finalizacion de un string
		if (rxData == '@'){
			// Se modifica el estado de una variable de control
			stringComplete = 1;

			// Configuramos las variables para guardar el string y esperar uno nuevo
			ptrBufferReception[counterReception] = '\0';
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
		parseCommands(ptrHandlerCommSerial,ptrBufferReception, ptrcmdHandler);
		stringComplete = 0;
	}

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
void helpMenu(USART_Handler_t *ptrHandlerCommSerial){

	usart_WriteMsg(ptrHandlerCommSerial, "Help Menu CMDS: \n");
	usart_WriteMsg(ptrHandlerCommSerial, "1) Dir 0:forw / 1:back ; dutty(\%) \" Dir # # @\" \n");
	usart_WriteMsg(ptrHandlerCommSerial, "2) Spd \%leftM 		; \%rightM \" Spd # # @\" \n");
	usart_WriteMsg(ptrHandlerCommSerial, "3) Rot 0:left 1:right  ; #turns  \" Rot # # @\" \n");
	usart_WriteMsg(ptrHandlerCommSerial, "4) TestEncoders percDuttyCycle:left \" TestEncoders # @\" \n");

	usart_WriteMsg(ptrHandlerCommSerial, "5) Test 0:left / 1:right; dutty   \" Test # # @\" \n");


	usart_WriteMsg(ptrHandlerCommSerial, "6) Stop \" Stop @\" \n");
	usart_WriteMsg(ptrHandlerCommSerial, "7) Resume \" Resume @\" \n");


}

// ====== AQUI VAN LAS FUNCIONES QUE MODIFICAN PARAMETROS DEL ROBOT ======




void parseCommands(USART_Handler_t *ptrHandlerCommSerial, char  *ptrbufferReception, cmd_Handler_t *ptrcmdHandler){

	sscanf(ptrbufferReception,"%s %u %u %s",ptrcmdHandler->cmd,&ptrcmdHandler->firstParameter,&ptrcmdHandler->secondParameter,ptrcmdHandler->lastString);
	//Comando para solicitar ayuda
	if(strcmp(ptrcmdHandler->cmd, "help") == 0){
		helpMenu(ptrHandlerCommSerial);
	}

	else{
		usart_WriteMsg(ptrHandlerCommSerial, "Comando erroneo.\n Ingresa \"help @\" para ver la lista de comandos.\n");
	}


}




