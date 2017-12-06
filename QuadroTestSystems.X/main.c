/*
 * File:   main.c
 * Author: alex
 *
 * Created on December 6, 2017, 3:38 PM
 */

#include <pragmas.h>

#include <per_proto.h>
#include <motor_control.h>
#include <stdint.h>


#define UART_DEBUG_MODULE   1
static uart_module_t    uart_debug;


int main ( void ) 
{
    uart_debug = UART_init( UART_DEBUG_MODULE, UART_BAUD_RATE_115200_LS, false, INT_PRIO_HIGH );
    UART_write_string( uart_debug, "/------------------------/\n" );
    UART_write_string( uart_debug, "UART initialized\n" );
    
    motor_control_init();
    UART_write_string( uart_debug, "Motors initialized\n" );
    motor_control_set_motors_stopped();
    
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
    
#define POWER_SWITCH_DELAY_MS 1000
    
    while ( 1 )
    {
        for ( i_motor = 0; i_motor < sizeof( motor_list )/sizeof( motor_list[0] ); i_motor++ )
        {
            motor_control_set_motor_started( motor_list[i_motor] );
            for ( i_power = 0; i_power < sizeof( power_list )/sizeof( power_list[0] ); i_power++ )
            {
                motor_control_set_motor_power( motor_list[i_motor], power_list[i_power] );
                delay_ms( POWER_SWITCH_DELAY_MS );
            }
            motor_control_set_motors_stopped();
        }
    }
    
    return 0;
}
