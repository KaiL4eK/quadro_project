/*
 * File:   potentiometer.c
 * Author: alexey
 *
 */

#include "core.h"

#define POTNT_CH 8

void potnt_init ( void )
{
//ADON: A/D Operating Mode bit (1)
//1 = A/D Converter module is operating
//0 = A/D Converter is off
    AD1CON1bits.ADON = 0;
//ASAM: A/D Sample Auto-Start bit
//1 = Sampling begins immediately after last conversion completes; SAMP bit is auto-set
//0 = Sampling begins when the SAMP bit is set
    AD1CON1bits.ASAM = 1;
//SSRC<2:0>: Conversion Trigger Source Select bits
//111 = Internal counter ends sampling and starts conversion (auto-convert)
//110 = CTMU event ends sampling and starts conversion
//101 = Reserved
//100 = Timer5 compare ends sampling and starts conversion
//011 = Reserved
//010 = Timer3 compare ends sampling and starts conversion
//001 = Active transition on INT0 pin ends sampling and starts conversion
//000 = Clearing SAMP bit ends sampling and starts conversion
    AD1CON1bits.SSRC = 0b111;
    AD1CON2 = 0x0000;
//SAMC<4:0>: Auto-Sample Time bits
//11111 = 31 T AD
//·····
//00001 = 1 T AD
//00000 = 0 T AD (not recommended)
    AD1CON3bits.SAMC = 0b11111;
//ADCS<7:0>: A/D Conversion Clock Select bits
//11111111
//······ = Reserved, do not use
//01000000
//00111111 = 64 T CY
//00111110 = 63 T CY
//······
//00000001 = 2 * T CY
//00000000 = T CY
    AD1CON3bits.ADCS = 0;
//CH0SA<4:0>: Channel 0 Positive Input Select for MUX A Multiplexer Setting bits
//10001 = Channel 0 positive input is internal band gap reference (V BG )
//10000 = Channel 0 positive input is V BG /2
//01111 = Channel 0 positive input is AN15
//01110 = Channel 0 positive input is AN14
//01101 = Channel 0 positive input is AN13
//01100 = Channel 0 positive input is AN12
//01011 = Channel 0 positive input is AN11
//01010 = Channel 0 positive input is AN10
//01001 = Channel 0 positive input is AN9
//01000 = Channel 0 positive input is AN8
//00111 = Channel 0 positive input is AN7
//00110 = Channel 0 positive input is AN6
//00101 = Channel 0 positive input is AN5
//00100 = Channel 0 positive input is AN4
//00011 = Channel 0 positive input is AN3
//00010 = Channel 0 positive input is AN2
//00001 = Channel 0 positive input is AN1
//00000 = Channel 0 positive input is AN0
    AD1CHSbits.CH0SA = POTNT_CH;
//PCFG<15:0>: Analog Input Pin Configuration Control bits
//1 = Pin for corresponding analog channel is configured in Digital mode; I/O port read enabled
//0 = Pin configured in Analog mode; I/O port read disabled, A/D samples pin voltage
    AD1PCFGL &= ~(1 << POTNT_CH);
    AD1CSSL = 0;
    AD1CON1bits.ADON = 1;
}

int16_t ADC_res = 0;

int16_t potnt_get_value ( void )
{
//DONE: A/D Conversion Status bit
//1 = A/D conversion is done
//0 = A/D conversion is NOT done
    if ( AD1CON1bits.DONE )
    {
        AD1CON1bits.DONE = 0;            // reset DONE bit
        ADC_res = ADC1BUF0;
    }
	return( ADC_res );       			// read ADC1 data 
}
