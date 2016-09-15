#include "motor_control.h"

/********************************/
/*              PWM             */
/********************************/

// As DC comparison happens with 15-1 bits so it is needed to make shift for 1 bit left / *2

#define ESC_MAX_POWER   7599L // USEC_2_PWM(1900)
#define ESC_MIN_POWER   4799L // USEC_2_PWM(1200)
#define ESC_STOP_POWER  3599L // USEC_2_PWM(900)

bool        aMotor_armed = false;
uint16_t    aMotor_PWM;

void set_motor_PWM( uint16_t pwm_value )
{
    PDC1 = pwm_value << 1;
}

void set_motor_started( void )
{
    if ( aMotor_armed )
        return;
    
    aMotor_armed = true;
    set_motor_PWM( ESC_MIN_POWER );
}

void set_motor_stopped( void )
{
    aMotor_armed = false;
    set_motor_PWM( ESC_STOP_POWER );
}

inline uint16_t power_2_PWM( uint16_t power )
{
//    return( val * (esc_max_power - esc_min_power)/INPUT_POWER_MAX + esc_min_power );
    return( power + ESC_MIN_POWER );
}

// Always shift duty cycle << 1 if prescaler not 1:1
void set_motor_power( int32_t power )
{
//    if ( !aMotor_armed )
//        return;
//    
    uint16_t input_power = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
    
    set_motor_PWM( power_2_PWM( input_power ) );
}

#define PWM_PERIOD      9999 //((FCY/FREQ_CONTROL_SYSTEM/PWM_PRESCALE) - 1)   // 15 bytes | max = 32767

void motor_init()
{
    _TRISE0 = _RE0 = 0;

    /* PWM period = Tcy * prescale * PTPER */
    /* PTPER = Fcy / Fpwm / prescale - 1 */
    P1TCONbits.PTCKPS = 0b01;   //<<<Prescale 1:4
    P1TCONbits.PTMOD = 0;       //<<<Free running PWM output mode
    P1TMRbits.PTMR = 0;
    
    PWM1CON1bits.PMOD1 = 1; //Independent PWM output mode PWM1
    PWM1CON1bits.PEN1L = 1; //Enable PWM1L - E0
    
    P1TPERbits.PTPER = PWM_PERIOD;
    
    set_motor_stopped();
    
    P1TCONbits.PTEN = 1; 
}
