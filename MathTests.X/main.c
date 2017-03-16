/*
 * File:   main.c
 * Author: alex
 *
 * Created on March 16, 2017, 2:52 PM
 */

#include <per_proto.h>
#include <pragmas.h>

#include <math_proto.h>

#define UART_INTERFACE 1
static uart_module_t       uart_interface  = NULL;

void process_inv_sqrt_benchmark ( void );

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
        
    uart_interface = UART_init( UART_INTERFACE, UART_460800, INT_PRIO_HIGHEST );
    UART_write_set_big_endian_mode( uart_interface, true );
    bool show_notif = false;
    
    while ( 1 )
    {
        if ( !show_notif )
        {
            UART_write_string( uart_interface, "\nBenchmark is ready:\n\tclick '1' to start <inv_sqrt> benchmark\n" );
            show_notif = true;
        }
        
        if ( UART_bytes_available( uart_interface ) )
        {
            uint8_t input = UART_get_byte( uart_interface );
            if ( input == '1' )
            {
                process_inv_sqrt_benchmark();

                UART_clean_input( uart_interface );
                show_notif = false;
            }
            else 
                UART_write_string( uart_interface, "\nBenchmark is ready:\n\tclick '1' to start <inv_sqrt> benchmark\n" );
        }
    }
    
    return 0;
}

void process_inv_sqrt_benchmark ( void )
{
#define COUNT_INV_SQRT_LIMIT    65000.0
#define COUNT_DELTA             1

    uint32_t time_elapsed_us = 0;
//    uint32_t prev_maxtime_us = 0;
    uint32_t max_time_inv_sqrt = 0;
    uint32_t max_time_opt_inv_sqrt = 0;
    
    float curr_error = 0;
    float max_error = 0;
    float prev_error = 0;
    float value = 0;
    
    for ( value = COUNT_DELTA; value < COUNT_INV_SQRT_LIMIT; value += COUNT_DELTA )
    {
        uint16_t int_val = value;
        bool show_data = false;
        if ( int_val % 1000 == 0 )
            show_data = true;
        
        timer_start();
        float inv_sqrt_val = 1.0/sqrt( value );
        timer_stop();
        time_elapsed_us = timer_get_us();
        max_time_inv_sqrt = max( max_time_inv_sqrt, time_elapsed_us );
        
        timer_start();
        float opt_inv_sqrt_val = inv_sqrt( value );
        timer_stop();
        time_elapsed_us = timer_get_us();
        max_time_opt_inv_sqrt = max( max_time_opt_inv_sqrt, time_elapsed_us );

        curr_error = inv_sqrt_val - opt_inv_sqrt_val;
        prev_error = max_error;
        max_error = max( max_error, curr_error );
        
        
        if ( show_data || prev_error != max_error )
            UART_write_string( uart_interface, "Data: %f -> %f / %f -> %f\n", value, inv_sqrt_val, opt_inv_sqrt_val, curr_error );
    }
        
    UART_write_string( uart_interface, "Max manual time = %ld us\n", max_time_inv_sqrt );
    UART_write_string( uart_interface, "Max optimized time = %ld us\n", max_time_opt_inv_sqrt );
    UART_write_string( uart_interface, "Max error = %f\n", max_error );
    
    
}
