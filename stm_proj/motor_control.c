#include <core.h>


#define MOTOR_ESC_MAX_PWM   2000    // 2.0ms
#define MOTOR_ESC_MIN_PWM   1200    // 1.2ms
#define MOTOR_ESC_STOP_PWM  900     // 0.9ms

PWMDriver       *pwmMotorDriver             = &PWMD3;
motor_power_t   motor_powers[MOTOR_COUNT];

bool            motors_armed          = false;

bool motor_control_is_armed( void )
{
    return motors_armed;
}

static inline uint16_t power_2_PWM( motor_power_t power )
{
    return( power + MOTOR_ESC_MIN_PWM );
}

static void set_motor_PWM( motor_num_t i_motor, uint16_t pwm_value )
{
    pwmEnableChannel( pwmMotorDriver, i_motor, pwm_value );
}

void motor_control_set_motors_started( void )
{
    motors_armed         = true;

    set_motor_PWM( MOTOR_1, MOTOR_ESC_MIN_PWM );
    set_motor_PWM( MOTOR_2, MOTOR_ESC_MIN_PWM );
    set_motor_PWM( MOTOR_3, MOTOR_ESC_MIN_PWM );
    set_motor_PWM( MOTOR_4, MOTOR_ESC_MIN_PWM );
}

void motor_control_set_motors_stopped( void )
{
    motors_armed         = false;
    
    set_motor_PWM( MOTOR_1, MOTOR_ESC_STOP_PWM );
    set_motor_PWM( MOTOR_2, MOTOR_ESC_STOP_PWM );
    set_motor_PWM( MOTOR_3, MOTOR_ESC_STOP_PWM );
    set_motor_PWM( MOTOR_4, MOTOR_ESC_STOP_PWM );
}

static const PWMConfig pwm_conf = {
    .frequency = 1000000,   /* 1MHz */
    .period    = 4000,      /* 4 ms ~ 250 Hz */
    .callback  = NULL,
    .channels  = {
                  {PWM_OUTPUT_ACTIVE_HIGH, NULL},
                  {PWM_OUTPUT_ACTIVE_HIGH, NULL},
                  {PWM_OUTPUT_ACTIVE_HIGH, NULL},
                  {PWM_OUTPUT_ACTIVE_HIGH, NULL}
                  },
    .cr2        = 0,
    .dier       = 0
};

BSEMAPHORE_DECL(mc_lock, false);

void motor_control_lock( void )
{
    chBSemWait( &mc_lock );
}

void motor_control_unlock( void )
{
    chBSemSignal( &mc_lock );
}

void motor_control_update_PWM( void )
{
    if ( !motors_armed )
        return;
    
    motor_num_t     m_idx;
    motor_power_t   pwr;

    motor_control_lock();
    
    for ( m_idx = MOTOR_1; m_idx < MOTOR_COUNT; m_idx++ ) 
    {
        pwr = motor_powers[m_idx] < 0 ? MOTOR_ESC_STOP_PWM : 
                clip_value( motor_powers[m_idx], MOTOR_INPUT_MIN, MOTOR_INPUT_MAX );

        set_motor_PWM( m_idx, power_2_PWM( pwr ) );   
    }

    motor_control_unlock();
}

motor_power_t *motor_control_get_powers_ptr( void )
{
    return motor_powers;
}

void motor_control_init( void )
{
    palSetPadMode( GPIOA, 6, PAL_MODE_ALTERNATE(2) );
    palSetPadMode( GPIOC, 7, PAL_MODE_ALTERNATE(2) );
    palSetPadMode( GPIOC, 8, PAL_MODE_ALTERNATE(2) );
    palSetPadMode( GPIOC, 9, PAL_MODE_ALTERNATE(2) );

    pwmStart( pwmMotorDriver, &pwm_conf );

    motor_control_set_motors_stopped();
}

