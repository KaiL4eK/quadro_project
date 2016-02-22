#ifndef PERIPHERY_PROTO_H_
#define	PERIPHERY_PROTO_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <xc.h>

#define OFF_WATCH_DOG_TIMER     { RCONbits.SWDTEN = 0; }
#define OFF_ALL_ANALOG_INPUTS   { AD1PCFGL = 0x1fff; }
// Next macro works just with board dsPIC33FJ256MC710 with quartz
#define SWITCH_TO_32MHZ         _FOSCSEL( FNOSC_PRI & IESO_OFF ); \
                                _FOSC( POSCMD_HS & OSCIOFNC_OFF & FCKSM_CSECMD ); \
                                _FWDT( FWDTEN_OFF );              // Watchdog Timer Enabled/disabled by user software

#define FOSC        32000000ULL
#define FCY         (FOSC/2)

/*** ADC.c ***/

int ADC_init ( uint8_t channel );
int16_t ADC_read( void );

/*** UART.c ***/

void init_UART1( uint32_t UART_br );
void UART_write_words( uint16_t *arr, uint8_t count );
void UART_write_string( const char *fstring, ... );
void UART_write_byte( uint8_t elem );
uint8_t UART_get_last_received_command();

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

/** Timer module **/

typedef uint32_t TimerTicks32_t;
typedef uint16_t TimerTicks16_t;

// timer_start, timer_restart, timer_stop - divider = 1
void timer_start();
uint32_t timer_restart();
uint32_t timer_stop();
uint32_t convert_ticks_to_us ( TimerTicks32_t timer_ticks, uint8_t timer_divider );
uint32_t convert_ticks_to_ms ( TimerTicks32_t timer_ticks, uint8_t timer_divider );

#define TIMER_DIV_1   0b00
#define TIMER_DIV_8   0b01
#define TIMER_DIV_64  0b10
#define TIMER_DIV_256 0b11

/** Input capture module **/

#define IC_TIMER_2  1
#define IC_TIMER_3  0

#define IC_CE_MODE_DISABLED         0b000
#define IC_CE_MODE_EDGE             0b001
#define IC_CE_MODE_FALLING_EDGE     0b010
#define IC_CE_MODE_RISING_EDGE      0b011
#define IC_CE_MODE_4TH_RISE_EDGE    0b100
#define IC_CE_MODE_16TH_RISE_EDGE   0b101

#define IC_INT_MODE_1ST_CE    0b00
#define IC_INT_MODE_2ND_CE    0b01
#define IC_INT_MODE_3RD_CE    0b10
#define IC_INT_MODE_4TH_CE    0b11

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

