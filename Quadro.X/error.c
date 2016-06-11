#include "core.h"

#define SET_ERR_L   { ERR_LIGHT = 0; }

void error_process ( void )
{
    SET_ERR_L;
    UART_write_string( UARTm1, "Err\n" );
    while ( 1 );
}
