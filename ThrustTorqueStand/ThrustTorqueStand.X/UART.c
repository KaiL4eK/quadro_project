#include "core.h"

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

volatile UART_buffer_t  ub1 = { .i_head_byte = 0, .i_tail_byte = 0, .n_bytes = 0, .overflow = false };

/********************************/
/*              UART            */
/********************************/

void init_UART1( UART_speed_t UART_br )
{
	U1MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
	U1MODEbits.UEN = 0;		// Bits8,9 TX,RX enabled, CTS,RTS not
	U1BRG = UART_br;
    U1MODEbits.BRGH = 1;
    _U1RXIE = 1;          // Enable Rx interrupt
    _U1RXIF = 0;
    _U1RXR = 37;    // U1RX to pin RPI37 - PTP15
    _RP14R = 3;     // U1TX to pin RP14 - PTP5
    
	U1MODEbits.UARTEN = 1;	// And turn the peripheral on
	U1STAbits.UTXEN = 1;
}

uint8_t input_command = 0;

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

uint8_t UART_bytes_available()
{
    return( ub1.n_bytes );
}

uint8_t UART_get_byte()   
{
    volatile UART_buffer_t *data = &ub1;
    
    if ( data->n_bytes == 0 )
        return( 0 );
    
    data->n_bytes--;
    return(  data->data_buffer[data->i_tail_byte++] );
}

void UART_write_byte( uint8_t byte )
{
    while( U1STAbits.UTXBF ) Nop();
    U1TXREG = byte;
}

#define BUFFER_MAX_SIZE 256
static char send_buffer[BUFFER_MAX_SIZE];

void UART_write_string( const char *fstring, ... )
{
    int iter = 0;
    va_list str_args;
    
    va_start( str_args, fstring );
    vsprintf( send_buffer, fstring, str_args );
    va_end( str_args );
    
    while( send_buffer[iter] != '\0' )
        UART_write_byte( send_buffer[iter++] );
}