/*
 * mpu6050_driver.h
 *
 *  Created on: Jun 25, 2024
 *      Author: aristizabal
 */

#ifndef MPU6050_DRIVER_H_
#define MPU6050_DRIVER_H_

#define PWR_MGMT_1                    0x6b
#define CONFIG_REG                    0x1A



#define MPU6050_ADDRESS               0x68
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



enum {
  MPU6050_RANGE_2_G   = 0x0,  ///< +/- 2g (default value)
  MPU6050_RANGE_4_G   = 0x8,  ///< +/- 4g
  MPU6050_RANGE_8_G   = 0x10,  ///< +/- 8g
  MPU6050_RANGE_16_G  = 0x18, ///< +/- 16g
};



enum {
  MPU6050_RANGE_250_DEG   = 0x0,  ///< +/- 250 deg/s (default value)
  MPU6050_RANGE_500_DEG   = 0x08,  ///< +/- 500 deg/s
  MPU6050_RANGE_1000_DEG  = 0x10, ///< +/- 1000 deg/s
  MPU6050_RANGE_2000_DEG  = 0x18, ///< +/- 2000 deg/s
};


enum {
  MPU6050_BAND_260_HZ, ///< Docs imply this disables the filter
  MPU6050_BAND_184_HZ, ///< 184 Hz
  MPU6050_BAND_94_HZ,  ///< 94 Hz
  MPU6050_BAND_44_HZ,  ///< 44 Hz
  MPU6050_BAND_21_HZ,  ///< 21 Hz
  MPU6050_BAND_10_HZ,  ///< 10 Hz
  MPU6050_BAND_5_HZ,   ///< 5 Hz
};


void begin();
void calibration(float* rateCalibrationArray);
void readAccelData(float* accelData);
void readGyroData(float* gyroData);
void readTempData(float* tempData);
void calculateAngle(float* anglesData,  float* accelData);


//accel_range_t getAccelRange(void);
void setAccelRange(I2C_Handler_t ptrHandlerI2C, uint16_t newRange);
//gyro_range_t getGyroRange(void);
void setGyroRange(I2C_Handler_t ptrHandlerI2C, uint16_t newRange);


void reset(void);


void rawGyroData(int16_t* gyroData);
void rawAccelData(int16_t* gyroData);

void readRegisters(uint8_t reg, uint8_t count,uint8_t* data); //debemos crear la funcion de multiples registros en I2C

void calculateAngle(float* anglesData, float* accelData);




#endif /* MPU6050_DRIVER_H_ */
