#ifndef PERIPHERY_PROTO_H_
#define	PERIPHERY_PROTO_H_

#include <stdint.h>
#include <xc.h>

#define DEBUG
#ifdef DEBUG
#define debug( x ) UART_writeln_string( x );
#define fdebug( x, y )  { UART_write_string( x": " );UART_write_hint32( y ); }
#define idebug( x, y )  { UART_write_string( x": " );UART_write_int32( y ); }
#define hdebug( x ) UART_write_hint( x );
#else
#define debug( x )
#define fdebug( x, y )
#define idebug( x, y )
#define hdebug( x )
#endif

/*** ADC.c ***/

void ADC_init();
int16_t ADC_read( void );

/*** UART.c ***/

void init_UART1( uint32_t UART_br );
void UART_write_words( uint16_t *arr, uint8_t count );
void UART_write_string( char *string );
void UART_writeln_string( char *string );
void UART_write_hint( int num );
void UART_write_int32( int32_t num );
void UART_write_int( int num );
void UART_write_int16( int16_t num );
void UART_write_float( double num );
void UART_write_byte( uint8_t elem );
uint8_t UART_get_last_received_command();
void UART_write_hint32( uint32_t num );

/*** twi.c ***/

void i2c_init( long Fscl );
int8_t i2c_write_byte_eeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint8_t data);
int8_t i2c_write_word_eeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint16_t data);
int8_t i2c_write_bits_eeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t length, uint8_t data);
int8_t i2c_read_bytes_eeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint8_t *data, uint8_t lenght);
uint8_t i2c_read_byte_eeprom(uint8_t slave_addr, uint8_t eeprom_addr);

/*** spi.c ***/

typedef enum
{
    FREQ_16000K,
    FREQ_4000K,
    FREQ_1000K,
    FREQ_125K        
}SPI_speed_t;

void spi_init( void );
uint8_t spi_write( uint8_t data );
uint8_t spi_read( void );
void spi_cs_set( uint8_t bit );
void spi_set_speed( SPI_speed_t speed );

/*** timing.c ***/

void setup_PLL_oscillator( void );

#ifndef DELAY_MS_
#define DELAY_MS_
void delay_ms( uint16_t mseconds );
#endif
#ifndef DELAY_US_
#define DELAY_US_
void delay_us( uint16_t useconds );
#endif

void timer_start();
uint32_t timer_restart();
uint32_t timer_stop();

#define TIMER_MS_TICK (FCY/1000) // Last is prescale of timer
#define TIMER_US_TICK (FCY/1000000) // Last is prescale of timer

#define TIMER_DIV_1   0b00
#define TIMER_DIV_8   0b01
#define TIMER_DIV_64  0b10
#define TIMER_DIV_256 0b11

/*** error.c ***/

#define ERR_LIGHT   _LATA4
#define INIT_ERR_L  { _TRISA4 = 0; ERR_LIGHT = 1; }
void error_process ( void );

/*** flash.c ***/

typedef enum
{
    FILE_NUM
            
}FlashData_t;

int flash_flush ( void );
int flash_read ( void );
int flash_set ( FlashData_t data_type, int data );
int flash_get ( FlashData_t data_type );

#endif	/* PERIPHERY_PROTO_H_ */

