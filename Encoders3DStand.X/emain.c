/*
 * File:   emain.c
 * Author: alexey
 *
 * Created on June 4, 2016, 12:37 PM
 */

#include <xc.h>
#include "per_proto.h"
#include "pragmas.h"

uint8_t     sendFlag = 0;
uint32_t    encoderRoll = 0,
            encoderPitch = 0;

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    
    UART_init( UARTm1, UART_460800 );
    UART_write_string( UARTm1, "UART initialized\n" );
    
    _TRISD9 = _TRISD8 = 1;
    
    IC1CONbits.ICM = IC_CE_MODE_DISABLED;
    IC1CONbits.ICTMR = IC_TIMER_2;
    IC1CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC1CONbits.ICM = IC_CE_MODE_EDGE;
    _IC1IP = 7;     //Priority 7 (highest))
    _IC1IF = 0;     // Zero interrupt flag
    _IC1IE = 1;     // Enable interrupt
    
    IC2CONbits.ICM = IC_CE_MODE_DISABLED;
    IC2CONbits.ICTMR = IC_TIMER_2;
    IC2CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC2CONbits.ICM = IC_CE_MODE_EDGE;
    _IC2IP = 7;     //Priority 7 (highest))
    _IC2IF = 0;     // Zero interrupt flag
    _IC2IE = 1;     // Enable interrupt
    
    IC3CONbits.ICM = IC_CE_MODE_DISABLED;
    IC3CONbits.ICTMR = IC_TIMER_2;
    IC3CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC3CONbits.ICM = IC_CE_MODE_EDGE;
    _IC3IP = 7;     //Priority 7 (highest))
    _IC3IF = 0;     // Zero interrupt flag
    _IC3IE = 1;     // Enable interrupt
    
    IC4CONbits.ICM = IC_CE_MODE_DISABLED;
    IC4CONbits.ICTMR = IC_TIMER_2;
    IC4CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC4CONbits.ICM = IC_CE_MODE_EDGE;
    _IC4IP = 7;     //Priority 7 (highest))
    _IC4IF = 0;     // Zero interrupt flag
    _IC4IE = 1;     // Enable interrupt
    
    while ( 1 ) 
    {
        UART_write_string( UARTm1, "%ld %ld\n", encoderRoll, encoderPitch );
        delay_ms( 300 );
    }
    
    return ( 0 );
}

void __attribute__( (__interrupt__, no_auto_psv) ) _IC1Interrupt()
{
    if ( _RD8 )
        encoderRoll += (_RD9<<1)-1;
    else
        encoderRoll += 1-(_RD9<<1);
    
    uint16_t trash = IC1BUF;
    _IC1IF = 0;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _IC2Interrupt()
{
    if ( _RD9 )
        encoderRoll += 1-(_RD8<<1);
    else
        encoderRoll += (_RD8<<1)-1;
    
    uint16_t trash = IC2BUF;
    _IC2IF = 0;
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

void __attribute__( (__interrupt__, no_auto_psv) ) _IC4Interrupt()
{
    if ( _RD11 )
        encoderPitch += 1-(_RD10<<1);
    else
        encoderPitch += (_RD10<<1)-1;
    
    uint16_t trash = IC4BUF;
    _IC4IF = 0;
}
