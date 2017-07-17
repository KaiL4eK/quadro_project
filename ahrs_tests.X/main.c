/*
 * File:   main.c
 * Author: alexey
 *
 * Created on February 9, 2017, 11:40 PM
 */

#include <per_proto.h>

#ifdef DSPIC_ENABLE_PLL
    #include "pragmas_pll.h"  
#else
    #include "pragmas.h"
#endif

#include <MPU6050.h>

volatile static uart_module_t               uart_debug      = NULL;
volatile static uart_module_t               uart_interface  = NULL;
volatile static gy_521_gyro_accel_data_t    *mpu_data       = NULL;

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
//    delay_ms( 300 );
    
    setup_PLL_oscillator();
    
    uart_interface  = UART_init( 1, UART_BAUD_RATE_460800_HS, true, INT_PRIO_HIGH );
    UART_write_set_big_endian_mode( uart_interface, true ); 
    
    uart_debug      = UART_init( 2, UART_BAUD_RATE_460800_HS, true, INT_PRIO_HIGH );
    UART_write_string( uart_debug, "UART ready\n" );
    
    i2c_init( 1 ); 
    UART_write_string( uart_debug, "I2C ready\n" );
    
    if ( mpu6050_init( NULL, uart_debug ) != 0 )    
        error_loop( "MPU6050 initialize failed\n" );

    mpu6050_offsets_t mpu6050_offsets = { -1805, -604, 1429, 41, -16, -14 };
    
    mpu6050_set_bandwidth( MPU6050_DLPF_BW_42 );
    mpu6050_set_offsets( &mpu6050_offsets );
    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_500 );
    mpu6050_set_accel_fullscale( MPU6050_ACCEL_FS_2 );
//    mpu6050_set_sample_rate_divider( 1 );

    mpu_data = mpu6050_get_raw_data();
    UART_write_string( uart_debug, "MPU6050 ready\n" );    

//    mpu6050_calibration();
    
    _TRISF12 = 1;
    
    timer_interrupt_initialization();
    
    while ( 1 )
    {
    }
    
    return 0;
}

const float INTERRUPT_PERIOD_S = 100.0/1000;

// Generates interrupt each 1 msec
void timer_interrupt_initialization( void )
{
    uint32_t timer_counter_limit = FCY * INTERRUPT_PERIOD_S;
    
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
    
    UART_write_string( uart_debug, "AD: %06d %06d %06d\n", mpu_data->x_accel, mpu_data->y_accel, mpu_data->z_accel );
    UART_write_string( uart_debug, "GD: %06d %06d %06d\n", mpu_data->x_gyro, mpu_data->y_gyro, mpu_data->z_gyro );
    UART_write_string( uart_debug, "-------------------------------------------------------\n" );
//    uart_response( uart_debug );
//    send_serial_data( uart_interface, uart_debug );
    
    _T3IF = 0;
}

void uart_response ( uart_module_t uart )
{
    if ( UART_bytes_available( uart ) )
    {
        UART_write_string( uart, "Clicked: %c %d %d\n", UART_get_byte( uart ), _RF12, mpu_data->x_gyro );
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
        
        buffer[i++] = mpu_data->x_accel;
        buffer[i++] = mpu_data->y_accel;
        buffer[i++] = mpu_data->z_accel;
        buffer[i++] = mpu_data->x_gyro;
        buffer[i++] = mpu_data->y_gyro;
        buffer[i++] = mpu_data->z_gyro;   
        buffer[i++] = _RF12;   
        
        UART_write_words( uart, (uint16_t *)buffer, sizeof( buffer )/sizeof( *buffer ) );
    }
    
    if ( UART_bytes_available( uart ) )
    {
        byte = UART_get_byte( uart );
        UART_write_string( debug, "Check: 0x%x\n", byte );
        
        if ( byte == '1' )
        {
            data_switch = !data_switch;
            UART_write_string( debug, "Serial data changed state to %s\n", data_switch ? "online" : "offline" );
            
            if ( !data_switch )
                _LATF12 = 0;
            
            UART_write_byte( uart, '0' );
        }
    }
}
