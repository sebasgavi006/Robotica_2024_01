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


// Variables para los comandos
char cmd[128] = {0};
unsigned int firstParameter = 0;
unsigned int secondParameter = 0;
char lastString[32] = {0};


char bufferMsg[128] = {0};
char bufferReceiver[64] = {0};
uint8_t rxData = 0;
uint8_t counterReception = 0;
uint8_t stringComplete = 0;




uint8_t defaultSpeed = 0;


/*
 * Esta funcion organiza los datos recibido a traves del puerto serial en un string
 * y devuelve una variable de estado cuando se completa el string
 */
uint8_t bufferReception(uint8_t rxData, char *ptrBufferReception, uint8_t counterReception){

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
		parseCommands(bufferReceiver);
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
			if (defaultSpeed != 0){
				forwardMove(secondParameter);
			}
			else{
				forwardMove(10);
			}
			usart_WriteMsg(&usart1Comm, "Moviéndose hacia adelante \n");
		}
		else if (firstParameter == 1 && secondParameter >= 0){
			if (defaultSpeed != 0){
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
			if(flagEncR){
				sprintf(bufferMsg,"Right,%u\n", counter_R);
				usart_WriteMsg(&usart1Comm, bufferMsg);
				flagEncR = 0;
			}
			if(flagEncL){
				sprintf(bufferMsg,"Left,%u\n", counter_L);
				usart_WriteMsg(&usart1Comm, bufferMsg);
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

