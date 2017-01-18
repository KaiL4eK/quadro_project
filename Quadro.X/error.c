#include "core.h"

#define SET_ERR_L   { ERR_LIGHT = 0; }

UART_moduleNum_t uart_debug = UARTmUndef;

void error_process_init ( UART_moduleNum_t uart )
{
    uart_debug = uart;
}

void error_process ( const char *err_str )
{
    SET_ERR_L;
    while ( 1 ) { 
        UART_write_string( uart_debug, "Error: %s\n", err_str ); 
        delay_ms( 500 ); 
    };
}
