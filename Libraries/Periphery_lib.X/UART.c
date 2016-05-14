#include "per_proto.h"

/********************************/
/*              UART            */
/********************************/

#define BUFFER_MAX_SIZE 512

void UART_write_byte( uint8_t elem );

char UART_buffer[BUFFER_MAX_SIZE];

void init_UART1( UART_speed_t UART_br )
{
	U1MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
	U1MODEbits.UEN = 0;		// Bits8,9 TX,RX enabled, CTS,RTS not
	U1BRG = UART_br;
//    _U1RXIE = 1;          // Enable Rx interrupt
//    _U1RXIF = 0;
    
	U1MODEbits.UARTEN = 1;	// And turn the peripheral on
	U1STAbits.UTXEN = 1;
//    _U1TXIE = 1;          //Enable TX interrupt 
//    _U1TXIF = 0;
}

uint8_t input_command = 0;

void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    input_command = U1RXREG;
    _U1RXIF = 0;
}

int UART_receive_byte( uint8_t *received_byte )
{
    if ( U1STAbits.URXDA )
    {
        *received_byte = U1RXREG;
//        UART_write_byte( *received_byte );
        return( 0 );
    }
    return( -1 );
}

uint8_t UART_get_last_received_command()
{
    if ( input_command == 0 )
        return( 0 );
    uint8_t last_rcv = input_command;
    input_command = 0;
    return( last_rcv );
}

void UART_write_byte( uint8_t elem )
{
    while( U1STAbits.UTXBF ) Nop();
    U1TXREG = elem;
}

#define HIGH( x ) (((x) >> 8) & 0xff)
#define LOW( x ) ((x) & 0xff)

void UART_write_words( uint16_t *arr, uint8_t count )
{
    uint8_t iter = 0;
    for ( iter = 0; iter < count; iter++ )
    {
        UART_write_byte( HIGH( arr[iter] ) );
        UART_write_byte( LOW( arr[iter] ) );
    }
}

void UART_write_string( const char *fstring, ... )
{
    int iter = 0;
    va_list str_args;
    
    va_start( str_args, fstring );
    vsprintf( UART_buffer, fstring, str_args );
    va_end( str_args );
    
    while( UART_buffer[iter] != '\0' )
    {
        UART_write_byte( UART_buffer[iter++] );
    }
}
