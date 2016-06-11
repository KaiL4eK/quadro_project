#include "core.h"

uint8_t     u1_buffer_index = 0,
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
    if ( offset == 0 )
        return;
    
    if ( module & UARTm1 )
    {
        _U1RXIE = 0;
        memcpy( replaceBuffer, u1_buffer + offset, CMD_PROC_BUFFER_LENGTH - offset );
        u1_buffer_index -= offset;

        uint8_t *tmpPointer = replaceBuffer;
        replaceBuffer = u1_buffer;
        u1_buffer = tmpPointer;
        _U1RXIE = 1;
    }
    
    if ( module & UARTm2 )
    {
        _U2RXIE = 0;
        memcpy( replaceBuffer, u2_buffer + offset, CMD_PROC_BUFFER_LENGTH - offset );
        u2_buffer_index -= offset;

        uint8_t *tmpPointer = replaceBuffer;
        replaceBuffer = u2_buffer;
        u2_buffer = tmpPointer;
        _U2RXIE = 1;
    }

//    UART_write_string( UARTm1, "\t>>>%d\n", u2_buffer_index );
}

uint8_t cmdBShift = 0;

UART_commands_e cmdProcessor_U1_rcvCommand ( void )
{
    UART_commands_e command = NO_COMMAND;
    
    if ( u1_buffer_index > cmdBShift )
    {
        if ( u1_buffer[cmdBShift] == COMMAND_PREFIX )
        {
            if ( u1_buffer_index >= COMMAND_FRAME_SIZE + cmdBShift )
            {
                switch( u1_buffer[cmdBShift + 1])
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
                cmdBShift += COMMAND_FRAME_SIZE;
            }
        }
        else
        {
            cmdBShift++;
        }
    }
    else
    {
        replace_buffers( UARTm1, cmdBShift );
        cmdBShift = 0;
    }
    
    return( command );
}

uint8_t dataBShift = 0;

int cmdProcessor_U2_rcvData ( uint16_t *rcvBuffer )
{
//    UART_write_string( UARTm1, "\t%d %d\n", dataBShift, u2_buffer_index );
    if ( u2_buffer_index > dataBShift )
    {
        if ( u2_buffer[dataBShift] == DATA_PREFIX )
        {
            if ( u2_buffer_index >= DATA_FRAME_SIZE + dataBShift )
            {
                rcvBuffer[0] = (uint16_t)u2_buffer[1] << 8 | u2_buffer[2];
                rcvBuffer[1] = (uint16_t)u2_buffer[3] << 8 | u2_buffer[4];
                rcvBuffer[2] = (uint16_t)u2_buffer[5] << 8 | u2_buffer[6];
                dataBShift += DATA_FRAME_SIZE;
//                UART_write_string( UARTm1, "\t%d\n", dataBShift );
                return( 0 );
            }
        }
        else if ( u2_buffer[dataBShift] == RESPONSE_PREFIX )
        {
            if ( u2_buffer_index >= RESPONSE_FRAME_SIZE + dataBShift )
            {
                uint8_t respCode = u2_buffer[dataBShift + 1];
                dataBShift += RESPONSE_FRAME_SIZE;
                replace_buffers( UARTm2, dataBShift );
                dataBShift = 0;
                if ( respCode == RESP_ENDDATA )
                    return( 1 );
                else
                    return( -1 );
            }
        }
        else
        {
            dataBShift++;
        }
    }
    else
    {
        replace_buffers( UARTm2, dataBShift );
        dataBShift = 0;
    }
    return( -1 );
}

void cmdProcessor_cleanBuffer ( UART_moduleNum_t module )
{
    if ( module & UARTm1 )
    {
        cmdBShift = 0;
        replace_buffers( UARTm1, u1_buffer_index );
    }
    
    if ( module & UARTm2 )
    {
        dataBShift = 0;
        replace_buffers( UARTm2, u2_buffer_index );
    }
}

bool wd_timer = false;

void wdTimer_start ( void )
{    
    T2CONbits.TON = 0;
    T2CONbits.TCKPS = TIMER_DIV_256;
    _T2IP = INT_PRIO_LOW;
    _T2IE = 1;
    _T2IF = 0;
    PR2 = UINT16_MAX;
    
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
    wdTimer_start();
    
    int     result = -1;
    uint8_t respBShift = 0;
    
    while ( !wd_timer )
    {
        if ( u2_buffer_index > respBShift )
        {
            if ( u2_buffer[respBShift] == RESPONSE_PREFIX )
            {
                if ( u2_buffer_index >= RESPONSE_FRAME_SIZE + respBShift )
                {
                    result = u2_buffer[respBShift + 1] == RESP_NOERROR ? 0 : -1;
                    respBShift += RESPONSE_FRAME_SIZE;
//                    UART_write_string( UARTm1, ">>%d\n", respBShift );
                    break;
                }
            }
            else
            {
                respBShift++;
            }
        }
        Nop();
    }
    
    wdTimer_stop();
    replace_buffers( UARTm2, respBShift );
    
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
    {
        u1_buffer[u1_buffer_index] = U1RXREG;
        u1_buffer_index++;
//        UART_write_string( UARTm1, "\t1>%d %c\n", 
//                u1_buffer_index, u1_buffer[u1_buffer_index-1] );
        if ( u1_buffer_index == CMD_PROC_BUFFER_LENGTH )
        {
            UART_write_string( UARTm1, "Of1\n" );
            cmdProcessor_cleanBuffer( UARTm1 );
        }
    }
    
    _U1RXIF = 0;
}

void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
{
    while ( U2STAbits.URXDA )
    {
        u2_buffer[u2_buffer_index] = U2RXREG;
        u2_buffer_index++;
//        UART_write_string( UARTm1, "\t2>%d %c\n", 
//                u2_buffer_index, u2_buffer[u2_buffer_index-1] );
        if ( u2_buffer_index == CMD_PROC_BUFFER_LENGTH )
        {
            cmdProcessor_write_cmd_resp( UARTm2, 
                        COMMAND_PREFIX, CMD_DATA_STOP_CODE );
            cmdProcessor_cleanBuffer( UARTm2 );
            UART_write_string( UARTm1, "Of2\n" );
//            while( 1 ); // Error overflow
        }
    }

    _U2RXIF = 0;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _T2Interrupt()
{
    wd_timer = true;
    wdTimer_stop();
    _T2IF = 0;
}
