#include "error_.h"
#include "per_proto.h"

#define SET_ERR_L   { ERR_LIGHT = 0; }

void error_process ( void )
{
    SET_ERR_L;
    UART_write_string( "Err\n" );
    while ( 1 );
}
