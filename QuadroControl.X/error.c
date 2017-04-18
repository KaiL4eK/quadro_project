#include "core.h"

#define SET_ERR_L   { ERR_LIGHT = 0; }

uart_module_t uart_debug = NULL;

void error_process_init ( uart_module_t uart_module )
{
    uart_debug = uart_module;
}

void error_process ( const char *err_str )
{
    SET_ERR_L;
    while ( 1 ) { 
        UART_write_string( uart_debug, "Error: %s\n", err_str );
        delay_ms( 500 ); 
    };
}
