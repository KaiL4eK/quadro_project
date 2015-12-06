#include "UART.h"
#include "external.h"

/********************************/
/*              UART            */
/********************************/

#define BUFFER_MAX_SIZE 1024

void UART_write_byte( uint8_t elem );

char UART_buffer[BUFFER_MAX_SIZE];

void init_UART1( uint32_t UART_br )
{
	U1MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
	U1MODEbits.UEN = 0;		// Bits8,9 TX,RX enabled, CTS,RTS not
	U1BRG = (FCY/(UART_br*16))-1;
    _U1RXIE = 1;
    _U1RXIF = 0;
    
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
        UART_write_byte( LOW( arr[iter] ) );
        UART_write_byte( HIGH( arr[iter] ) );
    }
}

void UART_write_string( char *string )
{
    int iter = 0;
    while( string[iter] != '\0' )
    {
        UART_write_byte( string[iter++] );
    }
}

void UART_writeln_string( char *string )
{
    UART_write_string( string );
    UART_write_string( "\r\n" );
}

void UART_write_int32( int32_t num )
{
    sprintf(UART_buffer, "%ld", num);
    UART_writeln_string( UART_buffer );
}

void UART_write_int16( int16_t num )
{
    sprintf(UART_buffer, "%d", num);
    UART_writeln_string( UART_buffer );
}

void UART_write_int( int num )
{
    UART_write_int16( num );
}

void UART_write_float( double num )
{
    sprintf(UART_buffer, "%f", num);
    UART_writeln_string( UART_buffer );
}

void UART_write_hint( int num )
{
    sprintf(UART_buffer, "0x%x", num);
    UART_writeln_string( UART_buffer );
}

void UART_write_hint32( uint32_t num )
{
    sprintf(UART_buffer, "0x%08lx", num);
    UART_writeln_string( UART_buffer );
}
