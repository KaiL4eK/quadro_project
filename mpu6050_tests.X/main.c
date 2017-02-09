/*
 * File:   main.c
 * Author: alexey
 *
 * Created on February 9, 2017, 11:40 PM
 */


#include <per_proto.h>
#include <xc.h>
#include <pragmas.h>

#include <stdbool.h>

#include <MPU6050.h>

volatile static uart_module_t   uart_debug      = NULL;
volatile static uart_module_t   uart_interface  = NULL;
volatile gyro_accel_data_t      *mpu_data       = NULL;

void timer_interrupt_initialization( void );
void send_serial_data ( uart_module_t uart );

void error_loop ( const char *str )
{
    while ( 1 )
    {
        UART_write_string( uart_debug, str );
        delay_ms( 1000 );
    }
}

int main ( void ) 
{
    uart_interface  = UART_init( 1, UART_460800, INT_PRIO_HIGH );
    UART_write_set_big_endian_mode( uart_interface, true ); 
    
    uart_debug      = UART_init( 2, UART_460800, INT_PRIO_HIGH );
    UART_write_string( uart_debug, "UART ready\n" );
    
    if ( mpu6050_init( NULL, uart_debug ) != 0 )    
        error_loop( "MPU6050 initialize failed\n" );

    mpu6050_offsets_t mpu6050_offsets = { -3840, 1185, 1912, 21, -10, 81 };

    mpu6050_set_bandwidth( MPU6050_DLPF_BW_42 );
    mpu6050_set_offsets( &mpu6050_offsets );
    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_500 );
    mpu6050_set_accel_fullscale( MPU6050_ACCEL_FS_8 );
//    mpu6050_set_sample_rate_divider( 1 );

    mpu_data = mpu6050_get_raw_data();
    UART_write_string( uart_debug, "MPU6050 ready\n" );    

    timer_interrupt_initialization();
    
    while ( 1 )
    {
    }
    
    return 0;
}

const float INTERRUPT_PERIOD = 1.0/500;

// Generates interrupt each 2 msec
void timer_interrupt_initialization( void )
{
    uint32_t timer_counter_limit = FCY * INTERRUPT_PERIOD;
    
    T2CONbits.TON   = 0;
    T2CONbits.T32   = 1;
    T2CONbits.TCKPS = TIMER_DIV_1;
    _T3IP           = INT_PRIO_HIGHEST;
    _T3IE           = 1;
    PR3             = ((timer_counter_limit >> 16) & 0xffff);
    PR2             = (timer_counter_limit & 0xffff);
    T2CONbits.TON   = 1;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _T3Interrupt()
{
    if ( mpu6050_receive_gyro_accel_raw_data() )
        return;
    
    send_serial_data( uart_interface );
    
    _T3IF = 0;
}

void send_serial_data ( uart_module_t uart )
{
    static bool    data_switch = false;
    
    uint8_t byte    = 0;
    
    if ( data_switch )
    {
        UART_write_words( uart, (uint16_t)mpu_data, 6 );
    }
    
    if ( UART_bytes_available( uart ) == 0 )
        return;
    else
    {
        byte = UART_get_byte( uart );
        UART_write_string( uart, "Check: 0x%x\n", byte );
        
        if ( byte == '1' )
        {
            data_switch = !data_switch;
            UART_write_string( uart, "Serial data changed state to %s\n", data_switch ? "online" : "offline" );
            UART_write_byte( uart, '0' );
        }
    }
}
