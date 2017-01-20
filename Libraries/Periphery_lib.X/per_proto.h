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

#define RADIANS_TO_DEGREES          57.295779513f
#define DEGREES_TO_RADIANS          0.017453293f

#define SWAP( x, y ) { uint8_t tmp = x; x = y; y = tmp; }
#define OFF_ALL_ANALOG_INPUTS   { AD1PCFGL = 0x1fff; }

#define FOSC        32000000ULL
#define FCY         (FOSC/2)

/*** ADC.c ***/

int ADC_init ( uint8_t channel );
int16_t ADC_read( void );

/*** UART.c ***/

typedef enum
{
    // High speed values BRGH = 1
    UART_460800 = 8,
    UART_230400 = 16,
    UART_115200 = 34,        
    UART_57600 = 68,
    // Low speed values BRGH = 0
    UART_38400 = 25,
    UART_19200 = 51,
    UART_9600 = 103
} UART_speed_t;
 
typedef enum
{
    UARTr_interrupt = 1,
    UARTr_polling = 2
} UART_receiveMode_t;

typedef enum
{
    UARTmUndef = 100,
    UARTm1 = 0,
    UARTm2 = 1
} UART_moduleNum_t;

typedef enum
{
    UART_little_endian,
    UART_big_endian
} UART_write_endian_t;

typedef enum
{
    INT_PRIO_OFF = 0,
    INT_PRIO_LOWEST = 1,
    INT_PRIO_LOW = 2,
    INT_PRIO_MID_LOW = 3,
    INT_PRIO_MID = 4,
    INT_PRIO_MID_HIGH = 5,
    INT_PRIO_HIGH = 6,
    INT_PRIO_HIGHEST = 7
} Interrupt_priority_lvl_t;

void UART_init( UART_moduleNum_t module, UART_speed_t baud, Interrupt_priority_lvl_t priority );
void UART_write_byte( UART_moduleNum_t module, uint8_t elem );
void UART_write_words( UART_moduleNum_t module, uint16_t *arr, uint8_t count );
void UART_write_string( UART_moduleNum_t module, const char *fstring, ... );

void UART_write_set_endian ( UART_moduleNum_t module, UART_write_endian_t mode );

uint8_t UART_bytes_available( UART_moduleNum_t module );
uint8_t UART_get_byte( UART_moduleNum_t module );
void UART_get_bytes( UART_moduleNum_t module, uint8_t *out_buffer, uint8_t n_bytes );

/*** twi.c ***/

void i2c_init( long Fscl );

int i2c_write_bit_eeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t data);
uint8_t i2c_read_bit_eeprom( uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start );

int i2c_write_bits_eeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t length, uint8_t data);
uint8_t i2c_read_bits_eeprom( uint8_t slave_addr, uint8_t eeprom_addr, uint8_t bit_start, uint8_t length );

int i2c_write_byte_eeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint8_t data);
uint8_t i2c_read_byte_eeprom(uint8_t slave_addr, uint8_t eeprom_addr);

int i2c_write_bytes_eeprom( uint8_t slave_addr, uint8_t eeprom_addr, uint8_t size, uint8_t *data_buffer );
int i2c_read_bytes_eeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint8_t lenght, uint8_t *data);

int i2c_write_word_eeprom(uint8_t slave_addr, uint8_t eeprom_addr, uint16_t data);

/*** spi.c ***/

typedef enum
{
    SPI_SPEED_LOW       = 0b00,
    SPI_SPEED_MID       = 0b01,
    SPI_SPEED_HIGH      = 0b10,
    SPI_SPEED_HIGHEST   = 0b11
}SPI_speed_t;

void spi_init( uint8_t idle_state );
uint8_t spi_write( uint8_t data );
uint8_t spi_read( void );
void spi_cs_set( uint8_t bit );
void spi_set_speed( SPI_speed_t primary );

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

#define clip_value( val, min, max ) ((val) > max ? max : (val) < min ? min : (val))

#endif	/* PERIPHERY_PROTO_H_ */

