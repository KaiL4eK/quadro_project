/*
 * File:   emain.c
 * Author: alexey
 *
 * Created on June 4, 2016, 12:37 PM
 */

#include <xc.h>
#include "per_proto.h"
#include "pragmas.h"

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    
    UART_init( UARTm2, UART_460800 );
    UART_write_string( UARTm2, "UART initialized\n" );
    
    while ( 1 )
    {
        UART_write_string( UARTm2, "Res: %d\n", _RA15 );
        delay_ms( 300 );
    }
    
    return 0;
}
