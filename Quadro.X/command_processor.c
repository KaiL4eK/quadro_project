#include "core.h"

#define CMD_BUFFER_LENGTH 128

const   uint8_t cmdConnect = ~(1 << 1);

int     cmdBInt_index = 0;

uint8_t     *commandBuffer = NULL,
            *replaceBuffer = NULL;

void cmdProcessor_init ( void )
{
    commandBuffer = calloc( 1, CMD_BUFFER_LENGTH );
    replaceBuffer = calloc( 1, CMD_BUFFER_LENGTH );
}

void replace_buffers ( int offset )
{
    memcpy( replaceBuffer, commandBuffer + offset, CMD_BUFFER_LENGTH - offset );
    cmdBInt_index -= offset;
    
    uint8_t *tmpPointer = replaceBuffer;
    replaceBuffer = commandBuffer;
    commandBuffer = tmpPointer;
}

UART_commands_e receive_command ( uint8_t *receivedByte )
{
    UART_commands_e command = NO_COMMAND;
    if ( cmdBInt_index >= 2 )
    {
        switch ( commandBuffer[0] )
        {
            case '*':   // Command
                if ( (uint8_t)commandBuffer[1] == 127 )
                    command = CONNECT;
                else
                    command = UNKNOWN_COMMAND; 
                
                replace_buffers( 2 );
                break;
            default:
                replace_buffers( 1 );
        }
    }
    return( command );
}

void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    _U1RXIF = 0;
}

void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
{
    while ( U2STAbits.URXDA )
    {
        commandBuffer[cmdBInt_index++] = U2RXREG;
//        UART_write_string( UARTm1, "%c %d\n", commandBuffer[cmdBInt_index-1], cmdBInt_index );
        if ( cmdBInt_index == CMD_BUFFER_LENGTH )
        {
            // Error overflow
            while( 1 );
        }
    }
        
    _U2RXIF = 0;
}
