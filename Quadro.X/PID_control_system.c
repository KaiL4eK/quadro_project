/*
 * File:   PID_control_system.c
 * Author: alexey
 *
 * Created on December 29, 2016, 10:34 PM
 */

#include "core.h"

//PID_rates_t roll_rates  = { .prop_rev = 8, .integr_rev = 20000, .diff = 1 },
//            pitch_rates = { .prop_rev = 8, .integr_rev = 20000, .diff = 1 };

//PID_rates_float_t   roll_rates  = { .prop_rev = 0.08, .integr_rev = 0.00006, .diff = 8.2 },
//                    pitch_rates = { .prop_rev = 0.08, .integr_rev = 0.00006, .diff = 8.2 };

PID_rates_float_t   roll_rates  = { .prop_rev = 0.3,   .integr_rev = 0.00013,    .diff = 8 },
                    pitch_rates = { .prop_rev = 0.3,   .integr_rev = 0.00013,    .diff = 8 },
                    yaw_rates   = { .prop_rev = 0.05,  .integr_rev = 0.00005,    .diff = 0 };

static int32_t  integr_sum_pitch = 0;
static int32_t  integr_sum_roll  = 0;
static int32_t  integr_sum_yaw   = 0;

PID_parts_t pitch_parts,
            roll_parts,
            yaw_parts;

#define CONTROL_LIMIT 3000

void PID_controller_reset_integral_sums ( void )
{
    integr_sum_pitch = integr_sum_roll = integr_sum_yaw = 0;
}

#define INTEGR_LIMIT 1000000L

int16_t PID_controller_generate_pitch_control( error_value_t error )
{
    static error_value_t  prev_error = 0;
    
    integr_sum_pitch += error;
    integr_sum_pitch = clip_value( integr_sum_pitch, -INTEGR_LIMIT, INTEGR_LIMIT );

    pitch_parts.p = error * pitch_rates.prop_rev;
    pitch_parts.i = integr_sum_pitch * pitch_rates.integr_rev;
    pitch_parts.d = (error - prev_error) * pitch_rates.diff;
    
    int32_t regul = (pitch_parts.p + pitch_parts.i + pitch_parts.d);

    prev_error = error;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}

int16_t PID_controller_generate_roll_control( error_value_t error )
{
    static error_value_t  prev_error = 0;
    
    integr_sum_roll += error;
    integr_sum_roll = clip_value( integr_sum_roll, -INTEGR_LIMIT, INTEGR_LIMIT );

    roll_parts.p = error * roll_rates.prop_rev;
    roll_parts.i = integr_sum_roll * roll_rates.integr_rev;
    roll_parts.d = (error - prev_error) * roll_rates.diff;
    
    int32_t regul = (roll_parts.p + roll_parts.i + roll_parts.d);

    prev_error = error;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}

int16_t PID_controller_generate_yaw_control( error_value_t error )
{
    static error_value_t  prev_error = 0;
    
    integr_sum_yaw += error;
    integr_sum_yaw = clip_value( integr_sum_yaw, -INTEGR_LIMIT/10, INTEGR_LIMIT/10 );

    yaw_parts.p = error * yaw_rates.prop_rev;
    yaw_parts.i = integr_sum_yaw * yaw_rates.integr_rev;
    yaw_parts.d = (error - prev_error) * yaw_rates.diff;
    
    int32_t regul = (yaw_parts.p + yaw_parts.i + yaw_parts.d);

    prev_error = error;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}
