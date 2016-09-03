#include "core.h"

#define SET_ERR_L   { ERR_LIGHT = 0; }

void error_process ( const char *err_str )
{
    SET_ERR_L;
    while ( 1 ) { UART_write_string( UARTm1, "Err %s\n", err_str ); }
}
