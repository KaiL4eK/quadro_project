#include "core.h"
#include "per_proto.h"
#include <string.h>


#define CMD_LENGTH 3

char commandBuffer[CMD_LENGTH];
int cmdB_iter = 0;

UART_commands_e receive_command ( void )
{
    UART_commands_e command = NO_COMMAND;
    while ( UART_receive_byte( UARTm1, &commandBuffer[cmdB_iter], NULL ) == 0 )
    {
        if ( ++cmdB_iter == CMD_LENGTH )
        {
            cmdB_iter = 0;
            if ( !strcmp( "con", commandBuffer ) )
                command = CONNECT;
            else if ( !strcmp( "dst", commandBuffer ) )
                command = DATA_START;
            else if ( !strcmp( "psd", commandBuffer ) )
                command = DATA_STOP;
            break;
        }
    }
    return( command );
}
