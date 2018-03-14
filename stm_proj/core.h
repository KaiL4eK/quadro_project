#pragma once

#include <errno.h>

#ifndef EOK
	#define EOK		0
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#define F446RE

extern BaseSequentialStream  *debug_stream;
#define  uprintf  chprintf

#define delay_ms(x) (chThdSleepMilliseconds((x)))

/** ??? **/
typedef void * i2c_module_t;

uint8_t 	i2c_read_byte 	( i2c_module_t p_module, uint8_t i2c_address, uint8_t reg_addr );
int 		i2c_read_bytes 	( i2c_module_t p_module, uint8_t i2c_address, uint8_t reg_addr, uint8_t size, uint8_t *data);
int 		i2c_write_bytes ( i2c_module_t p_module, uint8_t i2c_address, uint8_t reg_addr, uint8_t size, uint8_t *data );
int 		i2c_write_byte 	( i2c_module_t p_module, uint8_t i2c_address, uint8_t reg_addr, uint8_t data );
int 		i2c_write_word 	( i2c_module_t p_module, uint8_t i2c_address, uint8_t reg_addr, uint16_t data );

uint8_t 	i2c_read_bit 	( i2c_module_t p_module, uint8_t i2c_address, uint8_t reg_addr, uint8_t bit_start );
uint8_t 	i2c_read_bits 	( i2c_module_t p_module, uint8_t i2c_address, uint8_t reg_addr, uint8_t bit_start, uint8_t length );
int 		i2c_write_bit 	( i2c_module_t p_module, uint8_t i2c_address, uint8_t reg_addr, uint8_t bit_start, uint8_t data );
int 		i2c_write_bits  ( i2c_module_t p_module, uint8_t i2c_address, uint8_t reg_addr, uint8_t bit_start, uint8_t length, uint8_t data );

