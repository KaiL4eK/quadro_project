#include <stdint.h>

#include "core.h"

uint16_t    u1_buffer_index = 0,
            u2_buffer_index = 0;

uint8_t     *u1_buffer = NULL,
            *u2_buffer = NULL,
            *replaceBuffer = NULL;

void cmdProcessor_init ( void )
{
    u1_buffer = calloc( 1, CMD_PROC_BUFFER_LENGTH );
    u2_buffer = calloc( 1, CMD_PROC_BUFFER_LENGTH );
    replaceBuffer = calloc( 1, CMD_PROC_BUFFER_LENGTH );
    UART_set_receive_mode( UARTm1 | UARTm2, UARTr_interrupt, INT_PRIO_HIGHEST );
}

void replace_buffers ( UART_moduleNum_t module, int offset )
{
    _U2RXIE = 0;
    if ( module & UARTm1 )
    {
        memcpy( replaceBuffer, u1_buffer + offset, CMD_PROC_BUFFER_LENGTH - offset );
        u1_buffer_index -= offset;

        uint8_t *tmpPointer = replaceBuffer;
        replaceBuffer = u1_buffer;
        u1_buffer = tmpPointer;
    }
    
    if ( module & UARTm2 )
    {
        memcpy( replaceBuffer, u2_buffer + offset, CMD_PROC_BUFFER_LENGTH - offset );
        u2_buffer_index -= offset;

        uint8_t *tmpPointer = replaceBuffer;
        replaceBuffer = u2_buffer;
        u2_buffer = tmpPointer;
    }
    _U2RXIE = 1;
}

UART_commands_e cmdProcessor_U1_rcvCommand ( void )
{
    UART_commands_e command = NO_COMMAND;
    if ( u1_buffer_index >= COMMAND_FRAME_SIZE )
    {
        if ( u1_buffer[0] == CMD_PREFIX )
        {
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
        }
        else
            replace_buffers( UARTm1, 1 );
    }
    return( command );
}

int cmdProcessor_U2_rcvData ( uint16_t *rcvBuffer /*Must be size of 3 bytes*/ )
{
    if ( u2_buffer_index < DATA_FRAME_SIZE || 
         u2_buffer[0] != DATA_PREFIX )
        return( -1 );
    
    rcvBuffer[0] = (uint16_t)u2_buffer[1] << 8 | u2_buffer[2];
    rcvBuffer[1] = (uint16_t)u2_buffer[3] << 8 | u2_buffer[4];
    rcvBuffer[2] = (uint16_t)u2_buffer[5] << 8 | u2_buffer[6];
    replace_buffers( UARTm2, DATA_FRAME_SIZE );
    
    return( 0 );
}

void cmdProcessor_cleanBuffer ( UART_moduleNum_t module )
{
    if ( module & UARTm1 )
        replace_buffers( UARTm1, u1_buffer_index );
    
    if ( module & UARTm2 )
        replace_buffers( UARTm2, u2_buffer_index );
}

bool wd_timer = false;

void wdTimer_start ( void )
{
    TMR2 = 0;
    wd_timer = false;
    T2CONbits.TON = 1;
}

void wdTimer_stop ( void )
{
    T2CONbits.TON = 0;
}

int cmdProcessor_U2_waitResponse ( void )
{
    T2CONbits.TON = 0;
    T2CONbits.TCKPS = TIMER_DIV_256;
    _T2IP = 1;
    _T2IE = 1;
    _T2IF = 0;
    PR2 = UINT16_MAX;
    wdTimer_start();
    
    int result = -1;
    while ( u2_buffer_index < RESPONSE_FRAME_SIZE &&
            !wd_timer )
        Nop();
    
    if ( wd_timer )
        return( result );
    else
        wdTimer_stop();
        
    if ( u2_buffer[0] == RESPONSE_PREFIX && 
         u2_buffer[1] == RESP_NOERROR )
    {
        result = 0;
        replace_buffers( UARTm2, RESPONSE_FRAME_SIZE );
    }
    else
        cmdProcessor_cleanBuffer( UARTm2 );
    
    
    return( result );
}

void cmdProcessor_write_cmd_resp ( UART_moduleNum_t module, uint8_t prefix, uint8_t code )
{
    uint16_t sendCommand = (uint16_t)prefix << 8 | code;
    UART_write_words( module, &sendCommand, 1 );
}

void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    while ( U1STAbits.URXDA )
        u1_buffer[u1_buffer_index++] = U1RXREG;
    
    _U1RXIF = 0;
}

void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
{
    while ( U2STAbits.URXDA )
        u2_buffer[u2_buffer_index++] = U2RXREG;

    _U2RXIF = 0;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _T2Interrupt()
{
    wd_timer = true;
    wdTimer_stop();
    _T2IF = 0;
}
