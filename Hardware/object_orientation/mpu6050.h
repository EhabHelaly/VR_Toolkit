/*
MPU6050 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  - most of the code is a port of the arduino mpu6050 library by Jeff Rowberg
    https://github.com/jrowberg/i2cdevlib
  - Mahony complementary filter for attitude estimation
    http://www.x-io.co.uk
*/


#ifndef MPU6050_H_
#define MPU6050_H_

#include <avr/io.h>
#include "mpu6050registers.h"



//definitions
#define MPU6050_ADDR (0x68<<1) //device address - 0x68 pin low (GND), 0x69 pin high (VCC)
#define COMPASS_ADDR (0x1E<<1)

//HMC338L Registers
#define HMC338L_REGISTER_A 0X00
#define HMC338L_REGISTER_B 0X01
#define HMC338L_MODE_REGISTER 0X02
#define HMC338L_OUTPUT_REGISTER 0X03

//HMC338L gain value
#define HMC338L_LSB_0_point_88 1370.0 
#define HMC338L_LSB_1_point_3  1090.0
#define HMC338L_LSB_1_point_9  820.0
#define HMC338L_LSB_2_point_5  660.0
#define HMC338L_LSB_4          440.0
#define HMC338L_LSB_4_point_7  390.0
#define HMC338L_LSB_5_point_6  330.0
#define HMC338L_LSB_8_point_1  230.0
#define COMPASS_GAIN  HMC338L_LSB_0_point_88 

//value stored at register A
#define COMPASS_OUTPUT_RATE 0X78

//vAlue stored at register B
#define HMC338L_FS_0_point_88 (0<<5)
#define HMC338L_FS_1_point_3  (1<<5)
#define HMC338L_FS_1_point_9  (2<<5)
#define HMC338L_FS_2_point_5  (3<<5)
#define HMC338L_FS_4          (4<<5)
#define HMC338L_FS_4_point_7  (5<<5)
#define HMC338L_FS_5_point_6  (6<<5)
#define HMC338L_FS_8_point_1  (7<<5)
#define COMPASS_FS HMC338L_FS_0_point_88

//value stored at conFiguration register
#define COMPASS_CONTINUSLY_MODE 0X00

//enable the getattitude functions
//because we do not have a magnetometer, we have to start the chip always in the same position
//then to obtain your object attitude you have to apply the aerospace sequence
//0 disabled
//1 mahony filter
//2 dmp chip processor
//#define MPU6050_GETATTITUDE 1

//definitions for raw data
//gyro and acc scale
#define MPU6050_GYRO_FS MPU6050_GYRO_FS_2000
#define MPU6050_ACCEL_FS MPU6050_ACCEL_FS_2

#define MPU6050_GYRO_LSB_250 131.0
#define MPU6050_GYRO_LSB_500 65.5
#define MPU6050_GYRO_LSB_1000 32.8
#define MPU6050_GYRO_LSB_2000 16.4
#if MPU6050_GYRO_FS == MPU6050_GYRO_FS_250
#define MPU6050_GGAIN MPU6050_GYRO_LSB_250
#elif MPU6050_GYRO_FS == MPU6050_GYRO_FS_500
#define MPU6050_GGAIN MPU6050_GYRO_LSB_500
#elif MPU6050_GYRO_FS == MPU6050_GYRO_FS_1000
#define MPU6050_GGAIN MPU6050_GYRO_LSB_1000
#elif MPU6050_GYRO_FS == MPU6050_GYRO_FS_2000
#define MPU6050_GGAIN MPU6050_GYRO_LSB_2000
#endif

#define MPU6050_ACCEL_LSB_2 16384.0
#define MPU6050_ACCEL_LSB_4 8192.0
#define MPU6050_ACCEL_LSB_8 4096.0
#define MPU6050_ACCEL_LSB_16 2048.0
#if MPU6050_ACCEL_FS == MPU6050_ACCEL_FS_2
#define MPU6050_AGAIN MPU6050_ACCEL_LSB_2
#elif MPU6050_ACCEL_FS == MPU6050_ACCEL_FS_4
#define MPU6050_AGAIN MPU6050_ACCEL_LSB_4
#elif MPU6050_ACCEL_FS == MPU6050_ACCEL_FS_8
#define MPU6050_AGAIN MPU6050_ACCEL_LSB_8
#elif MPU6050_ACCEL_FS == MPU6050_ACCEL_FS_16
#define MPU6050_AGAIN MPU6050_ACCEL_LSB_16
#endif


#define MPU6050_AXOFFSET 0
#define MPU6050_AYOFFSET 0
#define MPU6050_AZOFFSET 0
#define MPU6050_AXGAIN 16384.0
#define MPU6050_AYGAIN 16384.0
#define MPU6050_AZGAIN 16384.0
#define MPU6050_GXOFFSET -106
#define MPU6050_GYOFFSET -32
#define MPU6050_GZOFFSET -26
#define MPU6050_GXGAIN 16.4
#define MPU6050_GYGAIN 16.4
#define MPU6050_GZGAIN 16.4


//definitions for attitude 1 function estimation
//#if MPU6050_GETATTITUDE == 1
//setup timer0 overflow event and define madgwickAHRSsampleFreq equal to timer0 frequency
//timerfreq = (FCPU / prescaler) / timerscale
//     timerscale 8-bit = 256
// es. 61 = (16000000 / 1024) / 256
//|(1<<CS00)
#define MPU6050_TIMER0INIT TCCR0 |=(1<<CS02); \
		TIMSK |=(1<<TOIE0);

#define sampleFreq	122.07031			// sample frequency in Hz
#define twoKpDef	(2.0f * 0.5f)	// 2 * proportional gain
#define twoKiDef	(2.0f * 0.0f)	// 2 * integral gain
//#endif




//functions
extern void compass_init();
extern void mpu6050_init();
extern uint8_t mpu6050_testConnection();


extern void mpu6050_setSleepDisabled();
extern void mpu6050_setSleepEnabled();

extern int8_t mpu6050_readBytes(uint8_t regAddr, uint8_t length, uint8_t *data);
extern int8_t mpu6050_readByte(uint8_t regAddr, uint8_t *data);
extern void mpu6050_writeBytes(uint8_t regAddr, uint8_t length, uint8_t* data);
extern void mpu6050_writeByte(uint8_t regAddr, uint8_t data);
extern int8_t mpu6050_readBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);
extern int8_t mpu6050_readBit(uint8_t regAddr, uint8_t bitNum, uint8_t *data);
extern void mpu6050_writeBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
extern void mpu6050_writeBit(uint8_t regAddr, uint8_t bitNum, uint8_t data);


extern void mpu6050_updateQuaternion();
extern void mpu6050_getQuaternion(double *qw, double *qx, double *qy, double *qz);
extern void mpu6050_getRollPitchYaw();
extern void send_data();
extern void compass_getData(int16_t* mx, int16_t* my, int16_t* mz);
extern float invSqrt(float x);
extern void MahonyAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
extern void MahonyAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az);


#endif
