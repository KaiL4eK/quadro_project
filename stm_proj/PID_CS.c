#include <core.h>

#define PROP_RATE       1
#define INTEGR_RATE     0
#define DIFF_RATE       10

typedef struct {
    float p, i, d;
} PID_parts_t;

PID_rates_t     roll_rates  = { .prop = PROP_RATE,   .integr = INTEGR_RATE,    .diff = DIFF_RATE },
                pitch_rates = { .prop = PROP_RATE,   .integr = INTEGR_RATE,    .diff = DIFF_RATE },
                yaw_rates   = { .prop = 10,          .integr = 0,              .diff = 0 };

float       integr_sum_pitch = 0;
float       integr_sum_roll  = 0;
float       integr_sum_yaw   = 0;

PID_parts_t pitch_parts,
            roll_parts,
            yaw_parts;

#define CONTROL_LIMIT           (MOTOR_INPUT_MAX - 100)
#define CONTROL_LIMIT_INTEGR    (MOTOR_INPUT_MAX - 300)

void PID_controller_reset_integral_sums ( void )
{
    integr_sum_pitch = integr_sum_roll = integr_sum_yaw = 0;
}

void PID_controller_set_rates( PID_rates_t *rates_p, PID_rates_name_t name )
{
    switch ( name )
    {
        case PID_ROLL:
            roll_rates = *rates_p;
            break;

        case PID_PITCH:
            pitch_rates = *rates_p;
            break;

        case PID_YAW:
            yaw_rates = *rates_p;
            break;

        default:
            ;
    }
}

PID_rates_t PID_controller_get_rates( PID_rates_name_t name )
{
    switch ( name )
    {
        case PID_ROLL:
            return roll_rates;

        case PID_PITCH:
            return pitch_rates;

        case PID_YAW:
            return yaw_rates;

        default:
            ;
    }

    PID_rates_t zero = {0, 0, 0};
    return zero;
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

    /* Inversed to have negative derivative (in feedback) */
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

    /* Inversed to have negative derivative (in feedback) */
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
