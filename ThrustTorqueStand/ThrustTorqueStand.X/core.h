#ifndef CORE_H_
#define	CORE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <xc.h>
#include "freq.h"

void delay_ms ( uint16_t time_ms );
void delay_us ( uint16_t time_us );
void pll_32MHz_init ( void );

/*** UART.c ***/

typedef enum
{
    UART_460800 = 8,
    UART_230400 = 16,
    UART_115200 = 34,        
    UART_57600 = 68,
    UART_38400 = 103,
    UART_19200 = 207        
}UART_speed_t;

uint8_t UART_bytes_available( void );
uint8_t UART_get_byte( void );

void init_UART1( UART_speed_t UART_br );
void UART_write_string( const char *fstring, ... );
void UART_write_byte( uint8_t elem );

/*** spi.c ***/

typedef enum
{
    SPI_PRIM_64 = 0b00,
    SPI_PRIM_16 = 0b01,
    SPI_PRIM_4 = 0b10,
    SPI_PRIM_1 = 0b11
}SPI_primPrescale_t;

typedef enum
{
    SPI_SEC_8 = 0b000,
    SPI_SEC_7 = 0b001,
    SPI_SEC_6 = 0b010,
    SPI_SEC_5 = 0b011,
    SPI_SEC_4 = 0b100,
    SPI_SEC_3 = 0b101,
    SPI_SEC_2 = 0b110,
    SPI_SEC_1 = 0b111
}SPI_secondPrescale_t;

void spi_init( void );
uint8_t spi_write( uint8_t data );
uint8_t spi_read( void );
void spi_cs_set( uint8_t bit );
void spi_set_speed( SPI_primPrescale_t primary, SPI_secondPrescale_t secondary );


void i2c_init( uint32_t Fscl );
int8_t i2c_write_byte_eeprom( uint8_t slave_addr, uint8_t eeprom_addr, uint8_t data );
int8_t i2c_write_word_eeprom( uint8_t slave_addr, uint8_t eeprom_addr, uint16_t data );
int8_t i2c_write_bits_eeprom( uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t length, uint8_t data );
int8_t i2c_read_bytes_eeprom( uint8_t slave_addr, uint8_t eeprom_addr, uint8_t *data, uint8_t lenght );
uint8_t i2c_read_byte_eeprom( uint8_t slave_addr, uint8_t eeprom_addr );

void lcd_init( void );
void lcd_write_string ( char * );
void lcd_clear ( void );
void lcd_setLineOne ( void );
void lcd_setLineTwo ( void );

void potnt_init ( void );
int16_t potnt_get_value ( void );

void pwm_init ( uint16_t period_ms );
void pwm_set_dutyCycle_percent ( uint8_t percentage );

typedef struct
{
    uint8_t tm_sec,
            tm_min,
            tm_hour,
            tm_mday,
            tm_mon,
            tm_year,
            tm_wday,
            tm_yday,
            tm_isdst;
} time_t;

void rtc_init ( void );
void rtc_receive_time ( void );
time_t *rtc_get_raw_data ( void );

#endif	/* CORE_H_ */






