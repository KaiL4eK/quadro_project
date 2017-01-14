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

typedef enum {
    MPU6050_DLPF_BW_256 = 0x00,
    MPU6050_DLPF_BW_188 = 0x01,
    MPU6050_DLPF_BW_98  = 0x02,
    MPU6050_DLPF_BW_42  = 0x03,
    MPU6050_DLPF_BW_20  = 0x04,
    MPU6050_DLPF_BW_10  = 0x05,
    MPU6050_DLPF_BW_5   = 0x06
} mpu6050_bandwidth_t;

int mpu6050_init ( void );
gyro_accel_data_t *mpu6050_get_raw_data ( void );
int mpu6050_receive_gyro_accel_raw_data ( void );
void mpu6050_set_bandwidth ( mpu6050_bandwidth_t bw );
void mpu6050_calibration ( UART_moduleNum_t uart );

// DMP

bool mpu6050_dmp_packet_available();
int mpu6050_dmp_init();

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

void mpu6050_dmp_get_euler_angles(euler_angles_t *a);

#define RADIANS_TO_DEGREES          57.295779513f

#endif	/* MPU6050_H_ */

