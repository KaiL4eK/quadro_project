#include "per_proto.h"

/********************************/
/*              UART            */
/********************************/

typedef struct
{
    UART_receiveMode_t  receive_mode;
    uint8_t             initialized;
    
}UART_module_FD;

UART_module_FD  uart1 = { .initialized = 0, .receive_mode = 0 },
                uart2 = { .initialized = 0, .receive_mode = 0 };

#define BUFFER_MAX_SIZE 512
static char send_buffer[BUFFER_MAX_SIZE];

void UART_init( UART_moduleNum_t module, UART_speed_t UART_br, bool highSpeed )
{
    if ( module & UARTm1 )
    {
        U1MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
        U1MODEbits.UEN = 0;		// Bits8,9 TX,RX enabled, CTS,RTS not
        if ( highSpeed )
        {
            U1BRG = UART_br;
            U1MODEbits.BRGH = 1;
        }
        else
        {
            U1BRG = UART_br/4;
            U1MODEbits.BRGH = 0;
        }
        U1MODEbits.UARTEN = 1;	// And turn the peripheral on
        U1STAbits.UTXEN = 1;

        uart1.initialized = 1;
    }
    
    if ( module & UARTm2 )
    {
        U2MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
        U2MODEbits.UEN = 0;		// Bits8,9 TX,RX enabled, CTS,RTS not
        if ( highSpeed )
        {
            U2BRG = UART_br;
            U2MODEbits.BRGH = 1;
        }
        else
        {
            U2BRG = UART_br/4;
            U2MODEbits.BRGH = 0;
        }
        U2MODEbits.UARTEN = 1;	// And turn the peripheral on
        U2STAbits.UTXEN = 1;

        uart2.initialized = 1;
    }
}

void UART_set_receive_mode ( UART_moduleNum_t module, UART_receiveMode_t mode, Interrupt_priority_lvl_t priority )
{
    if ( module & UARTm1 )
    {
        uart1.receive_mode = mode;
        switch ( mode )
        {
            case UARTr_interrupt:
                _U1RXIE = 1;          // Enable Rx interrupt
                _U1RXIP = priority;
                break;
            case UARTr_polling:
                _U1RXIE = 0;
                break;
        }
        _U1RXIF = 0;
    }
    
    if ( module & UARTm2 )
    {
        uart2.receive_mode = mode;
        switch ( mode )
        {
            case UARTr_interrupt:
                _U2RXIE = 1;          // Enable Rx interrupt
                _U2RXIP = priority;
                break;
            case UARTr_polling:
                _U2RXIE = 0;
                break;
        }
        _U2RXIF = 0;
    }
}

/**
 * Polling API
 */

int UART_receive_byte( UART_moduleNum_t module, uint8_t *received_byte1, uint8_t *received_byte2 )
{
    int result = -1;
    if ( (module & UARTm1) && 
            uart1.receive_mode == UARTr_polling && 
            received_byte1 != NULL && 
            U1STAbits.URXDA )
    {
        *received_byte1 = U1RXREG;
        result = 0;
    }

    if ( (module & UARTm2) && 
            uart2.receive_mode == UARTr_polling && 
            received_byte2 != NULL && 
            U2STAbits.URXDA )
    {
        *received_byte2 = U2RXREG;
        result = 0;
    }
    return( result );
}

/**
 * Interrupt API
 */

//uint8_t input_byte1 = 0,
//        input_byte2 = 0;
//
//void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
//{
//    input_byte1 = U1RXREG;
//    _U1RXIF = 0;
//}
//
//void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
//{
//    input_byte2 = U2RXREG;
//    _U2RXIF = 0;
//}
//
//int UART_get_last_received_byte( UART_moduleNum_t module, uint8_t *received_byte1, uint8_t *received_byte2 )   
//{
//    if ( (module & UARTm1) && 
//            uart1.receive_mode == UARTr_interrupt && 
//            received_byte1 != NULL )
//    {
//        *received_byte1 = input_byte1;
//        input_byte1 = 0;
//    }
//    
//    if ( (module & UARTm2) && 
//            uart2.receive_mode == UARTr_interrupt && 
//            received_byte2 != NULL )
//    {
//        *received_byte2 = input_byte2;
//        input_byte2 = 0;
//    }
//    return( 0 );
//}

/**
 * Writing API
 */

void UART_write_byte( UART_moduleNum_t module, uint8_t elem )
{
    if ( module & UARTm1 )
    {
        while( U1STAbits.UTXBF ) Nop();
        U1TXREG = elem;
    }
    
    if ( module & UARTm2 )
    {
        while( U2STAbits.UTXBF ) Nop();
        U2TXREG = elem;
    }
}

#define HIGH( x ) (((x) >> 8) & 0xff)
#define LOW( x ) ((x) & 0xff)

void UART_write_words( UART_moduleNum_t module, uint16_t *arr, uint8_t count )
{
    uint8_t iter = 0;
    for ( iter = 0; iter < count; iter++ )
    {
        UART_write_byte( module, HIGH( arr[iter] ) );
        UART_write_byte( module, LOW( arr[iter] ) );
    }
}

void UART_write_string( UART_moduleNum_t module, const char *fstring, ... )
{
    int iter = 0;
    va_list str_args;
    
    va_start( str_args, fstring );
    vsprintf( send_buffer, fstring, str_args );
    va_end( str_args );
    
    while( send_buffer[iter] != '\0' )
    {
        UART_write_byte( module, send_buffer[iter++] );
    }
}
