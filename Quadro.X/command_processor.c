#include "core.h"

uint16_t    buffer_index = 0;

uint8_t     *buffer = NULL,
            *replaceBuffer = NULL;

void cmdProcessor_init ( void )
{
    buffer = calloc( 1, CMD_PROC_BUFFER_LENGTH );
    replaceBuffer = calloc( 1, CMD_PROC_BUFFER_LENGTH );
    UART_set_receive_mode( UARTm1 | UARTm2, UARTr_interrupt, INT_PRIO_HIGHEST );
}

void replace_buffers ( int offset )
{
    memcpy( replaceBuffer, buffer + offset, CMD_PROC_BUFFER_LENGTH - offset );
    buffer_index -= offset;
    
    uint8_t *tmpPointer = replaceBuffer;
    replaceBuffer = buffer;
    buffer = tmpPointer;
}

UART_commands_e receive_command ()
{
    UART_commands_e command = NO_COMMAND;
    if ( buffer_index >= COMMAND_FRAME_SIZE )
    {
        if ( buffer[0] == CMD_PREFIX )
        {
            switch( buffer[1])
            {
                case CMD_CONNECT_CODE:
                    command = CONNECT;
                    break;
                case CMD_DATA_START_CODE:
                    command = DATA_START;
                    break;
                case CMD_DATA_STOP_CODE:
                    command = DATA_STOP;
                    break;
                default:
                    command = UNKNOWN_COMMAND;
            }
            replace_buffers( COMMAND_FRAME_SIZE );  
        }
        else
            replace_buffers( 1 );
    }
    return( command );
}

void cmdProcessor_write_cmd_resp ( UART_moduleNum_t module, uint8_t prefix, uint8_t code )
{
    uint16_t sendCommand = (uint16_t)prefix << 8 | code;
    UART_write_words( module, &sendCommand, 1 );
}

void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    _U1RXIF = 0;
}

void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
{
    while ( U2STAbits.URXDA )
    {
        buffer[buffer_index++] = U2RXREG;
        if ( buffer_index == CMD_PROC_BUFFER_LENGTH )
        {
            // Error overflow
            while( 1 );
        }
    }
        
    _U2RXIF = 0;
}
