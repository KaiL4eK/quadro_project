/*
 * File:   ad7705.c
 * Author: alex
 *
 * Created on May 17, 2016, 11:37 AM
 */

#include "ad7705.h"

Communication_reg_t comm_reg;

//void ad7705_wait_for_data_ready ( void )
//{
//    do {        
//        comm_reg.val = ad7705_read_register( COMMUNICATION );
//    } while ( comm_reg.bits.nDRDY );
//}

int ad7705_init_clock_register ( uint8_t channel );
int ad7705_init_setup_register ( uint8_t channel, uint8_t gain_val );

static int ad7705_turnOn ( void )
{
    nRESET_PIN = 0;
    delay_ms( 1 );
    nRESET_PIN = 1;
    return( 0 );
}

int ad7705_init ( void )
{
    int res = 0;
    nRESET_PIN_DIR = 0;
    nRESET_PIN = 0;    
    nDRDY_PIN_DIR = 1;
    
    ad7705_turnOn();
    
    spi_cs_set( 0 );
    
//    wait_for_data_ready();
    
    spi_write( 0xFF );
    spi_write( 0xFF );
    spi_write( 0xFF );
    spi_write( 0xFF );
    
    if ( ad7705_read_register( SETUP, CHANNEL_1 ) != SETUP_INITIAL_VAL )
        res = -1;
    else if ( ad7705_read_register( CLOCK, CHANNEL_1 ) != CLOCK_INITIAL_VAL )
        res = -2;
    else if ( ad7705_read_register( GAIN, CHANNEL_1 ) != GAIN_INITIAL_VAL )
        res = -3;
    else if ( ad7705_read_register( OFFSET, CHANNEL_1 ) != OFFSET_INITIAL_VAL )
        res = -4;
    else if ( ad7705_read_register( TEST, CHANNEL_1 ) != TEST_INITIAL_VAL )    // The major check!
        res = -5;
    else
    {   // Normal initialization
        ad7705_init_clock_register( CHANNEL_1 );
        ad7705_init_setup_register( CHANNEL_1, GAIN_VAL_64 );
        while ( nDRDY_PIN ) { Nop(); }
    }
    
    spi_cs_set( 1 );
    return( res );
}

uint32_t ad7705_read_register ( AD7705_reg register_addr, uint8_t channel )
{
    uint32_t result = 0;
    spi_cs_set( 0 );
    
    comm_reg.val = 0;   // Zero
    comm_reg.bits.RS = register_addr & 0x7;
    comm_reg.bits.RW = READ_BIT;
    comm_reg.bits.STBY = 0;
    comm_reg.bits.CH = channel & CHANNEL_MASK;
    comm_reg.bits.nDRDY = 0;
    spi_write( comm_reg.val );
//    UART_write_string( "Write byte: 0x%x to communication reg\n", comm_reg.val );
    
    switch ( register_addr )
    {
        // 24 bits registers
        case OFFSET:
        case GAIN:
            result = spi_read();
        // 16 bits register
        case DATA:
            result = result << 8 | spi_read();
        // 8 bits registers
        case SETUP:
        case CLOCK:
        case TEST:
        case COMMUNICATION:
            result = result << 8 | spi_read();
            break;
        default:
            break;
    }
    
    spi_cs_set( 1 );
    return( result );
}

int ad7705_write_register ( AD7705_reg register_addr, uint8_t channel, uint8_t value )
{
    if ( register_addr == DATA )
        return( INCORRECT_VAL );
    
    spi_cs_set( 0 );
    
    Communication_reg_t comm_reg;
    comm_reg.bits.RS = register_addr & 0x7;
    comm_reg.bits.RW = WRITE_BIT;
    comm_reg.bits.STBY = 0;
    comm_reg.bits.CH = channel & CHANNEL_MASK;
    comm_reg.bits.nDRDY = 0;
    spi_write( comm_reg.val );
//    UART_write_string( "Write byte: 0x%x to communication reg\n", comm_reg.val );
    spi_write( value );
//    UART_write_string( "Write byte: 0x%x\n", value );
    spi_cs_set( 1 );
    return( 0 );
}

int ad7705_init_clock_register ( uint8_t channel )
{
    Clock_reg_t clock_reg;
    clock_reg.val = 0;
    clock_reg.bits.FS = 0b11;   // 0b11 - 500Hz / 0b10 - 250Hz / 0b01 - 60 Hz
    clock_reg.bits.CLK = 1;     // 2.4576/4.9152MHz - 1
    clock_reg.bits.CLKDIV = 0;  // 2.4576/1MHz      - 0
    clock_reg.bits.CLKDIS = 0;
    clock_reg.bits.ZERO = 0b000;

//    UART_write_string( "Clock\t" );
    ad7705_write_register( CLOCK, channel, clock_reg.val );
    
    return( 0 );
}

bool ad7705_is_data_ready( void )
{
    return( !nDRDY_PIN );
}

uint16_t ad7705_read_data( uint8_t channel )
{
    return( ad7705_read_register( DATA, channel ) );
}

int ad7705_init_setup_register ( uint8_t channel, uint8_t gain_val )
{
    Setup_reg_t setup_reg;
    setup_reg.val = 0;

    setup_reg.bits.G = gain_val & GAIN_MASK;
    setup_reg.bits.MD = 0b01;   // Self-calibration mode
    setup_reg.bits.nBU = 1;
    setup_reg.bits.BUF = 0;
    setup_reg.bits.FSYNC = 0;   // Must be zero!

//    UART_write_string( "Setup\t" );
    ad7705_write_register( SETUP, channel, setup_reg.val );
    
    return( 0 );
}
