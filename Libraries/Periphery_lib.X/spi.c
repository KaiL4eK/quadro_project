#include "per_proto.h"

#define CS_TRIS _TRISA0
#define CS_LAT  _LATA0

void spi_init( void )
{
    SPI2STATbits.SPIEN = 0;     // Disable SPI2 module
    
    CS_TRIS = 0;                // Chip select bit as output
    CS_LAT = 1;
    
    IFS2bits.SPI2IF = 0;
    IEC2bits.SPI2IE = 0;
    
    SPI2CON1bits.DISSCK = 0;    // Internal serial clock is enabled
    SPI2CON1bits.DISSDO = 0;    // SDO2 pin is controlled by the module
    SPI2CON1bits.MODE16 = 0;    // Communication is byte-wide (8 bits)
    SPI2CON1bits.MSTEN = 1;     // Master mode enabled
    SPI2CON1bits.SMP = 1;       // 1 = Input data sampled at end of data output time; 
                                // 0 = Input data sampled at middle of data output time
    SPI2CON1bits.CKE = 1;       // 1 = Serial output data changes on transition from active clock state to Idle clock state
                                // 0 = Serial output data changes on transition from Idle clock state to active clock state
    SPI2CON1bits.CKP = 0;       // 1 = Idle state for clock is a high level; active state is a low level
                                // 0 = Idle state for clock is a low level; active state is a high level
    SPI2CON1bits.PPRE = 0b00;   // 11 = Primary prescale 1:1
                                // 10 = Primary prescale 4:1
                                // 01 = Primary prescale 16:1
                                // 00 = Primary prescale 64:1
    SPI2CON1bits.SPRE = 0b000;  // 111 = Secondary prescale 1:1
                                // 110 = Secondary prescale 2:1
                                // •
                                // 000 = Secondary prescale 8:1
    SPI2STATbits.SPIEN = 1;     // Enable SPI2 module
}

void spi_set_speed( SPI_speed_t speed )
{
    SPI2STATbits.SPIEN = 0;     // Disable SPI2 module
    
    switch( speed )
    {
        case FREQ_16000K:
            SPI2CON1bits.PPRE = 0b11;   // Primary prescale 1:1
            break;
        case FREQ_4000K:
            SPI2CON1bits.PPRE = 0b10;   // Primary prescale 4:1
            break;
        case FREQ_1000K:
            SPI2CON1bits.PPRE = 0b01;   // Primary prescale 16:1
            break;
        case FREQ_125K:
            SPI2CON1bits.PPRE = 0b00;   // Primary prescale 64:1
            break;
    }
    
    SPI2STATbits.SPIEN = 1;     // Enable SPI2 module
}

void spi_cs_set( uint8_t bit )
{
    CS_LAT = bit;
}

uint8_t spi_write( uint8_t data )
{
    SPI2BUF = data;                    // write to buffer for TX
    while ( SPI2STATbits.SPITBF );
    while ( !SPI2STATbits.SPIRBF );    // wait for transfer to complete
    uint8_t rcv_data = SPI2BUF;
    return rcv_data;                    // read the received value
}

uint8_t spi_read( void )
{
    return( spi_write(0xff) );
}
