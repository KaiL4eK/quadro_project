#ifndef MPU6050_H_
#define	MPU6050_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xc.h>
#include "per_proto.h"

typedef union
{
  struct
  {
    uint8_t x_accel_h;
    uint8_t x_accel_l;
    uint8_t y_accel_h;
    uint8_t y_accel_l;
    uint8_t z_accel_h;
    uint8_t z_accel_l;
    uint8_t t_h;
    uint8_t t_l;
    uint8_t x_gyro_h;
    uint8_t x_gyro_l;
    uint8_t y_gyro_h;
    uint8_t y_gyro_l;
    uint8_t z_gyro_h;
    uint8_t z_gyro_l;
  } reg;
  struct
  {
    int16_t x_accel;
    int16_t y_accel;
    int16_t z_accel;
    int16_t temperature;
    int16_t x_gyro;
    int16_t y_gyro;
    int16_t z_gyro;
  } value;
}gyro_accel_data_t;

gyro_accel_data_t *mpu6050_init ( void );
void send_UART_mpu6050_data ( UART_moduleNum_t mod );
int mpu6050_receive_gyro_accel_raw_data ( void );
void mpu6050_set_DLPF ( uint8_t value );
void mpu6050_set_sleep_bit ( uint8_t value );
void mpu6050_set_clock_source ( uint8_t value );
void mpu6050_set_gyro_fullscale ( uint8_t value );
void mpu6050_set_accel_fullscale ( uint8_t value );
void mpu6050_set_interrupt_data_rdy_bit ( uint8_t value );
void mpu6050_set_sample_rate_divider ( uint8_t value );

uint8_t mpu6050_get_id ( void );
void mpu6050_reset ( void );
void mpu6050_calibration ( void );

int16_t MPU6050_getXAccelOffset ( void );
void mpu6050_setXAccelOffset ( int16_t offset );
int16_t MPU6050_getYAccelOffset ( void );
void mpu6050_setYAccelOffset ( int16_t offset );
int16_t MPU6050_getZAccelOffset ( void );
void mpu6050_setZAccelOffset ( int16_t offset );
int16_t MPU6050_getXGyroOffset ( void );
void mpu6050_setXGyroOffset ( int16_t offset );
int16_t MPU6050_getYGyroOffset ( void );
void mpu6050_setYGyroOffset ( int16_t offset );
int16_t MPU6050_getZGyroOffset ( void );
void mpu6050_setZGyroOffset ( int16_t offset );

// DMP

bool mpu6050_dmpPacketAvailable();
uint8_t mpu6050_dmpInitialize();

typedef struct
{
    float w;
    float x;
    float y;
    float z;
}quaternion_t;

typedef struct
{
    float roll;
    float pitch;
    float yaw;
}euler_angles_t;

#endif	/* MPU6050_H_ */

