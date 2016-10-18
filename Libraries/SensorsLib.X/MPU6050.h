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
#include "MPU6050_regs.h"

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
bool mpu6050_test_connection( void );

uint16_t mpu6050_getFIFOCount();
uint8_t mpu6050_getFIFOByte();
void mpu6050_getFIFOBytes( uint8_t *data, uint8_t length );
void mpu6050_resetFIFO();
void mpu6050_setFIFOEnabled( bool enabled );

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
int mpu6050_dmpInitialize();

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

typedef union
{
    struct
    {
        uint8_t quat_w[2];
        uint8_t res1[2];
        uint8_t quat_x[2];
        uint8_t res2[2];
        uint8_t quat_y[2];
        uint8_t res3[2];
        uint8_t quat_z[2];
        uint8_t res4[2];
        uint8_t x_gyro[2];
        uint8_t res5[2];
        uint8_t y_gyro[2];
        uint8_t res6[2];
        uint8_t z_gyro[2];
        uint8_t res7[2];
        uint8_t x_accel[2];
        uint8_t res8[2];
        uint8_t y_accel[2];
        uint8_t res9[2];
        uint8_t z_accel[2];
        uint8_t res10[4];
    } reg;
    struct
    {
        int16_t quat_w;
        uint8_t res1[2];
        int16_t quat_x;
        uint8_t res2[2];
        int16_t quat_y;
        uint8_t res3[2];
        int16_t quat_z;
        uint8_t res4[2];
        int16_t x_gyro;
        uint8_t res5[2];
        int16_t y_gyro;
        uint8_t res6[2];
        int16_t z_gyro;
        uint8_t res7[2];
        int16_t x_accel;
        uint8_t res8[2];
        int16_t y_accel;
        uint8_t res9[2];
        int16_t z_accel;
        uint8_t res10[4];
    } value;
}mpu_fifo_frame_t;

int mpu6050_dmpGetEuler(euler_angles_t *a);
void mpu6050_get_euler( euler_angles_t *angles );

#define RADIANS_TO_DEGREES          57.295779513f

#endif	/* MPU6050_H_ */

