/*
 * mpu6050_driver.c
 *
 *  Created on: Jun 25, 2024
 *      Author: aristizabal
 */

#include "mpu6050_driver.h"
#include "i2c_driver_hal.h"
#include "math.h"


// Variable global para la escala (LSB/g)
float accelScale = 16384.0f;  // Valor inicial para ±2g
float gyroScale = 131.0f;  // Valor inicial para ±250 deg/s
float offsetGyroData[3] = {0};
int calibFlag=0;




void imuBegin(I2C_Handler_t* ptrHandlerI2C){
	i2c_WriteSingleRegister(ptrHandlerI2C, PWR_MGMT_1, 0x0);
}

uint8_t imuWhoAmI(I2C_Handler_t* ptrHandlerI2C){

	uint8_t WhoAmI = i2c_ReadSingleRegister(ptrHandlerI2C,WHO_AM_I);

	return WhoAmI;
}

void setAccelRange(I2C_Handler_t* ptrHandlerI2C, eAccelRange_t Range){
//	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_ACCEL_CONFIG_REG, Range);
	uint8_t regValue = i2c_ReadSingleRegister(ptrHandlerI2C, MPU6050_ACCEL_CONFIG_REG);

	//limpiamos SOLO los bits AFS_SEL[1:0] que son los bits 3 y 4 del registro
	regValue &= ~0x18;//mascara 0b11000
	//establecemos el nuevo rango
	regValue |= Range;
	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_ACCEL_CONFIG_REG, regValue);

	// Configurar el valor de escala basado en el rango seleccionado
    switch (Range) {
        case ACCEL_RANGE_2_G:
            accelScale = 16384.0f;  // LSB/g para ±2g
            break;
        case ACCEL_RANGE_4_G:
            accelScale = 8192.0f;   // LSB/g para ±4g
            break;
        case ACCEL_RANGE_8_G:
            accelScale = 4096.0f;   // LSB/g para ±8g
            break;
        case ACCEL_RANGE_16_G:
            accelScale = 2048.0f;   // LSB/g para ±16g
            break;
        default:
            accelScale = 16384.0f;  // Valor por defecto en caso de error
            break;
    }




}

void setGyroRange(I2C_Handler_t* ptrHandlerI2C, eGyroRange_t Range){
//	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_GYRO_CONFIG_REG, newRange);
	uint8_t regValue = i2c_ReadSingleRegister(ptrHandlerI2C, MPU6050_GYRO_CONFIG_REG);
	//limpiamos SOLO los bits AFS_SEL[1:0] que son los bits 3 y 4 del registro
	regValue &= ~0x18;//mascara 0b11000
	//establecemos el nuevo rango
	regValue |= Range;
	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_GYRO_CONFIG_REG, regValue);
	// Configurar el valor de escala basado en el rango seleccionado
    switch (Range) {
        case GYRO_RANGE_250_DEG:
        	gyroScale = 131.0f;  // LSB/g para ±2g
            break;
        case GYRO_RANGE_500_DEG:
        	gyroScale = 65.5f;   // LSB/g para ±4g
            break;
        case GYRO_RANGE_1000_DEG:
        	gyroScale = 32.8f;   // LSB/g para ±8g
            break;
        case GYRO_RANGE_2000_DEG:
        	gyroScale = 16.4f;   // LSB/g para ±16g
            break;
        default:
        	gyroScale = 131.0f;  // Valor por defecto en caso de error
            break;
    }
}

void readAccel(I2C_Handler_t* ptrHandlerI2C, float* dataArray){
	uint8_t rawData[6] = {0};
	i2c_ReadRegisters(ptrHandlerI2C, MPU6050_ACCEL_XOUT_H_REG, 6, rawData);
    int16_t accelX = (int16_t)((rawData[0] << 8) | rawData[1]);
    int16_t accelY = (int16_t)((rawData[2] << 8) | rawData[3]);
    int16_t accelZ = (int16_t)((rawData[4] << 8) | rawData[5]);

    dataArray[0] = accelX / accelScale;
    dataArray[1] = accelY / accelScale;
    dataArray[2] = accelZ / accelScale;
}

void readGyro(I2C_Handler_t* ptrHandlerI2C, float* dataArray){
	uint8_t rawData[6] = {0};
	i2c_ReadRegisters(ptrHandlerI2C, MPU6050_GYRO_XOUT_H_REG, 6, rawData);
    int16_t gyroX = (int16_t)((rawData[0] << 8) | rawData[1]);
    int16_t gyroY = (int16_t)((rawData[2] << 8) | rawData[3]);
    int16_t gyroZ = (int16_t)((rawData[4] << 8) | rawData[5]);

    dataArray[0] = gyroX / gyroScale;
    dataArray[1] = gyroY / gyroScale;
    dataArray[2] = gyroZ / gyroScale;

    if (calibFlag) {
    	dataArray[0] = dataArray[0] - offsetGyroData[0];
    	dataArray[1] = dataArray[1] - offsetGyroData[1];
    	dataArray[2] = dataArray[2] - offsetGyroData[2];

	}
}

void readTemp(I2C_Handler_t* ptrHandlerI2C, float* temp){
	uint8_t rawData[2] = {0};
	i2c_ReadRegisters(ptrHandlerI2C, MPU6050_TEMP_XOUT_H_REG, 2, rawData);
    int16_t tempRaw = (int16_t)((rawData[0] << 8) | rawData[1]);

    *temp = (tempRaw / 340.0f) + 36.53f;
}


void gyroStaticCalibration(I2C_Handler_t* ptrHandlerI2C, int numReadings){
	float sumX = 0;
	float sumY = 0;
	float sumZ = 0;
	float auxGyroData[3] = {0};

	for (int i = 0; i < numReadings; ++i) {

		readGyro(ptrHandlerI2C,auxGyroData);
		sumX += auxGyroData[0];
		sumY += auxGyroData[1];
		sumZ += auxGyroData[2];
	}

	offsetGyroData[0] = sumX /numReadings;
	offsetGyroData[1] = sumY /numReadings;
	offsetGyroData[2] = sumZ /numReadings;
	calibFlag = 1;

}





void readData(uint8_t* rawArray ,float* outData, uint8_t dataType, uint8_t sensorCfg){

	if(dataType == dataTypeGyro){
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

void setAccelDLPF(I2C_Handler_t* ptrHandlerI2C, eDLPFConfig_t dlpfCfg) {
    uint8_t regValue = i2c_ReadSingleRegister(ptrHandlerI2C, 0x1A);

    // Limpiar los bits correspondientes al DLPF_CFG [2:0]
    regValue &= ~0x07; // Mascara 0b00000111

    // Establecer el nuevo valor para DLPF_CFG
    regValue |= dlpfCfg;

    // Escribir el valor configurado de nuevo en el registro CONFIG
    i2c_WriteSingleRegister(ptrHandlerI2C, 0x1A, regValue);
}
