/*
 * File:   PID_control_system.c
 * Author: alexey
 *
 * Created on December 29, 2016, 10:34 PM
 */

#include "core.h"

PID_rates_t roll_rates  = { .prop_rev = 7, .integr_rev = 4000, .diff = 25 },
            pitch_rates = { .prop_rev = 7, .integr_rev = 4000, .diff = 25 };

static int64_t  integr_sum_pitch = 0;
static int64_t  integr_sum_roll  = 0;

void PID_controller_reset_integral_sums ( void )
{
    integr_sum_pitch = integr_sum_roll = 0;
}

int32_t PID_controller_generate_pitch_control( error_value_t error )
{
    static error_value_t  prev_error = 0;
    
    integr_sum_pitch += error/10;

    int32_t regul = (error/pitch_rates.prop_rev + integr_sum_pitch/pitch_rates.integr_rev + (error - prev_error) * pitch_rates.diff);

    prev_error = error;

    return( regul );
}

int32_t PID_controller_generate_roll_control( error_value_t error )
{
    static error_value_t  prev_error = 0;
    
    integr_sum_roll += error/10;

    int32_t regul = (error/roll_rates.prop_rev + integr_sum_roll/roll_rates.integr_rev + (error - prev_error) * roll_rates.diff);

    prev_error = error;

    return( regul );
}