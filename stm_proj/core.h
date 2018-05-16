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

#ifdef CORE_DEBUG_ENABLED
    #define dprintf(str, ...)               chprintf(debug_stream, str, ##__VA_ARGS__);
    #define dprintf_mod(mod, str, ...)      dprintf(mod str, ##__VA_ARGS__);
#else
    #define dprintf(str, ...)
    #define dprintf_mod(mod, str, ...)
#endif

#define delay_ms(x) (chThdSleepMilliseconds((x)))

#ifndef clip_value
    #define clip_value( val, min, max ) ((val) > max ? max : (val) < min ? min : (val))
#endif

#define INT_ACCURACY_MULTIPLIER		100

/*** Common ***/

#define  CONTROL_SYSTEM_PERIOD_MS   5

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

void ahrs_module_init ( tprio_t prio );

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
extern motor_power_t   motor_powers[MOTOR_COUNT];

void motor_control_init( void );
/* Adviced to call during changing PWM values */
void motor_control_lock( void );
void motor_control_unlock( void );

void motor_control_set_motors_started( void );
void motor_control_set_motors_stopped( void );
bool motor_control_is_armed( void );
void motor_control_update_PWM( void );

/*** PID control system ***/

typedef enum {
    PID_ROLL,
    PID_PITCH,
    PID_YAW
} PID_rates_name_t;

typedef struct {
    float           prop,
                    diff,
                    integr;
} PID_rates_t;

void PID_controller_reset_integral_sums ( void );
int16_t PID_controller_generate_pitch_control( float error, float angle_speed );
int16_t PID_controller_generate_roll_control( float error, float angle_speed );
int16_t PID_controller_generate_yaw_control( float error );

PID_rates_t PID_controller_get_rates( PID_rates_name_t name );
void        PID_controller_set_rates( PID_rates_t *rates_p, PID_rates_name_t name );


/*** Radio control ***/
/*
 * Use periphery:
 *      EXT on PB2, PB1, PB15, PB14, PB13 (in order of channels)
 *      TIM7 for measuring
 */

// #define RC_TIME_MEASUREMENT_DEBUG

typedef enum {
    RC_ROLL        = 0,
    RC_PITCH       = 1,

    RC_THRUST      = 2,
    RC_YAW         = 3,

    RC_SWITCH      = 4

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
    return (control_input.channels[RC_SWITCH] > (MAX_CONTROL_VALUE / 2));
}

// [0; 2000] -> [-1000; 1000]
static inline int32_t radio_control_get_angle_value( rc_control_name_t angle_name )
{
    if ( angle_name != RC_ROLL &&  angle_name != RC_PITCH && angle_name != RC_YAW )
        return 0;

    return (control_input.channels[angle_name] - (MAX_CONTROL_VALUE / 2));
}

// [0; 2000] -> [0; 500]
static inline int32_t radio_control_get_thrust_value( void )
{
    return (control_input.channels[RC_THRUST] * 500 / MAX_CONTROL_VALUE);
}

/*** BT Control ***/
/* The module uses other modules functions to represent data */

int bt_control_init( tprio_t prio );
