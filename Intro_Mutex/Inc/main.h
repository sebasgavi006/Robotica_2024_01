/*
 * main.h
 *
 *      Author: Sebastian Gaviria
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <gpio_driver_hal.h>
#include <pll_driver_hal.h>
#include <exti_driver_hal.h>
#include <usart_driver_hal.h>


typedef struct {
	uint8_t payload[10];	//informacion recibida por el serial
	uint32_t len;			//tamao de los datos recibidos
}command_t;


typedef enum {
	sMainMenu =0,
	sLedEffect,
	sRtcMenu
}state_t;


//Handle de las tareas
extern TaskHandle_t xHandleTask_Menu;
extern TaskHandle_t xHandleTask_Print;
extern TaskHandle_t xHandleTask_Commands;

//Cabeceras de las funciones del FreeRTOS
void vTask_Menu(void* pvParameters);
void vTask_Print(void* pvParameters);
void vTask_Commands(void* pvParameters);


void led_state_callback(TimerHandle_t xTimer);

extern QueueHandle_t xQueue_InputData;
extern QueueHandle_t xQueue_Print;

extern state_t next_state;

extern USART_Handler_t commTerm_Handler;

#endif /* MAIN_H_ */
