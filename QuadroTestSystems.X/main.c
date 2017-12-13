/*
 * File:   main.c
 * Author: alex
 *
 * Created on December 6, 2017, 3:38 PM
 */

#include <pragmas.h>

#include <per_proto.h>
#include <motor_control.h>
#include <remote_control.h>
#include <stdint.h>


#define UART_DEBUG_MODULE   2
static uart_module_t    uart_debug;

const char *menu_string =   "Hello! It`s testing programm for F450 frame quadro\n"
                            "For testing module choose one of next points:\n"
                            "    1 - test motor rotation direction\n"
                            "    2 - test raw RC output\n";

const char *cancel_msg  = "Press 'c' to cancel\n";

#define TEST_UNDEFINED          -1
#define TEST_MOTOR_DIRECTION    0
#define TEST_RC_RAW_OUTPUT      1

RC_input_values_t   *rc_input;
RC_input_raw_t      *rc_input_raw;

int get_user_input ( uart_module_t uart_intrf )
{
    char input = 0;
    
    while ( 1 )
    {
        if ( UART_bytes_available( uart_intrf ) )
        {
            input = UART_get_byte( uart_intrf );
        
            switch ( input )
            {
                case '1':
                    return TEST_MOTOR_DIRECTION;
                 
                case '2':
                    return TEST_RC_RAW_OUTPUT;

                default:
                    return TEST_UNDEFINED;
            }
        }
    }
}

bool get_user_cancel_delay ( uart_module_t uart_intrf, uint32_t delay_ms )
{
    char input = 0;
    
    timer_set_timeout( delay_ms );
    
    while ( 1 )
    {
        if ( UART_bytes_available( uart_intrf ) )
        {
            input = UART_get_byte( uart_intrf );
            if ( input == 'c' )
                return true;
        }
        
        if ( timer_is_timeout() )
            break;
    }
    
    return false;
}

/******** TESTS ********/

#define POWER_SWITCH_DELAY_MS 1000

void test_motor_direction ( uart_module_t uart_intrf )
{
    int         i_power;
    int         i_motor;
    uint16_t    power_list[] = { INPUT_POWER_MIN,
                                 INPUT_POWER_MAX * 0.25, 
                                 INPUT_POWER_MAX * 0.5,
                                 INPUT_POWER_MAX * 0.75, 
                                 INPUT_POWER_MAX };
    motor_num_t motor_list[] = { MOTOR_1,
                                 MOTOR_2,
                                 MOTOR_3,
                                 MOTOR_4 };
    
    while ( 1 )
    {
        for ( i_motor = 0; i_motor < sizeof( motor_list )/sizeof( motor_list[0] ); i_motor++ )
        {
            motor_control_set_motor_started( motor_list[i_motor] );
            for ( i_power = 0; i_power < sizeof( power_list )/sizeof( power_list[0] ); i_power++ )
            {
                motor_control_set_motor_power( motor_list[i_motor], power_list[i_power] );
                if ( get_user_cancel_delay( uart_intrf, POWER_SWITCH_DELAY_MS ) )
                {
                    motor_control_set_motors_stopped();
                    return;
                }
            }
            motor_control_set_motors_stopped();
        }
    }
}

#define RC_RAW_OUTPUT_TEST_DELAY_MS    100

void test_rc_raw_output ( uart_module_t uart_intrf )
{
    if ( !remote_control_find_controller() )
    {
        UART_write_string( uart_debug, "Error: RC not connected\n" );
        return;
    }
    
    char input = 0;
    
    while ( 1 )
    {
        if ( UART_bytes_available( uart_intrf ) )
        {
            input = UART_get_byte( uart_intrf );
            if ( input == 'c' )
                return;
        }
        
        UART_write_string( uart_intrf, "3) %04ld, 4) %04ld, 1) %04ld, 2) %04ld, 5) %04ld\n", 
                rc_input_raw->channel_3, 
                rc_input_raw->channel_4, 
                rc_input_raw->channel_1, 
                rc_input_raw->channel_2, 
                rc_input_raw->channel_5 );
        
        delay_ms( RC_RAW_OUTPUT_TEST_DELAY_MS );
    }
}

int main ( void ) 
{
    uart_debug = UART_init( UART_DEBUG_MODULE, UART_BAUD_RATE_115200_LS, false, INT_PRIO_HIGH );
    UART_write_string( uart_debug, "/------------------------/\n" );
    UART_write_string( uart_debug, "UART initialized\n" );
    
    motor_control_init();
    UART_write_string( uart_debug, "Motors initialized\n" );
    motor_control_set_motors_stopped();
    
    rc_input        = remote_control_init();
    rc_input_raw    = remote_control_get_raw_prt();
    UART_write_string( uart_debug, "RC initialized\n" );
    
    
    while ( 1 )
    {
        UART_write_string( uart_debug, menu_string );

        switch ( get_user_input( uart_debug ) )
        {
            case TEST_MOTOR_DIRECTION:
                UART_write_string( uart_debug, "Motor direction test start\n" );
                UART_write_string( uart_debug, cancel_msg );
                test_motor_direction( uart_debug );
                break;
                
            case TEST_RC_RAW_OUTPUT:
                UART_write_string( uart_debug, "RC raw output test start\n" );
                UART_write_string( uart_debug, cancel_msg );
                test_rc_raw_output( uart_debug );
                break;
                
            default:
                UART_write_string( uart_debug, "Error: No such command\n" );
                break;
        }
        

    }
    
    return 0;
}
