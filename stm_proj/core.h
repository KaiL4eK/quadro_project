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

#ifndef clip_value
    #define clip_value( val, min, max ) ((val) > max ? max : (val) < min ? min : (val))
#endif

#define INT_ACCURACY_MULTIPLIER		100

/*** I2C ***/
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

/*** PWM ***/
/* Limits (approx.): -32k --- 32k */
typedef int16_t motor_power_t;


#define MOTOR_INPUT_MIN		0
#define MOTOR_INPUT_MAX		800

typedef enum {
    MOTOR_1 = 0,
    MOTOR_2	= 1,
    MOTOR_3 = 2,
    MOTOR_4 = 3
} motor_num_t;

#define MOTOR_COUNT    		4

void motor_control_init( void );
/* Adviced to call during changing PWM values */
void motor_control_lock( void );
void motor_control_unlock( void );

motor_power_t *motor_control_get_powers_ptr( void );
void motor_control_set_motors_started( void );
void motor_control_set_motors_stopped( void );
bool motor_control_is_armed( void );

/*** PID control system ***/
typedef struct {
    uint16_t        prop_rev,
                    diff,
                    integr_rev;
} PID_rates_t;

typedef struct {
    float           prop,
                    diff,
                    integr;
} PID_rates_float_t;

typedef struct {
    float p, i, d;
} PID_parts_t;

