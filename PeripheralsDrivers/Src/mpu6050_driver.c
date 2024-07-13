/*
 * mpu6050_driver.c
 *
 *  Created on: Jun 25, 2024
 *      Author: aristizabal
 */

#include "mpu6050_driver.h"
#include "i2c_driver_hal.h"
#include "math.h"


void begin(I2C_Handler_t* ptrHandlerI2C){
	i2c_WriteSingleRegister(ptrHandlerI2C, PWR_MGMT_1, 0x0);
}


void setGyroRange(I2C_Handler_t* ptrHandlerI2C, uint16_t newRange){
	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_GYRO_CONFIG_REG, newRange);
}

void setAccelRange(I2C_Handler_t* ptrHandlerI2C, uint16_t newRange){
	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_ACCEL_CONFIG_REG, newRange);
}


float calculateAngle(float* anglesData, float* accelData){
  anglesData[0]   = atan(accelData[1] / sqrt(accelData[0]*accelData[0] + accelData[2]*accelData[2])) * 1/(M_PI/180);   //angleRoll
  anglesData[1]   = atan(-accelData[0] / sqrt(accelData[1]*accelData[1] + accelData[2]*accelData[2])) * 1/(M_PI/180);  //anglePitch
  anglesData[2]   = atan(sqrt(accelData[0]*accelData[0] + accelData[1]*accelData[1]) / accelData[2] ) * 1/(M_PI/180);  //angleYaw
  return *anglesData;
}


void rawData(I2C_Handler_t* ptrHandlerI2C, uint8_t* rawArray , uint8_t dataType){

	uint8_t rawData[6] = {0};


	switch (dataType) {
		case dataTypeAccel:
			i2c_ReadRegisters(ptrHandlerI2C, MPU6050_ACCEL_XOUT_H_REG, 6, rawData);
			rawArray[0] = ((int16_t)rawData[0] << 8) | rawData[1];
			rawArray[1] = ((int16_t)rawData[2] << 8) | rawData[3];
			rawArray[2] = ((int16_t)rawData[4] << 8) | rawData[5];

			break;
		case dataTypeGyro:
			i2c_ReadRegisters(ptrHandlerI2C, MPU6050_GYRO_XOUT_H_REG, 6, rawData);
			rawArray[0] = ((int16_t)rawData[0] << 8) | rawData[1];
			rawArray[1] = ((int16_t)rawData[2] << 8) | rawData[3];
			rawArray[2] = ((int16_t)rawData[4] << 8) | rawData[5];

			break;
		case dataTypeTemp:
			i2c_ReadRegisters(ptrHandlerI2C, MPU6050_TEMP_XOUT_H_REG, 2, rawData);
			rawArray[0] = ((int16_t)rawData[0] << 8) | rawData[1];

			break;

		default:
			break;
	}
}



void readData(uint8_t* rawArray ,float* outData, uint8_t dataType, uint8_t sensorCfg){


	if(dataType == dataTypeAccel){
		switch (sensorCfg) {
			case MPU6050_RANGE_2_G:
				outData[0] = rawArray[0] / 16384;
				outData[1] = rawArray[1] / 16384;
				outData[2] = rawArray[2] / 16384;
				break;
			case MPU6050_RANGE_4_G:
				outData[0] = rawArray[0] / 8192;
				outData[1] = rawArray[1] / 8192;
				outData[2] = rawArray[2] / 8192;

							break;
			case MPU6050_RANGE_8_G:
				outData[0] = rawArray[0] / 4096;
				outData[1] = rawArray[1] / 4096;
				outData[2] = rawArray[2] / 4096;

							break;
			case MPU6050_RANGE_16_G:
				outData[0] = rawArray[0] / 2048;
				outData[1] = rawArray[1] / 2048;
				outData[2] = rawArray[2] / 2048;

							break;
			default:
				break;
		}

	}

	else if(dataType == dataTypeGyro){
			switch (sensorCfg) {
				case MPU6050_RANGE_250_DEG:
					outData[0] = rawArray[0] / 131;
					outData[1] = rawArray[1] / 131;
					outData[2] = rawArray[2] / 131;
					break;
				case MPU6050_RANGE_500_DEG:
					outData[0] = rawArray[0] / 65.5;
					outData[1] = rawArray[1] / 65.5;
					outData[2] = rawArray[2] / 65.5;
								break;
				case MPU6050_RANGE_1000_DEG:
					outData[0] = rawArray[0] / 32.8;
					outData[1] = rawArray[1] / 32.8;
					outData[2] = rawArray[2] / 32.8;
								break;
				case MPU6050_RANGE_2000_DEG:
					outData[0] = rawArray[0] / 16.4;
					outData[1] = rawArray[1] / 16.4;
					outData[2] = rawArray[2] / 16.4;
								break;
				default:
					break;
			}
		}
	else if(dataType == dataTypeTemp){
		outData[0] = (rawArray[0] / 340.0) + 36.53;
	}


}

