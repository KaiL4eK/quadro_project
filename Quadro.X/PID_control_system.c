/*
 * File:   PID_control_system.c
 * Author: alexey
 *
 * Created on December 29, 2016, 10:34 PM
 */

#include "core.h"

#define PROP_RATE       15
#define INTEGR_RATE     0.054
#define DIFF_RATE       20

PID_rates_float_t   roll_rates  = { .prop = PROP_RATE,   .integr = INTEGR_RATE,    .diff = DIFF_RATE },
                    pitch_rates = { .prop = PROP_RATE,   .integr = INTEGR_RATE,    .diff = DIFF_RATE },
                    yaw_rates   = { .prop = 43,   .integr = 0.030 };

float       integr_sum_pitch = 0;
float       integr_sum_roll  = 0;
float       integr_sum_yaw   = 0;

PID_parts_t pitch_parts,
            roll_parts,
            yaw_parts;

#define CONTROL_LIMIT 1000
#define CONTROL_LIMIT_INTEGR 1000

void PID_controller_reset_integral_sums ( void )
{
    integr_sum_pitch = integr_sum_roll = integr_sum_yaw = 0;
}

int16_t PID_controller_generate_pitch_control( float error, float angle_speed )
{
    static float  prev_angle_speed = 0;
//    static float prev_error = 0;
    
    integr_sum_pitch += error * pitch_rates.integr;
    integr_sum_pitch = clip_value( integr_sum_pitch, -CONTROL_LIMIT_INTEGR, CONTROL_LIMIT_INTEGR );

    pitch_parts.p = error * pitch_rates.prop;
    pitch_parts.i = integr_sum_pitch;
//    pitch_parts.d = (error - prev_error) * pitch_rates.diff;
    pitch_parts.d = (prev_angle_speed - angle_speed) * pitch_rates.diff;

    int32_t regul = (pitch_parts.p + pitch_parts.i + pitch_parts.d);

//    prev_error = error;    
    prev_angle_speed = angle_speed;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}

int16_t PID_controller_generate_roll_control( float error, float angle_speed )
{
    static float  prev_angle_speed = 0;
//    static float prev_error = 0;
    
    integr_sum_roll += error * roll_rates.integr;
    integr_sum_roll = clip_value( integr_sum_roll, -CONTROL_LIMIT_INTEGR, CONTROL_LIMIT_INTEGR );

    roll_parts.p = error * roll_rates.prop;
    roll_parts.i = integr_sum_roll;
//    roll_parts.d = (error - prev_error) * roll_rates.diff;
    roll_parts.d = (prev_angle_speed - angle_speed) * roll_rates.diff;
    
    int32_t regul = (roll_parts.p + roll_parts.i + roll_parts.d);

//    prev_error = error;    
    prev_angle_speed = angle_speed;

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}

int16_t PID_controller_generate_yaw_control( float error )
{
    integr_sum_yaw += error * yaw_rates.integr;
    integr_sum_yaw = clip_value( integr_sum_yaw, -CONTROL_LIMIT_INTEGR, CONTROL_LIMIT_INTEGR );

    yaw_parts.p = error * yaw_rates.prop;
    yaw_parts.i = integr_sum_yaw;
    
    int32_t regul = (yaw_parts.p + yaw_parts.i);

    return( clip_value(regul, -CONTROL_LIMIT, CONTROL_LIMIT) );
}
