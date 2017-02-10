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

volatile static uart_module_t       uart_debug      = NULL;
volatile static uart_module_t       uart_interface  = NULL;
volatile static gyro_accel_data_t   *mpu_data       = NULL;

void timer_interrupt_initialization( void );
void send_serial_data ( uart_module_t uart, uart_module_t debug );
void uart_response ( uart_module_t uart );

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
    delay_ms( 300 );
    
    uart_interface  = UART_init( 1, UART_460800, INT_PRIO_HIGH );
    UART_write_set_big_endian_mode( uart_interface, true ); 
    
    uart_debug      = UART_init( 2, UART_460800, INT_PRIO_HIGH );
    UART_write_string( uart_debug, "UART ready\n" );
    
    i2c_init( 1, 400000L ); 
    UART_write_string( uart_debug, "I2C ready\n" );
    
    if ( mpu6050_init( NULL, uart_debug ) != 0 )    
        error_loop( "MPU6050 initialize failed\n" );

//    mpu6050_offsets_t mpu6050_offsets = { -3840, 1185, 1912, 21, -10, 81 };   // Home data
    mpu6050_offsets_t mpu6050_offsets = { -3473, -3008, 1743, 44, -68, -17 };     // ASC data

    
    mpu6050_set_bandwidth( MPU6050_DLPF_BW_42 );
    mpu6050_set_offsets( &mpu6050_offsets );
    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_500 );
    mpu6050_set_accel_fullscale( MPU6050_ACCEL_FS_2 );
//    mpu6050_set_sample_rate_divider( 1 );

    mpu_data = mpu6050_get_raw_data();
    UART_write_string( uart_debug, "MPU6050 ready\n" );    

//    mpu6050_calibration();
    
    _TRISA9 = 1;
    
    timer_interrupt_initialization();
    
    while ( 1 )
    {
    }
    
    return 0;
}

const float INTERRUPT_PERIOD = 1.0/1000;

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
    
    uart_response( uart_debug );
    send_serial_data( uart_interface, uart_debug );
    
    _T3IF = 0;
}

void uart_response ( uart_module_t uart )
{
    if ( UART_bytes_available( uart ) )
    {
        UART_write_string( uart, "Clicked: %c\n", UART_get_byte( uart ) );
    }
}

void send_serial_data ( uart_module_t uart, uart_module_t debug )
{
    static bool    data_switch = false;
    static int16_t buffer[7];
    
    uint8_t byte    = 0;
    
    if ( data_switch )
    {
        int i = 0;
        
        buffer[i++] = mpu_data->value.x_accel;
        buffer[i++] = mpu_data->value.y_accel;
        buffer[i++] = mpu_data->value.z_accel;
        buffer[i++] = mpu_data->value.x_gyro;
        buffer[i++] = mpu_data->value.y_gyro;
        buffer[i++] = mpu_data->value.z_gyro;   
        buffer[i++] = _RA9;   
        
        UART_write_words( uart, buffer, sizeof( buffer )/sizeof( *buffer ) );
    }
    
    if ( UART_bytes_available( uart ) )
    {
        byte = UART_get_byte( uart );
        UART_write_string( debug, "Check: 0x%x\n", byte );
        
        if ( byte == '1' )
        {
            data_switch = !data_switch;
            UART_write_string( debug, "Serial data changed state to %s\n", data_switch ? "online" : "offline" );
            UART_write_byte( uart, '0' );
        }
    }
}
