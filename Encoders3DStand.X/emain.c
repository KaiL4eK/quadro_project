/*
 * File:   emain.c
 * Author: alexey
 *
 * Created on June 4, 2016, 12:37 PM
 */

#include <xc.h>
#include "per_proto.h"
#include "pragmas.h"

#define ADC_input  18

void ADC1_init ( uint8_t channel )
{
    if ( channel > 32 )
        return;
    
    AD1CON1bits.ADON = 0;
    if ( channel > 15 )
        AD1PCFGL &= ~(1 << (channel-16)); // Analog mode pin setup
    else
        AD1PCFGL &= ~(1 << channel); // Analog mode pin setup
    
    AD1CON1bits.ASAM = 1;        // Auto sample
    AD1CON1bits.SSRC = 0b111;    // Auto convertion
    AD1CON1bits.AD12B = 0;       // 0 = 10 bit ADC; 1 = 12 bit ADC
    AD1CON2 = 0x0000;	
    AD1CON3bits.SAMC = 0b11111;	     // Sample time 
    AD1CON3bits.ADCS = 0b11111111;   // Conversion clock select 	         	
	AD1CHS0 = channel;
    AD1CON1bits.ADON = 1;
}

int16_t ADC1_read( void )
{	
    int16_t ADC_res = 0;
    while ( !AD1CON1bits.DONE )
    {
        AD1CON1bits.DONE = 0;            // reset DONE bit
        ADC_res = ADC1BUF0;
    }
	return( ADC_res );       			// read ADC1 data      
}

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    
    UART_init( UARTm2, UART_460800 );
    UART_write_string( UARTm2, "UART initialized\n" );
    
    ADC1_init( ADC_input );
    
    while ( 1 )
    {
//        int16_t result = ADC_read();
        UART_write_string( UARTm2, "Res: %d\n", _RA15 );
        delay_ms( 300 );
    }
    
    return 0;
}
