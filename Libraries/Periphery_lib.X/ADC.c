#include "ADC.h"

/********************************/
/*              ADC             */
/********************************/

// ADC initialization for channel 5

void ADC_init()
{
    AD1PCFGLbits.PCFG5 = 0;
    AD1CHS0 = 0x0005;
    AD1CON1bits.ASAM = 1;
    AD1CON1bits.SSRC = 0b111;
    AD1CON1bits.AD12B = 1;          // 0 = 10 bit ADC; 1 = 12 bit ADC
    AD1CON2 = 0x0000;	
    AD1CON3bits.SAMC=0x3;			// auto sample time 
    AD1CON3bits.ADCS=0x1f;	 		// conversion clock select 	         	
	AD1CON1bits.ADON = 1; 
}

int16_t ADC_res = 0;

int16_t ADC_read( void )
{	
    if ( AD1CON1bits.DONE )
    {
        AD1CON1bits.DONE = 0;            // reset DONE bit
        ADC_res = ADC1BUF0;
    }
	return( ADC_res );       			// read ADC1 data      
}

