#pragma once

#include <errno.h>

#ifndef EOK
	#define EOK		0
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

// Libraries
#include <MPU6050.h>
#include <filters.h>

#define F446RE

#define SYSTEM_FREQUENCY        180000000UL

#define PROGRAM_ROUTINE_MASTER                  0
#define PROGRAM_ROUTINE_MPU6050_CALIBRATION     1
#define PROGRAM_ROUTINE_RC_CALIBRATION          2

#define MAIN_PROGRAM_ROUTINE                    PROGRAM_ROUTINE_MASTER

//#define TIME_MEASUREMENT_DEBUG

extern BaseSequentialStream  *debug_stream;
#define uprintf  chprintf
#define dprintf_str(str)                chprintf(debug_stream, str);
#define dprintf(str, ...)               chprintf(debug_stream, str, __VA_ARGS__);
#define dprintf_mod(mod, str, ...)      dprintf(mod str, __VA_ARGS__);
#define dprintf_mod_str(mod, str)       dprintf_str(mod str);


#define delay_ms(x) (chThdSleepMilliseconds((x)))

#ifndef clip_value
    #define clip_value( val, min, max ) ((val) > max ? max : (val) < min ? min : (val))
#endif

#define INT_ACCURACY_MULTIPLIER		100

/*** Common ***/

static inline void init_common_periphery ( void )
{
    EXTConfig extcfg;
    /* Zero all fields */
    memset( &extcfg, 0, sizeof( extcfg ) );
    extStart( &EXTD1, &extcfg );
}

/*** AHRS ***/

// #define AHRS_TIME_MEASUREMENT_DEBUG

extern euler_angles_t euler_angles;
extern euler_angles_t gyro_rates;

void ahrs_module_init ( void );

/*** Motor control ***/

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
void motor_control_update_PWM( void );

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

int16_t PID_controller_generate_pitch_control( float error, float angle_speed );
int16_t PID_controller_generate_roll_control( float error, float angle_speed );
int16_t PID_controller_generate_yaw_control( float error );

/*** Radio control ***/
/*
 * Use periphery:
 *      EXT on PB2, PB1, PB15, PB14, PB13 (in order of channels)
 *      TIM7 for measuring
 */

// #define RC_TIME_MEASUREMENT_DEBUG

typedef enum {
    ROLL        = 0,
    PITCH       = 1,

    THRUST      = 2,
    YAW         = 3,

    SWITCH      = 4

} rc_control_name_t;

#define MAX_CHANNELS_USED       5

typedef struct {

#define MIN_CONTROL_VALUE       0
#define MAX_CONTROL_VALUE       2000

    /* Control value is in range [0; 100] */
    int32_t    channels[MAX_CHANNELS_USED];
} rc_control_values_t;

extern rc_control_values_t      control_input;

#ifdef RC_TIME_MEASUREMENT_DEBUG
extern time_measurement_t       rc_tm;
#endif

void radio_control_init ( void );
bool radio_control_is_connected ( void );

/**
 * @brief   Start calibration process
 * @note    Debug stream must be enabled, limits move to serial
 */
int radio_control_calibration ( void );

/* Radio control flight */

// [0; 2000] -> [true, false]
static inline bool radio_control_is_switch_enabled( void )
{
    return (control_input.channels[SWITCH] > (MAX_CONTROL_VALUE / 2));
}

// [0; 2000] -> [-1000; 1000]
static inline int32_t radio_control_get_angle_value( rc_control_name_t angle_name )
{
    return (control_input.channels[angle_name] - (MAX_CONTROL_VALUE / 2));
}

// [0; 2000] -> [0; 500]
static inline int32_t radio_control_get_thrust_value( void )
{
    return (control_input.channels[THRUST] * 500 / MAX_CONTROL_VALUE);
}
