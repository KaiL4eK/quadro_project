/*
 * File:   main.c
 * Author: alexey
 *
 * Created on September 7, 2016, 6:47 PM
 */

#include <per_proto.h>
#include "MPU6050.h"

#include <pragmas.h>

int main(void) {
    OFF_ALL_ANALOG_INPUTS;
    
    UART_init( UARTm1, UART_460800, INT_PRIO_HIGH );
    
    i2c_init( 400000 );
    delay_ms( 100 );
    UART_write_string( UARTm1, "/--------------------/\nStart mpu_init\n" );
    
    if ( mpu6050_get_id() != 0x34 ) {
        UART_write_string( UARTm1, "MPU not found\n" );
        while( 1 );
    } 
    
//    mpu6050_init();
    int result = 0;
    if ( (result = mpu6050_dmpInitialize_2()) ) {
        UART_write_string( UARTm1, "Initialization failed: %d\n", result );
        while( 1 );
    }
    
    UART_write_string( UARTm1, "Initialized!\n" );
    
    euler_angles_t angles;
    
    while( 1 ) {
//        UART_write_string( UARTm1, "FIFO count: %d\n", mpu6050_getFIFOCount() );
        if ( mpu6050_dmpPacketAvailable() ) {
            if ( mpu6050_dmpGetEuler( &angles ) == 0 );
                UART_write_string( UARTm1, "Angles: %.2f, %.2f, %.2f\n", angles.roll, angles.pitch, angles.yaw );
        }
//        mpu6050_receive_gyro_accel_raw_data();
//        send_UART_mpu6050_data( UARTm1 );
//        UART_write_string( UARTm1, "Hello\n" );
        delay_ms(1);
    }
}
