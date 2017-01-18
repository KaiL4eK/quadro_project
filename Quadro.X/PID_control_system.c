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

static float    integr_sum_pitch = 0;
static float    integr_sum_roll  = 0;
static float    integr_sum_yaw   = 0;

PID_parts_t pitch_parts,
            roll_parts,
            yaw_parts;

#define CONTROL_LIMIT 1500

void PID_controller_reset_integral_sums ( void )
{
    integr_sum_pitch = integr_sum_roll = integr_sum_yaw = 0;
}

//#define INTEGR_LIMIT 1000000L

int16_t PID_controller_generate_pitch_control( error_value_t error )
{
    static error_value_t  prev_error = 0;
    
    integr_sum_pitch += error * pitch_rates.integr_rev;
    integr_sum_pitch = clip_value( integr_sum_pitch, -CONTROL_LIMIT, CONTROL_LIMIT );

    pitch_parts.p = error * pitch_rates.prop_rev;
    pitch_parts.i = integr_sum_pitch;
    pitch_parts.d = (error - prev_error) * pitch_rates.diff;
    
    int32_t regul = (pitch_parts.p + pitch_parts.i + pitch_parts.d);

    prev_error = error;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}

int16_t PID_controller_generate_roll_control( error_value_t error )
{
    static error_value_t  prev_error = 0;
    
    integr_sum_roll += error * roll_rates.integr_rev;
    integr_sum_roll = clip_value( integr_sum_roll, -CONTROL_LIMIT, CONTROL_LIMIT );

    roll_parts.p = error * roll_rates.prop_rev;
    roll_parts.i = integr_sum_roll;
    roll_parts.d = (error - prev_error) * roll_rates.diff;
    
    int32_t regul = (roll_parts.p + roll_parts.i + roll_parts.d);

    prev_error = error;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}

int16_t PID_controller_generate_yaw_control( error_value_t error )
{
    static error_value_t  prev_error = 0;
    
    integr_sum_yaw += error * yaw_rates.integr_rev;
    integr_sum_yaw = clip_value( integr_sum_yaw, -CONTROL_LIMIT, CONTROL_LIMIT );

    yaw_parts.p = error * yaw_rates.prop_rev;
    yaw_parts.i = integr_sum_yaw;
    yaw_parts.d = (error - prev_error) * yaw_rates.diff;
    
    int32_t regul = (yaw_parts.p + yaw_parts.i + yaw_parts.d);

    prev_error = error;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}
