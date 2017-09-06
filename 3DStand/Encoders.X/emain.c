/*
 * File:   emain.c
 * Author: alexey
 *
 * Created on June 4, 2016, 12:37 PM
 */

#include "core.h"

#ifdef DSPIC_ENABLE_PLL
    #include "pragmas_pll.h"  
#else
    #include "pragmas.h"
#endif

volatile static uart_module_t               uart_debug      = NULL;
volatile static uart_module_t               uart_interface  = NULL;

void initialize_encoders( void );
void timer_interrupt_initialization( void );

#define ENCODERS_ENABLED

#define ENCODER_DATA_ARR_SIZE   4
volatile uint16_t       encodersData[ENCODER_DATA_ARR_SIZE]     = {0};
volatile int32_t        *encoderRollPtr     = (int32_t *)&encodersData[0],
                        *encoderPitchPtr    = (int32_t *)&encodersData[2];

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;

    setup_PLL_oscillator();
    
    uart_debug      = UART_init( 1, UART_BAUD_RATE_921600_HS, true, INT_PRIO_HIGH );    
//    uart_interface  = UART_init( 2, UART_BAUD_RATE_460800_HS, true, INT_PRIO_HIGH );
    UART_write_string( uart_debug, "UART initialized\n" );
    
//    command_translator_init( UARTm1, UARTm2 );
    UART_write_string( uart_debug, "Start!\n" );
    
    initialize_encoders();
    timer_interrupt_initialization();
    
    UART_write_string( uart_debug, "Initialization completed!\n" );

    while ( 1 ) 
    {        
        // External input
        if ( UART_bytes_available( uart_debug ) )
        {
            uint8_t input_char = UART_get_byte( uart_debug );
            if ( input_char == 'r' )
            {
                *encoderPitchPtr = *encoderRollPtr = 0;
            }
        }
    }
    
    return ( 0 );
}

const float INTERRUPT_PERIOD_S = 1000.0/1000000;

// Generates interrupt each 1 msec
void timer_interrupt_initialization( void )
{
    uint32_t timer_counter_limit = FCY * INTERRUPT_PERIOD_S;
    
    T4CONbits.TON   = 0;
    T4CONbits.T32   = 1;
    T4CONbits.TCKPS = TIMER_DIV_1;
    _T5IP           = INT_PRIO_HIGHEST;
    _T5IE           = 1;
    PR5             = ((timer_counter_limit >> 16) & 0xffff);
    PR4             = (timer_counter_limit & 0xffff);
    T4CONbits.TON   = 1;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _T5Interrupt()
{  
//    UART_write_string( uart_debug, "%ld %ld\n", *encoderRollPtr, *encoderPitchPtr  );
    UART_write_words( uart_debug, (uint16_t *)encodersData, ENCODER_DATA_ARR_SIZE );
    
    _T5IF = 0;
}

void initialize_encoders( void )
{
#ifdef ENCODERS_ENABLED 
    
    _TRISD9 = _TRISD8 = _TRISD10 = _TRISD11 = 1;

    IC1CONbits.ICM = IC_CE_MODE_DISABLED;
    IC1CONbits.ICTMR = IC_TIMER_2;
    IC1CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC1CONbits.ICM = IC_CE_MODE_EDGE;
    _IC1IP = INT_PRIO_HIGHEST;
    _IC1IF = 0;     // Zero interrupt flag
    _IC1IE = 1;     // Enable interrupt

    IC3CONbits.ICM = IC_CE_MODE_DISABLED;
    IC3CONbits.ICTMR = IC_TIMER_2;
    IC3CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC3CONbits.ICM = IC_CE_MODE_EDGE;
    _IC3IP = INT_PRIO_HIGHEST;
    _IC3IF = 0;     // Zero interrupt flag
    _IC3IE = 1;     // Enable interrupt

#endif  
}

#ifdef ENCODERS_ENABLED 
void __attribute__( (__interrupt__, no_auto_psv) ) _IC1Interrupt()
{
    if ( _RD8 )
        *encoderRollPtr += (_RD9<<1)-1;
    else
        *encoderRollPtr += 1-(_RD9<<1);
    
    _IC1IF = 0;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _IC3Interrupt()
{
    if ( _RD10 )
        *encoderPitchPtr += (_RD11<<1)-1;
    else
        *encoderPitchPtr += 1-(_RD11<<1);
    
    _IC3IF = 0;
}
#endif
