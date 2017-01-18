#include <p33FJ256MC710.h>

#include "per_proto.h"

/********************************/
/*              UART            */
/********************************/

#define UART_DATA_BUFFER_SIZE 256

typedef struct
{
    bool                    initialized;
    
    UART_write_endian_t     write_endian_mode;
    uint8_t                 write_buffer[UART_DATA_BUFFER_SIZE];
    uint8_t                 i_write_head_byte;
    uint8_t                 i_write_tail_byte;
    uint8_t                 n_write_bytes_available;
    bool                    write_overflow;
    
    uint8_t                 read_buffer[UART_DATA_BUFFER_SIZE];
    uint8_t                 i_read_head_byte;
    uint8_t                 i_read_tail_byte;
    uint8_t                 n_read_bytes_available;
    bool                    read_overflow;
    
}UART_module_fd;

volatile UART_module_fd  uart_fd[] = {  {   .initialized = false,
                                            .write_endian_mode = UART_big_endian, .i_write_head_byte = 0, .i_write_tail_byte = 0, .n_write_bytes_available = 0, .write_overflow = false, 
                                                                                  .i_read_head_byte = 0,  .i_read_tail_byte = 0,  .n_read_bytes_available = 0,  .read_overflow = false },
                                        {   .initialized = false,
                                            .write_endian_mode = UART_big_endian, .i_write_head_byte = 0, .i_write_tail_byte = 0, .n_write_bytes_available = 0, .write_overflow = false, 
                                                                                  .i_read_head_byte = 0,  .i_read_tail_byte = 0,  .n_read_bytes_available = 0,  .read_overflow = false }    };

static inline bool UART_low_speed( UART_speed_t baud )
{
    return( baud == UART_38400 || baud == UART_19200 || baud == UART_9600 );
}

void UART_init( UART_moduleNum_t module, UART_speed_t baud, Interrupt_priority_lvl_t priority )
{
    if ( module == UARTmUndef )
        return;
    
    if ( module == UARTm1 )
    {
        U1MODEbits.UARTEN   = 0;            // Bit15 TX, RX DISABLED, ENABLE at end of func
        U1MODEbits.UEN      = 0;            // Bits8,9 TX,RX enabled, CTS,RTS not
        
        if ( UART_low_speed( baud ) )
            U1MODEbits.BRGH = 0;
        else
            U1MODEbits.BRGH = 1;
        U1BRG               = baud;
        
        _U1TXIF             = 0;
        _U1RXIF             = 0;
        
        _U1TXIE             = 1;            // Enable Tx interrupt
        _U1RXIE             = 1;            // Enable Rx interrupt
        
        _U1TXIP             = priority;
        _U1RXIP             = priority;
        
//        _TRISF3             = 0;
//        _LATF3              = 0;
        
        U1STAbits.UTXISEL0  = 0;
        U1STAbits.UTXISEL1  = 0;
        
        U1MODEbits.UARTEN   = 1;            // And turn the peripheral on
        U1STAbits.UTXEN     = 1; 
    } 
    else
    {
        U2MODEbits.UARTEN   = 0;
        U2MODEbits.UEN      = 0;
        
        if ( UART_low_speed( baud ) )
            U2MODEbits.BRGH = 0;
        else
            U2MODEbits.BRGH = 1;
        U2BRG               = baud;
        
        _U2TXIF             = 0;
        _U2RXIF             = 0;
        
        _U2TXIE             = 1;
        _U2RXIE             = 1;
        
        _U2TXIP             = priority;
        _U2RXIP             = priority;
        
//        _TRISF5             = 0;
        
        U2STAbits.UTXISEL0  = 0;
        U2STAbits.UTXISEL1  = 0;
        
        U2MODEbits.UARTEN   = 1;
        U2STAbits.UTXEN     = 1;
    }
    
    uart_fd[module].initialized   = true;
}

void UART_write_set_endian ( UART_moduleNum_t module, UART_write_endian_t mode )
{
    if ( module == UARTmUndef )
        return;
    
    uart_fd[module].write_endian_mode = mode;
}

/**
 * Reading API
 */

void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    if ( U1STAbits.URXDA ) 
    {
        volatile UART_module_fd  *p_fd = &uart_fd[UARTm1];
        
        p_fd->read_buffer[p_fd->i_read_head_byte++] = U1RXREG;
        
        if ( ++p_fd->n_read_bytes_available == UART_DATA_BUFFER_SIZE ) {
            p_fd->read_overflow = true;
            while ( p_fd->n_read_bytes_available == UART_DATA_BUFFER_SIZE ) {};  // Wait for buffer free
            p_fd->read_overflow = false;
        }
    } else
        _U1RXIF = 0;
}

void __attribute__( (__interrupt__, auto_psv) ) _U2RXInterrupt()
{
    if ( U2STAbits.URXDA ) 
    {
        volatile UART_module_fd  *p_fd = &uart_fd[UARTm2];
        
        p_fd->read_buffer[p_fd->i_read_head_byte++] = U2RXREG;
        
        if ( ++p_fd->n_read_bytes_available == UART_DATA_BUFFER_SIZE ) {
            p_fd->read_overflow = true;
            while ( p_fd->n_read_bytes_available == UART_DATA_BUFFER_SIZE ) {};  // Wait for buffer free
            p_fd->read_overflow = false;
        }
    } else 
        _U2RXIF = 0;
}

uint8_t UART_get_byte( UART_moduleNum_t module )   
{
    if ( module == UARTmUndef )
        return 0;
    
    volatile UART_module_fd *p_fd = &uart_fd[module];
    
    if ( p_fd->n_read_bytes_available == 0 )
        return( 0 );
    
    p_fd->n_read_bytes_available--;
    return p_fd->read_buffer[p_fd->i_read_tail_byte++];
}

void UART_get_bytes( UART_moduleNum_t module, uint8_t *out_buffer, uint8_t n_bytes )
{
    if ( module == UARTmUndef )
        return;
    
    volatile UART_module_fd *p_fd = &uart_fd[module];
    
    int16_t i = 0;
    for ( i = 0; i < n_bytes; i++ ) {
        if ( p_fd->n_read_bytes_available == 0 )
            return;

        p_fd->n_read_bytes_available--;
        out_buffer[i] = p_fd->read_buffer[p_fd->i_read_tail_byte++];
    }
}

uint8_t UART_bytes_available( UART_moduleNum_t module )
{
    if ( module == UARTmUndef )
        return 0;
    
    return uart_fd[module].n_read_bytes_available;
}

/**
 * Writing API
 */

void __attribute__( (__interrupt__, auto_psv) ) _U1TXInterrupt()
{
    volatile UART_module_fd  *p_fd = &uart_fd[UARTm1];
    
    if ( !U1STAbits.UTXBF )
    {
        if ( p_fd->n_write_bytes_available )
        {
            U1TXREG = p_fd->write_buffer[p_fd->i_write_tail_byte++];
            p_fd->n_write_bytes_available--;
        } else {
            _U1TXIF = 0;
        }
    }
};

void __attribute__( (__interrupt__, auto_psv) ) _U2TXInterrupt()
{
    volatile UART_module_fd  *p_fd = &uart_fd[UARTm2];
    
    if ( !U2STAbits.UTXBF )
    {
        if ( p_fd->n_write_bytes_available )
        {
            U2TXREG = p_fd->write_buffer[p_fd->i_write_tail_byte++];
            p_fd->n_write_bytes_available--;
        } else {
            _U2TXIF = 0;
        }
    }
};

void UART_write_byte( UART_moduleNum_t module, uint8_t elem )
{
    if ( module == UARTmUndef )
        return;
    
    volatile UART_module_fd *p_fd = &uart_fd[module];
    
    while ( p_fd->n_write_bytes_available == UART_DATA_BUFFER_SIZE ) { Nop(); }
    
    p_fd->write_buffer[p_fd->i_write_head_byte++] = elem;
    p_fd->n_write_bytes_available++;

    if ( module == UARTm1 )
        _U1TXIF = 1;
    else
        _U2TXIF = 1;  
}

#define HIGH_16( x ) (((x) >> 8) & 0xff)
#define LOW_16( x )  ((x) & 0xff)

void UART_write_words( UART_moduleNum_t module, uint16_t *arr, uint8_t count )
{
    if ( module == UARTmUndef )
        return;
    
    uint16_t iter = 0;
    for ( iter = 0; iter < count; iter++ ) {
        if ( uart_fd[module].write_endian_mode == UART_big_endian )
        {
            UART_write_byte( module, HIGH_16( arr[iter] ) );
            UART_write_byte( module, LOW_16( arr[iter] ) );
        } else {
            UART_write_byte( module, LOW_16( arr[iter] ) );
            UART_write_byte( module, HIGH_16( arr[iter] ) );
        }
    }
}

#define BUFFER_MAX_SIZE 256
static char send_buffer[BUFFER_MAX_SIZE];

void UART_write_string( UART_moduleNum_t module, const char *fstring, ... )
{
    if ( module == UARTmUndef )
        return;
    
    int iter = 0;
    va_list str_args;
    
    va_start( str_args, fstring );
    vsprintf( send_buffer, fstring, str_args );
    va_end( str_args );
    
    while( send_buffer[iter] != '\0' )
        UART_write_byte( module, send_buffer[iter++] );
}
