/*
 * File:   PID_control_system.c
 * Author: alexey
 *
 * Created on December 29, 2016, 10:34 PM
 */

#include "core.h"

PID_rates_float_t   roll_rates  = { .prop = 4.5,   .integr = 0.00000,    .diff = 20 },
                    pitch_rates = { .prop = 4.5,   .integr = 0.00000,    .diff = 20 },
                    yaw_rates   = { .prop = 0.00,   .integr = 0.00000,    .diff = 0 };

float       integr_sum_pitch = 0;
float       integr_sum_roll  = 0;
float       integr_sum_yaw   = 0;

PID_parts_t pitch_parts,
            roll_parts,
            yaw_parts;

#define CONTROL_LIMIT 1500

void PID_controller_reset_integral_sums ( void )
{
    integr_sum_pitch = integr_sum_roll = integr_sum_yaw = 0;
}

//#define INTEGR_LIMIT 1000000L

int16_t PID_controller_generate_pitch_control( float error, float angle_speed )
{
    static error_value_t  prev_angle_speed = 0;
    
    integr_sum_pitch += error * pitch_rates.integr;
    integr_sum_pitch = clip_value( integr_sum_pitch, -CONTROL_LIMIT, CONTROL_LIMIT );

    pitch_parts.p = error * pitch_rates.prop;
    pitch_parts.i = integr_sum_pitch;
    pitch_parts.d = (angle_speed - prev_angle_speed) * pitch_rates.diff;
    
    int32_t regul = (pitch_parts.p + pitch_parts.i + pitch_parts.d);

    prev_angle_speed = angle_speed;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}

int16_t PID_controller_generate_roll_control( float error, float angle_speed )
{
    static error_value_t  prev_angle_speed = 0;
    
    integr_sum_roll += error * roll_rates.integr;
    integr_sum_roll = clip_value( integr_sum_roll, -CONTROL_LIMIT, CONTROL_LIMIT );

    roll_parts.p = error * roll_rates.prop;
    roll_parts.i = integr_sum_roll;
    roll_parts.d = (angle_speed - prev_angle_speed) * roll_rates.diff;
    
    int32_t regul = (roll_parts.p + roll_parts.i + roll_parts.d);

    prev_angle_speed = angle_speed;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}

int16_t PID_controller_generate_yaw_control( float error )
{
    integr_sum_yaw += error * yaw_rates.integr;
    integr_sum_yaw = clip_value( integr_sum_yaw, -CONTROL_LIMIT, CONTROL_LIMIT );

    yaw_parts.p = error * yaw_rates.prop;
    yaw_parts.i = integr_sum_yaw;
    yaw_parts.d = 0;
    
    int32_t regul = (yaw_parts.p + yaw_parts.i);

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}
