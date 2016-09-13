/*
 * File:   main.c
 * Author: alexey
 *
 * Created on September 7, 2016, 6:47 PM
 */

#include <per_proto.h>
#include <MPU6050.h>

#include <pragmas.h>

euler_angles_t angles;

#define DMP_

int main(void) {
    OFF_ALL_ANALOG_INPUTS;
    
    UART_init( UARTm1, UART_460800, INT_PRIO_HIGH );
    
    i2c_init( 400000 );
    delay_ms( 100 );
    UART_write_string( UARTm1, "/--------------------/\nStart mpu_init\n" );
    
#ifndef DMP_
    if ( !(gyro_accel_data = mpu6050_init()) ) {
        UART_write_string( UARTm1, "Initialization failed\n" );
        while( 1 );
    }
#else
    int result = 0;
    
    delay_ms( 100 );
    
    if ( (result = mpu6050_dmpInitialize()) ) {
        UART_write_string( UARTm1, "Initialization failed: %d\n", result );
        while( 1 );
    }
#endif
    UART_write_string( UARTm1, "Initialized!\n" );
    
    
    
    while( 1 ) {
#ifdef DMP_
        if ( mpu6050_dmpPacketAvailable() ) {
            mpu6050_dmpGetEuler( &angles );
        }
#else   
        mpu6050_receive_gyro_accel_raw_data();
        mpu6050_get_euler( &angles );
        UART_write_string( UARTm1, "Angles: %.2f, %.2f, %.2f\n", angles.roll, angles.pitch, angles.yaw );
#endif
//        send_UART_mpu6050_data( UARTm1 );
//        UART_write_string( UARTm1, "Hello\n" );
        delay_ms(1);
    }
}
