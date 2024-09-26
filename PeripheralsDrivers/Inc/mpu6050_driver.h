/*
 * mpu6050_driver.h
 *
 *  Created on: Jun 25, 2024
 *      Author: aristizabal
 */

#ifndef MPU6050_DRIVER_H_

#include <stdint.h>
#include "i2c_driver_hal.h"
#include "stm32f4xx.h"



#define MPU6050_DRIVER_H_

#define PWR_MGMT_1                    0x6b
#define CONFIG_REG                    0x1A
#define WHO_AM_I					  0x75



#define MPU6050_ADDRESS               0b1101000//0x68
#define MPU6050_ADDRESS_0             0b1101001//0x69
#define MPU6050_ACCEL_CONFIG_REG      0x1C
#define MPU6050_GYRO_CONFIG_REG       0x1B
#define MPU6050_ACCEL_XOUT_H_REG      0x3B
#define MPU6050_TEMP_XOUT_H_REG       0x41
#define MPU6050_GYRO_XOUT_H_REG       0x43


#define MPU6050_MOT_THR_REG           0x1F    /// Motion detection threshold bits [7:0]
#define MPU6050_MOT_DUR_REG           0x20    /// Duration counter threshold for motion int. 1 kHz rate, LSB = 1 ms

#define MPU6050_INT_PIN_CFG_REG       0x37
#define MPU6050_INT_ENABLE_REG        0x38
#define MPU6050_INT_STATUS_REG        0x3A



typedef enum {
  ACCEL_RANGE_2_G   = 0x0,  ///< +/- 2g (default value)
  ACCEL_RANGE_4_G   = 0x8,  ///< +/- 4g
  ACCEL_RANGE_8_G   = 0x10,  ///< +/- 8g
  ACCEL_RANGE_16_G  = 0x18, ///< +/- 16g
}eAccelRange_t;



typedef enum {
  GYRO_RANGE_250_DEG   = 0x0,  ///< +/- 250 deg/s (default value)
  GYRO_RANGE_500_DEG   = 0x08,  ///< +/- 500 deg/s
  GYRO_RANGE_1000_DEG  = 0x10, ///< +/- 1000 deg/s
  GYRO_RANGE_2000_DEG  = 0x18, ///< +/- 2000 deg/s
}eGyroRange_t;



typedef enum {
    DLPF_260HZ = 0x00, // BW: 260Hz, Delay: 0ms
    DLPF_184HZ = 0x01, // BW: 184Hz, Delay: 2.0ms
    DLPF_94HZ  = 0x02, // BW: 94Hz,  Delay: 3.0ms
    DLPF_44HZ  = 0x03, // BW: 44Hz,  Delay: 4.9ms
    DLPF_21HZ  = 0x04, // BW: 21Hz,  Delay: 8.5ms
    DLPF_10HZ  = 0x05, // BW: 10Hz,  Delay: 13.8ms
    DLPF_5HZ   = 0x06, // BW: 5Hz,   Delay: 19.0ms
    DLPF_RESERVED = 0x07  // Reservado, no usar
} eDLPFConfig_t;

enum {
  byte_1 = 1,  ///< +/- 250 deg/s (default value)
  byte_2 = 2,  ///< +/- 500 deg/s
  byte_3 = 3, ///< +/- 1000 deg/s
  byte_4 = 4, ///< +/- 2000 deg/s
  byte_5 = 5,
  byte_6 = 6
};


enum {
  dataTypeAccel,  ///< +/- 250 deg/s (default value)
  dataTypeGyro,  ///< +/- 500 deg/s
  dataTypeTemp ///< +/- 1000 deg/s
};

void imuBegin(I2C_Handler_t* ptrHandlerI2C);
uint8_t imuWhoAmI(I2C_Handler_t* ptrHandlerI2C);
void calibration(float* rateCalibrationArray);
float calculateAngle(float* anglesData,  float* accelData);


//accel_range_t getAccelRange(void);
void setAccelRange(I2C_Handler_t* ptrHandlerI2C, eAccelRange_t Range);
//gyro_range_t getGyroRange(void);
void setGyroRange(I2C_Handler_t* ptrHandlerI2C, eGyroRange_t Range);
void setAccelDLPF(I2C_Handler_t* ptrHandlerI2C, eDLPFConfig_t dlpfCfg) ;


void reset(void);

void readAccel(I2C_Handler_t* ptrHandlerI2C, float* dataArray);
void readGyro(I2C_Handler_t* ptrHandlerI2C, float* dataArray);
void readTemp(I2C_Handler_t* ptrHandlerI2C, float* temp);

void gyroStaticCalibration(I2C_Handler_t* ptrHandlerI2C, int numReadings);


void rawData(I2C_Handler_t* ptrHandlerI2C, uint8_t* rawArray , uint8_t dataType);
void readData(uint8_t* rawArray ,float* outData, uint8_t dataType, uint8_t sensorCfg);

void readRegisters(uint8_t reg, uint8_t count,uint8_t* data); //debemos crear la funcion de multiples registros en I2C





#endif /* MPU6050_DRIVER_H_ */
