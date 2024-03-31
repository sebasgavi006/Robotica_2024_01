/*
 * cmd_driver.h
 *
 *  Created on: Mar 23, 2024
 *      Author: sebas
 */

#ifndef CMD_DRIVER_H_
#define CMD_DRIVER_H_

#include <stdint.h>
#include "stm32f4xx.h"
#include "usart_driver_hal.h"


typedef struct
{
	char			*cmd;
	unsigned int 	firstParameter;
	unsigned int	secondParameter;
	char			*lastString;
} cmd_Handler_t;


void bufferReception(USART_Handler_t *ptrHandlerCommSerial,uint8_t rxData, char *ptrBufferReception, uint8_t counterReception, cmd_Handler_t *ptrcmdHandler);


void parseCommands(USART_Handler_t *ptrHandlerCommSerial, char  *ptrbufferReception, cmd_Handler_t *ptrcmdHandler);
//void parseCommands(char *prtBufferReception, char *cmd, unsigned int firstParameter, unsigned int secondParameter, char *userMsg);


void helpMenu(USART_Handler_t *ptrHandlerCommSerial);

void modSpeed(uint8_t percSpeed);
void modDir(uint8_t dir);


#endif /* CMD_DRIVER_H_ */
