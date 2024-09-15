/*
 * i2c_driver_hal.h
 *
 *  Created on: Oct 31, 2023
 *      Author: sgaviriav
 */

#ifndef I2C_DRIVER_HAL_H_
#define I2C_DRIVER_HAL_H_

#include "stm32f4xx.h"

/* Definición para leer o escribir con el I2C */
enum
{
	I2C_WRITE_DATA = 0,
	I2C_READ_DATA
};

/* Definición de constantes para la señal de reloj que utilizará el periférico I2C */
enum
{
	MAIN_CLOCK_4_MHz_FOR_I2C	= 4,
	MAIN_CLOCK_16_MHz_FOR_I2C	= 16,
	MAIN_CLOCK_20_MHz_FOR_I2C	= 20,
	MAIN_CLOCK_50_MHz_FOR_I2C	= 50,
	MAIN_CLOCK_100_MHz_FOR_I2C	= 100
};

/* Constantes para los dos modos de operación del I2C */
enum
{
	I2C_MODE_SM = 0,
	I2C_MODE_FM
};

/* Velocidades del periférico según el modo de opercación */
enum
{
	/*----------------16 MHz-------------------*/
	I2C_MODE_SM_SPEED_100KHz_16MHz	=	80,
	I2C_MODE_FM_SPEED_400KHz_16MHz	=	14,

	/*----------------80 MHz-------------------*/
	I2C_MODE_SM_SPEED_100KHz_80MHz	=	200,
	I2C_MODE_FM_SPEED_400KHz_80MHz	=	33,

	/*----------------100 MHz-------------------*/
	I2C_MODE_SM_SPEED_100KHz_100MHz	= 500,
	I2C_MODE_FM_SPEED_400KHz_100MHz	= 83
};

/* Valores para el máximo tiempo del flanco de subida según el modo de operación del I2C */
enum
{
	I2C_MAX_RISE_TIME_SM_16MHZ		= 17,
	I2C_MAX_RISE_TIME_FM_16MHz		= 6,

	I2C_MAX_RISE_TIME_SM_80MHZ		= 81,
	I2C_MAX_RISE_TIME_FM_80MHz		= 25,

	I2C_MAX_RISE_TIME_SM_100MHZ		= 101,
	I2C_MAX_RISE_TIME_FM_100MHz		= 31
};


/* Definición del Handler del I2C */
typedef struct
{
	I2C_TypeDef		*ptrI2Cx;
	uint8_t			slaveAddress;
	uint8_t			modeI2C;
	uint8_t			dataI2C;
	uint8_t			mainClock;
} I2C_Handler_t;


/* Definición de las funciones públicas */
void i2c_Config(I2C_Handler_t *ptrHandlerI2C);
void i2c_StartTransaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_ReStartTransaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_SendSlaveAddressRW(I2C_Handler_t *ptrHandlerI2C, uint8_t slaveAddress, uint8_t readOrWrite);
void i2c_SendMemoryAddress(I2C_Handler_t *ptrHandlerI2C, uint8_t memAddress);
void i2c_SendDataByte(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite);
uint8_t i2c_ReadDataByte(I2C_Handler_t *ptrHandlerI2C);
void i2c_StopTransaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_SendAck(I2C_Handler_t *ptrHandlerI2C);
void i2c_SendNoAck(I2C_Handler_t *ptrHandlerI2C);

/* Funciones públicas de lectura y escritura mediante I2C */
uint8_t i2c_ReadSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead);
void i2c_ReadRegisters(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead,uint8_t numberByte, uint8_t* data);
void i2c_WriteSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead, uint8_t newValue);



#endif /* I2C_DRIVER_HAL_H_ */

