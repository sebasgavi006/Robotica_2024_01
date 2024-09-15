/*
 * mpu6050_driver.c
 *
 *  Created on: Jun 25, 2024
 *      Author: aristizabal
 */

#include "mpu6050_driver.h"
#include "i2c_driver_hal.h"
#include "math.h"


void imuBegin(I2C_Handler_t* ptrHandlerI2C){
	i2c_WriteSingleRegister(ptrHandlerI2C, PWR_MGMT_1, 0x0);
}

void setAccelRange(I2C_Handler_t* ptrHandlerI2C, eAccelRange_t Range){
	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_ACCEL_CONFIG_REG, Range);
}

void setGyroRange(I2C_Handler_t* ptrHandlerI2C, uint16_t newRange){
	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_GYRO_CONFIG_REG, newRange);
}




void readAccel(I2C_Handler_t* ptrHandlerI2C, float* dataArray){
	uint8_t rawData[6] = {0};
	i2c_ReadRegisters(ptrHandlerI2C, MPU6050_ACCEL_XOUT_H_REG, 6, rawData);
    int16_t accelX = (int16_t)((rawData[0] << 8) | rawData[1]);
    int16_t accelY = (int16_t)((rawData[2] << 8) | rawData[3]);
    int16_t accelZ = (int16_t)((rawData[4] << 8) | rawData[5]);

    dataArray[0] = accelX / 16384.0f;
    dataArray[1] = accelY / 16384.0f;
    dataArray[2] = accelZ / 16384.0f;
}

void readGyro(I2C_Handler_t* ptrHandlerI2C, float* dataArray){
	uint8_t rawData[6] = {0};
	i2c_ReadRegisters(ptrHandlerI2C, MPU6050_GYRO_XOUT_H_REG, 6, rawData);
    int16_t gyroX = (int16_t)((rawData[0] << 8) | rawData[1]);
    int16_t gyroY = (int16_t)((rawData[2] << 8) | rawData[3]);
    int16_t gyroZ = (int16_t)((rawData[4] << 8) | rawData[5]);

    dataArray[0] = gyroX / 131.0f;
    dataArray[1] = gyroY / 131.0f;
    dataArray[2] = gyroZ / 131.0f;
}

void readTemp(I2C_Handler_t* ptrHandlerI2C, float* temp){
	uint8_t rawData[2] = {0};
	i2c_ReadRegisters(ptrHandlerI2C, MPU6050_TEMP_XOUT_H_REG, 2, rawData);
    int16_t tempRaw = (int16_t)((rawData[0] << 8) | rawData[1]);

    *temp = (tempRaw / 340.0f) + 36.53f;
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
			case ACCEL_RANGE_2_G:
				outData[0] = rawArray[0] / 16384;
				outData[1] = rawArray[1] / 16384;
				outData[2] = rawArray[2] / 16384;
				break;
			case ACCEL_RANGE_4_G:
				outData[0] = rawArray[0] / 8192;
				outData[1] = rawArray[1] / 8192;
				outData[2] = rawArray[2] / 8192;

							break;
			case ACCEL_RANGE_8_G:
				outData[0] = rawArray[0] / 4096;
				outData[1] = rawArray[1] / 4096;
				outData[2] = rawArray[2] / 4096;

							break;
			case ACCEL_RANGE_16_G:
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
				case GYRO_RANGE_250_DEG:
					outData[0] = rawArray[0] / 131;
					outData[1] = rawArray[1] / 131;
					outData[2] = rawArray[2] / 131;
					break;
				case GYRO_RANGE_500_DEG:
					outData[0] = rawArray[0] / 65.5;
					outData[1] = rawArray[1] / 65.5;
					outData[2] = rawArray[2] / 65.5;
								break;
				case GYRO_RANGE_1000_DEG:
					outData[0] = rawArray[0] / 32.8;
					outData[1] = rawArray[1] / 32.8;
					outData[2] = rawArray[2] / 32.8;
								break;
				case GYRO_RANGE_2000_DEG:
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


float calculateAngle(float* anglesData, float* accelData){
  anglesData[0]   = atan(accelData[1] / sqrt(accelData[0]*accelData[0] + accelData[2]*accelData[2])) * 1/(M_PI/180);   //angleRoll
  anglesData[1]   = atan(-accelData[0] / sqrt(accelData[1]*accelData[1] + accelData[2]*accelData[2])) * 1/(M_PI/180);  //anglePitch
  anglesData[2]   = atan(sqrt(accelData[0]*accelData[0] + accelData[1]*accelData[1]) / accelData[2] ) * 1/(M_PI/180);  //angleYaw
  return *anglesData;
}
