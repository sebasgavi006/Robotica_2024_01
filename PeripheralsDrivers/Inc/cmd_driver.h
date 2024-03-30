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


uint8_t bufferReception(uint8_t RxData, char *ptrBufferReception, uint8_t counterReception);
//void parseCommands(char *prtBufferReception, char *cmd, unsigned int firstParameter, unsigned int secondParameter, char *userMsg);
void helpMenu(USART_Handler_t handlerCommSerial);

void modSpeed(uint8_t percSpeed);
void modDir(uint8_t dir);





#endif /* CMD_DRIVER_H_ */
