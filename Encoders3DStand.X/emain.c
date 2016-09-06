/*
 * File:   emain.c
 * Author: alexey
 *
 * Created on June 4, 2016, 12:37 PM
 */

#include "core.h"
#include "pragmas.h"
#include "serial_protocol.h"

void command_translator_init ( UART_moduleNum_t module_from, UART_moduleNum_t module_to );
void command_translator ( void );
void data_translator ( int32_t enc_roll, int32_t enc_pitch );

#define ENCODERS_ENABLED

uint8_t     sendFlag = 0;
int32_t     encoderRoll = 0,
            encoderPitch = 0;

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    // UARTm1 - USB
    // UARTm2 - dsPIC on Quadro
    UART_init( UARTm1, UART_9600, INT_PRIO_HIGHEST );
    UART_init( UARTm2, UART_9600, INT_PRIO_HIGHEST );
    UART_write_string( UARTm1, "UART initialized\n" );
    
    command_translator_init( UARTm1, UARTm2 );
    UART_write_string( UARTm1, "Start!\n" );
#ifdef ENCODERS_ENABLED 
    _TRISD9 = _TRISD8 = _TRISD10 = _TRISD11 = 1;
    
    IC1CONbits.ICM = IC_CE_MODE_DISABLED;
    IC1CONbits.ICTMR = IC_TIMER_2;
    IC1CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC1CONbits.ICM = IC_CE_MODE_EDGE;
    _IC1IP = INT_PRIO_MID;
    _IC1IF = 0;     // Zero interrupt flag
    _IC1IE = 1;     // Enable interrupt

    IC3CONbits.ICM = IC_CE_MODE_DISABLED;
    IC3CONbits.ICTMR = IC_TIMER_2;
    IC3CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC3CONbits.ICM = IC_CE_MODE_EDGE;
    _IC3IP = INT_PRIO_MID;
    _IC3IF = 0;     // Zero interrupt flag
    _IC3IE = 1;     // Enable interrupt

#endif

    while ( 1 ) 
    {
        command_translator();
        data_translator( encoderRoll, encoderPitch );

//        UART_write_string( UARTm1, "%ld %ld\n", encoderRoll, encoderPitch );
//        delay_ms( 100 );
    }
    
    return ( 0 );
}
#ifdef ENCODERS_ENABLED 
void __attribute__( (__interrupt__, no_auto_psv) ) _IC1Interrupt()
{
    if ( _RD8 )
        encoderRoll += (_RD9<<1)-1;
    else
        encoderRoll += 1-(_RD9<<1);
    
    uint16_t trash = IC1BUF;
    _IC1IF = 0;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _IC3Interrupt()
{
    if ( _RD10 )
        encoderPitch += (_RD11<<1)-1;
    else
        encoderPitch += 1-(_RD11<<1);
    
    uint16_t trash = IC3BUF;
    _IC3IF = 0;
}
#endif
