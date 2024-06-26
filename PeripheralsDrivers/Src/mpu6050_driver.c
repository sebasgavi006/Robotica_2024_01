/*
 * mpu6050_driver.c
 *
 *  Created on: Jun 25, 2024
 *      Author: aristizabal
 */

#include "mpu6050_driver.h"
#include "i2c_driver_hal.h"
#include "math.h"


void begin(I2C_Handler_t ptrHandlerI2C){
	i2c_WriteSingleRegister(ptrHandlerI2C, PWR_MGMT_1, 0x0);
}


void setGyroRange(I2C_Handler_t ptrHandlerI2C, uint16_t newRange){
	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_GYRO_CONFIG_REG, newRange);
}

void setAccelRange(I2C_Handler_t ptrHandlerI2C, uint16_t newRange){
	i2c_WriteSingleRegister(ptrHandlerI2C, MPU6050_ACCEL_CONFIG_REG, newRange);
}


void calculateAngle(float* anglesData, float* accelData){
  anglesData[0]   = atan(accelData[1] / sqrt(accelData[0]*accelData[0] + accelData[2]*accelData[2])) * 1/(3.142/180);   //angleRoll
  anglesData[1]   = atan(-accelData[0] / sqrt(accelData[1]*accelData[1] + accelData[2]*accelData[2])) * 1/(3.142/180);  //anglePitch
  anglesData[2]   = atan(sqrt(accelData[0]*accelData[0] + accelData[1]*accelData[1]) / accelData[2] ) * 1/(3.142/180);  //angleYaw
}

