#include "per_proto.h"

/********************************/
/*              UART            */
/********************************/

#define UART_DATA_BUFFER_SIZE 256

typedef struct uart_buffer_
{
    uint8_t data_buffer[UART_DATA_BUFFER_SIZE];
    uint8_t i_head_byte;
    uint8_t i_tail_byte;
    uint8_t n_bytes;
    bool    overflow;
    
}UART_buffer_t;

typedef struct
{
    bool                initialized;
    
}UART_module_fd;

UART_module_fd  uart1 = { .initialized = false },
                uart2 = { .initialized = false };

volatile UART_buffer_t  ub1 = { .i_head_byte = 0, .i_tail_byte = 0, .n_bytes = 0, .overflow = false },
                        ub2 = { .i_head_byte = 0, .i_tail_byte = 0, .n_bytes = 0, .overflow = false };


static inline bool UART_low_speed( UART_speed_t baud )
{
    return( baud == UART_38400 || baud == UART_19200 || baud == UART_9600 );
}

void UART_init( UART_moduleNum_t module, UART_speed_t baud, Interrupt_priority_lvl_t priority )
{
    if ( module == UARTm1 )
    {
        U1MODEbits.UARTEN   = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
        U1MODEbits.UEN      = 0;		// Bits8,9 TX,RX enabled, CTS,RTS not
        
        if ( UART_low_speed( baud ) )
            U1MODEbits.BRGH = 0;
        else
            U1MODEbits.BRGH = 1;
        U1BRG               = baud;
        
        _U1RXIE             = 1;          // Enable Rx interrupt
        _U1RXIP             = priority;
        
        U1MODEbits.UARTEN   = 1;	// And turn the peripheral on
        U1STAbits.UTXEN     = 1;

        uart1.initialized   = true;
        
    } else if ( module == UARTm2 )
    {
        U2MODEbits.UARTEN   = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
        U2MODEbits.UEN      = 0;		// Bits8,9 TX,RX enabled, CTS,RTS not
        
        if ( UART_low_speed( baud ) )
            U2MODEbits.BRGH = 0;
        else
            U2MODEbits.BRGH = 1;
        U2BRG               = baud;
        
        _U2RXIE             = 1;          // Enable Rx interrupt
        _U2RXIP             = priority;
        
        U2MODEbits.UARTEN   = 1;	// And turn the peripheral on
        U2STAbits.UTXEN     = 1;

        uart2.initialized   = true;
    }
}

/**
 * Polling API
 */

/*
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
*/

/**
 * Interrupt API
 */

void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    if ( U1STAbits.URXDA ) 
    {
        ub1.data_buffer[ub1.i_head_byte++] = U1RXREG;
        if ( ++ub1.n_bytes == UART_DATA_BUFFER_SIZE ) {
            ub1.overflow = true;
        }
    } else
        _U1RXIF = 0;
}

void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
{
    if ( U2STAbits.URXDA ) 
    {
        ub2.data_buffer[ub2.i_head_byte++] = U2RXREG;
        if ( ++ub2.n_bytes == UART_DATA_BUFFER_SIZE ) {
            ub2.overflow = true;
        }
    } else 
        _U2RXIF = 0;
}

uint8_t UART_get_byte( UART_moduleNum_t module )   
{
    volatile UART_buffer_t *data;
    
    if ( module == UARTm1 )
        data = &ub1;
    else if ( module == UARTm2 )
        data = &ub2;
    else
        return( 0 );
    
    if ( data->n_bytes == 0 )
        return( 0 );
    
    data->n_bytes--;
    return(  data->data_buffer[data->i_tail_byte++] );
}

void UART_get_bytes( UART_moduleNum_t module, uint8_t *out_buffer, uint8_t n_bytes )
{
    volatile UART_buffer_t *data;
    
    if ( module == UARTm1 )
        data = &ub1;
    else if ( module == UARTm2 )
        data = &ub2;
    else
        return;
    
    int16_t i = 0;
    for ( i = 0; i < n_bytes; i++ ) {
        if ( data->n_bytes == 0 )
            return;

        data->n_bytes--;
        out_buffer[i] = data->data_buffer[data->i_tail_byte++];
    }
}

uint8_t UART_bytes_available( UART_moduleNum_t module )
{
    if ( module == UARTm1 )
        return( ub1.n_bytes );
    else if ( module == UARTm2 )
        return( ub2.n_bytes );
    else
        return( 0 );
}

/**
 * Writing API
 */

void UART_write_byte( UART_moduleNum_t module, uint8_t elem )
{
    if ( module == UARTm1 ) {
        while( U1STAbits.UTXBF ) Nop();
        U1TXREG = elem;
    } else if ( module == UARTm2 ) {
        while( U2STAbits.UTXBF ) Nop();
        U2TXREG = elem;
    }
}

#define HIGH_16( x ) (((x) >> 8) & 0xff)
#define LOW_16( x )  ((x) & 0xff)

void UART_write_words( UART_moduleNum_t module, uint16_t *arr, uint8_t count )
{
    uint16_t iter = 0;
    for ( iter = 0; iter < count; iter++ ) {
        UART_write_byte( module, HIGH_16( arr[iter] ) );
        UART_write_byte( module, LOW_16( arr[iter] ) );
    }
}

#define BUFFER_MAX_SIZE 256
static char send_buffer[BUFFER_MAX_SIZE];

void UART_write_string( UART_moduleNum_t module, const char *fstring, ... )
{
    int iter = 0;
    va_list str_args;
    
    va_start( str_args, fstring );
    vsprintf( send_buffer, fstring, str_args );
    va_end( str_args );
    
    while( send_buffer[iter] != '\0' )
        UART_write_byte( module, send_buffer[iter++] );
}
