#include "core.h"

#define CMD_BUFFER_LENGTH 64

const   uint8_t cmdConnect = ~(1 << 1);

int     u1_buffer_index = 0,
        u2_buffer_index = 0;

uint8_t     *u1_buffer = NULL,
            *u2_buffer = NULL,
            *replaceBuffer = NULL;

void cmdProcessor_init ( void )
{
    u1_buffer = calloc( 1, CMD_BUFFER_LENGTH );
    u2_buffer = calloc( 1, CMD_BUFFER_LENGTH );
    replaceBuffer = calloc( 1, CMD_BUFFER_LENGTH );
}

void replace_buffers ( UART_moduleNum_t module, int offset )
{
    if ( module & UARTm1 )
    {
        memcpy( replaceBuffer, u1_buffer + offset, CMD_BUFFER_LENGTH - offset );
        u1_buffer_index -= offset;

        uint8_t *tmpPointer = replaceBuffer;
        replaceBuffer = u1_buffer;
        u1_buffer = tmpPointer;
    }
    
    if ( module & UARTm2 )
    {
        memcpy( replaceBuffer, u2_buffer + offset, CMD_BUFFER_LENGTH - offset );
        u2_buffer_index -= offset;

        uint8_t *tmpPointer = replaceBuffer;
        replaceBuffer = u2_buffer;
        u2_buffer = tmpPointer;
    }
}

#define COMMAND_FRAME_SIZE 2    // bytes
#define DATA_FRAME_SIZE 7

UART_commands_e cmdProcessor_U1_rcvCommand ( void )
{
    UART_commands_e command = NO_COMMAND;
    if ( u1_buffer_index >= COMMAND_FRAME_SIZE )
    {
        switch ( u1_buffer[0] )
        {
            case CMD_PREFIX:   // Command
                switch( u1_buffer[1])
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
                replace_buffers( UARTm1, COMMAND_FRAME_SIZE );
                break;
            case DATA_PREFIX:   // Data - don`t touch
                break;
            default:
                int offset = 0;
                replace_buffers( UARTm1, 1 );
        }
    }
    return( command );
}

int cmdProcessor_U2_rcvData ( uint16_t *rcvBuffer /*Must be size of 3 bytes*/ )
{
    if ( u2_buffer_index < DATA_FRAME_SIZE || u2_buffer[0] != '$' )
        return( -1 );
    
    rcvBuffer[0] = (uint16_t)u2_buffer[1] << 8 | u2_buffer[2];
    rcvBuffer[1] = (uint16_t)u2_buffer[3] << 8 | u2_buffer[4];
    rcvBuffer[2] = (uint16_t)u2_buffer[5] << 8 | u2_buffer[6];
    replace_buffers( UARTm2, DATA_FRAME_SIZE );
    
    return( 0 );
}

void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    while ( U1STAbits.URXDA )
    {
        u1_buffer[u1_buffer_index++] = U1RXREG;
        if ( u1_buffer_index == CMD_BUFFER_LENGTH )
        {
            // Error overflow
            while( 1 );
        }
    }
    
    _U1RXIF = 0;
}

void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
{
    while ( U2STAbits.URXDA )
    {
        u2_buffer[u2_buffer_index++] = U2RXREG;
        if ( u2_buffer_index == CMD_BUFFER_LENGTH )
        {
            // Error overflow
            while( 1 );
        }
    }
    _U2RXIF = 0;
}
