/*
 * task_handler.c
 *
 *  Created on: Aug 2, 2024
 *      Author: aristizabal
 */

/* Importamos las librerías */
#include "main.h"

/* Prototipos */
void process_command(command_t *cmd);
int extract_command(command_t *cmd);

const char *msg_invalid = "\n---- Invalid option ----\n";
const char *msg_option_0 =  "\n---- Selected option - 0 ----\n";
const char *msg_option_1 =  "\n---- Selected option - 1 ----\n";
const char *msg_option_2 =  "\n---- Selected option - 2 ----\n";
const char *msg_option_n =  "\n---- Option out of range ----\n";


/* Función que gobierna la tarea Menu */
void vTask_Menu(void* pvParameters){

	uint32_t cmd_addr;
	command_t *cmd;
	int option;

	const char *msg_menu = 	"---------------------\n"
							"|		  Menu		   |"
							"----------------------\n"
							"LED effect		---> 0\n"
							"Date and Time	---> 0\n"
							"Exit			---> 0\n"
							"Enter your choice here:\n";

	while(1){

		/* Envía a imprimir en la consola lo que se debe mostrar */
		xQueueSend(xQueue_Print, &msg_menu, portMAX_DELAY);


		/* Se queda esperando a recibir el comando que se debe ejecutar */
		xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
		cmd = (command_t*) cmd_addr;

		/* El comando recibido solo tiene el largo de 1 caracter */
		if(cmd->len == 1){
			/* Trandformando un ASCII a un numero... */
			option = cmd->payload[0] - 48;

			switch(option){
			case 0:
				/* Envia a imprimir en la consola lo que se debe mostrar en el menu */
				xQueueSend(xQueue_Print, &msg_option_0, portMAX_DELAY);


				/* Aca se debería notificar cambiar la variable next_state y notificar a la tarea Menu */
				next_state = sMainMenu;
				xTaskNotify(xHandleTask_Menu, 0, eNoAction);

				break;

			case 1:
				/* Envia a imprimir en la consola lo que se debe mostrar en el menu */
				xQueueSend(xQueue_Print, &msg_option_1, portMAX_DELAY);


				/* Aca se debería notificar cambiar la variable next_state y notificar a la tarea Menu */
				next_state = sMainMenu;
				xTaskNotify(xHandleTask_Menu, 0, eNoAction);

				break;

			case 2:
				/* Envia a imprimir en la consola lo que se debe mostrar en el menu */
				xQueueSend(xQueue_Print, &msg_option_2, portMAX_DELAY);


				/* Aca se debería notificar cambiar la variable next_state y notificar a la tarea Menu */
				next_state = sMainMenu;
				xTaskNotify(xHandleTask_Menu, 0, eNoAction);

				break;

			default:
				xQueueSend(xQueue_Print, &msg_option_n, portMAX_DELAY);
				/* El continue permite saltar los case del switch case y sale de la selección */
				continue;

			} // Fin del switch case
		}
		else{
			xQueueSend(xQueue_Print, &msg_invalid, portMAX_DELAY);
			/* Aca se debería notificar cambiar la variable next_state y notificar a la tarea Menu */
			next_state = sMainMenu;
			xTaskNotify(xHandleTask_Menu, 0, eNoAction);

		} // Fin del ciclo if

		/* La tarea vuelve a quedar en un estado de espera por un tiempo indefinido */
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
	}//end loop

}


/* Función que gobierna la tarea Print */
void vTask_Print(void* pvParameters){

	uint32_t *msg;

	while(1){
		/* Recive de la cola de impresión el comando */
		xQueueReceive(xQueue_Print, &msg, portMAX_DELAY);
		usart_WriteMsg(&commTerm_Handler, (char*)msg);
	}//end loop

}


/* Función que gobierna el Commands */
void vTask_Commands(void* pvParameters){

	BaseType_t notify_status = {0};
	command_t cmd = {0};

	while(1){

		/* Esperamos la notificación desde la interrupción */
		notify_status = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

		/* Si es TRUE significa que se recibió la notificación */
		if(notify_status == pdTRUE){
			/* Procesa el comando */
			/* Notificar a la tarea que debe continuar */
			process_command(&cmd);
		}

	}//end loop
}


/* Función que ayuda a procesar el comando */
void process_command(command_t *cmd){

	extract_command(cmd);

	switch(next_state){
	case sMainMenu:
		/* Notificamos a la tarea respectiva */
		xTaskNotify(xHandleTask_Menu, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	case sLedEffect:
		/* Notificamos a la tarea respectiva */
		//xTaskNotify(xHandleTask_Leds, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	case sRtcMenu:
		/* Notificamos a la tarea respectiva */
		//xTaskNotify(xHandleTask_Rtc, (uint32_t)cmd, eSetValueWithOverwrite);
		break;

	default:
		__NOP();
		break;
	}

}


/* Función que extrae el comando */
int extract_command(command_t *cmd){

	uint8_t item;
	uint8_t counter_j = 0;
	BaseType_t status;

	status = uxQueueMessagesWaiting(xQueue_InputData);
	if(status == 0){
		return -1;
	}

	do{
		/* Recibimos un elemento y lo montamos en 'item' */
		status = xQueueReceive(xQueue_InputData, &item, 0);
		if(status == pdTRUE){
			/* Vamos llenando el arreglo del comando */
			cmd->payload[counter_j++] = item;
		}

	}while(item != '#');

	cmd->payload[counter_j -1] = '\0';
	cmd->len = counter_j -1; // Longitud de los caracteres del comando sin contar el array

	return 0;

}


