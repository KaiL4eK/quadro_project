#include "motor_control.h"

/********************************/
/*              PWM             */
/********************************/

// As DC comparison happens with 15-1 bits so it is needed to make shift for 1 bit left / *2

//#define USEC_2_PWM(x)   ((x)*4 - 1)     // ((FCY/1000000L)*(x)/PWM_PRESCALE - 1)
#define MOTOR_ESC_MAX_PWM   7999L // USEC_2_PWM(2000)
#define MOTOR_ESC_MIN_PWM   4799L // USEC_2_PWM(1200)
#define MOTOR_ESC_STOP_PWM  3599L // USEC_2_PWM(900)

#define MOTOR_COUNT         4

static Motor_control_t  motor_control;
static motor_power_t    *motor_powers = (motor_power_t *)&motor_control;


volatile unsigned int   *aMotor_power_reg[]   = { &PDC1, &PDC2, &PDC3, &PDC4 };
bool                    aMotor_armed[]        = { false, false, false, false };
uint16_t                aMotor_PWM[]          = { 0, 0, 0, 0 };
bool                    motors_armed          = false;

static inline uint16_t power_2_PWM( motor_power_t power )
{
//    return( val * (esc_max_power - esc_min_power)/INPUT_POWER_MAX + esc_min_power );
    return( power + MOTOR_ESC_MIN_PWM );
}

// Always shift duty cycle << 1 if prescaler not 1:1
static void set_motor_PWM( motor_num_t i_motor, uint16_t pwm_value )
{
    *aMotor_power_reg[i_motor] = pwm_value << 1;
}


void motor_control_update_PWM ( void )
{
    if ( !motors_armed )
        return;
    
    uint8_t i;
    for ( i = 0; i < MOTOR_COUNT; i++ ) 
    {
        uint16_t input_power = 0;
                
        if ( motor_powers[i] < 0 )
            input_power = MOTOR_ESC_STOP_PWM;
        else
            input_power = clip_value( motor_powers[i], INPUT_POWER_MIN, INPUT_POWER_MAX );

        set_motor_PWM( i, power_2_PWM( input_power ) );   
    }
}


void motor_control_set_motor_power( motor_num_t nMotor, motor_power_t power )
{
    if ( !aMotor_armed[nMotor] )
        return;
    
    uint16_t input_power = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
    
    set_motor_PWM( nMotor, power_2_PWM( input_power ) );
}

void motor_control_set_motor_powers( motor_power_t powers[4] )
{
    if ( !motors_armed )
        return;
    
    uint8_t i;
    for ( i = 0; i < 4; i++ ) 
    {
        uint16_t input_power = clip_value( powers[i], INPUT_POWER_MIN, INPUT_POWER_MAX );
        
        set_motor_PWM( i, power_2_PWM( input_power ) );
    }
}

bool motor_control_is_armed( void )
{
    return motors_armed;
}

void motor_control_set_motor_started( motor_num_t nMotor )
{
    motors_armed         = true;
    aMotor_armed[nMotor] = true;
    set_motor_PWM( nMotor, MOTOR_ESC_MIN_PWM );
}

void motor_control_set_motor_stopped( motor_num_t nMotor )
{
    motors_armed         = false;
    aMotor_armed[nMotor] = false;
    set_motor_PWM( nMotor, MOTOR_ESC_STOP_PWM );
}

void motor_control_set_motors_started( void )
{
    motors_armed         = true;
    aMotor_armed[0] = aMotor_armed[1] = aMotor_armed[2] = aMotor_armed[3] = true;
    set_motor_PWM( 0, MOTOR_ESC_MIN_PWM );
    set_motor_PWM( 1, MOTOR_ESC_MIN_PWM );
    set_motor_PWM( 2, MOTOR_ESC_MIN_PWM );
    set_motor_PWM( 3, MOTOR_ESC_MIN_PWM );
}

void motor_control_set_motors_stopped( void )
{
    motors_armed         = false;
    aMotor_armed[0] = aMotor_armed[1] = aMotor_armed[2] = aMotor_armed[3] = false;
    set_motor_PWM( 0, MOTOR_ESC_STOP_PWM );
    set_motor_PWM( 1, MOTOR_ESC_STOP_PWM );
    set_motor_PWM( 2, MOTOR_ESC_STOP_PWM );
    set_motor_PWM( 3, MOTOR_ESC_STOP_PWM );
}

#define PWM_PERIOD      9999 //((FCY/FREQ_CONTROL_SYSTEM/PWM_PRESCALE) - 1)   // 15 bytes | max = 32767

Motor_control_t * motor_control_init( void )
{
//    _TRISE0 = _TRISE2 = _TRISE4 = _TRISE6 = 0;
//    _RE0 = _RE2 = _RE4 = _RE6 = 0;
    
    /* PWM period = Tcy * prescale * PTPER */
    /* PTPER = Fcy / Fpwm / prescale - 1 */
    P1TCONbits.PTCKPS = 0b01;   //<<<Prescale 1:4
    P1TCONbits.PTMOD = 0;       //<<<Free running PWM output mode
    P1TMRbits.PTMR = 0;
    
    PWM1CON1bits.PMOD1 = 1; //Independent PWM output mode PWM1
    PWM1CON1bits.PEN1L = 1; //Enable PWM1L - E0
    PWM1CON1bits.PMOD2 = 1; //Independent PWM output mode PWM2
    PWM1CON1bits.PEN2L = 1; //Enable PWM1L - E2
    PWM1CON1bits.PMOD3 = 1; //Independent PWM output mode PWM3
    PWM1CON1bits.PEN3L = 1; //Enable PWM1L - E4
    PWM1CON1bits.PMOD4 = 1; //Independent PWM output mode PWM4
    PWM1CON1bits.PEN4L = 1; //Enable PWM1L - E6
    
    P1TPERbits.PTPER = PWM_PERIOD;
    
    motor_control_set_motors_stopped();
    
    P1TCONbits.PTEN = 1; 
    
    return &motor_control;
}
