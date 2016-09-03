#include "motor_control.h"

/********************************/
/*              PWM             */
/********************************/

// As DC comparison happens with 15-1 bits so it is needed to make shift for 1 bit left / *2

bool        aMotor_armed[4]    = {false, false, false, false};
uint16_t    aMotor_PWM[4];

void set_motor_PWM( uint8_t nChannel, uint16_t pwm_value )
{
    switch( nChannel ) {
        case 1:
            PDC1 = pwm_value << 1;
            break;
            
        case 2:
            PDC2 = pwm_value << 1;
            break;
            
        case 3:
            PDC3 = pwm_value << 1;
            break;
            
        case 4:
            PDC4 = pwm_value << 1;
            break;
            
        default:
            break;
    }
}

void set_motors_started( uint8_t motor_nums )
{
    if ( motor_nums & MOTOR_1 ) {
        aMotor_armed[0] = true;
        set_motor_PWM( 1, ESC_MIN_POWER );
    }
    
    if ( motor_nums & MOTOR_2 ) {
        aMotor_armed[1] = true;
        set_motor_PWM( 2, ESC_MIN_POWER );
    }
    
    if ( motor_nums & MOTOR_3 ) {
        aMotor_armed[2] = true;
        set_motor_PWM( 3, ESC_MIN_POWER );
    }
    
    if ( motor_nums & MOTOR_4 ) {
        aMotor_armed[3] = true;
        set_motor_PWM( 4, ESC_MIN_POWER );
    }
}

void set_motors_stopped()
{
    aMotor_armed[0] = aMotor_armed[1] = aMotor_armed[2] = aMotor_armed[3] = false;
    set_motor_PWM( 1, ESC_STOP_POWER );
    set_motor_PWM( 2, ESC_STOP_POWER );
    set_motor_PWM( 3, ESC_STOP_POWER );
    set_motor_PWM( 4, ESC_STOP_POWER );
}

inline uint16_t power_2_PWM( uint16_t power )
{
//    return( val * (esc_max_power - esc_min_power)/INPUT_POWER_MAX + esc_min_power );
    return( power + ESC_MIN_POWER );
}

// Always shift duty cycle << 1 if prescaler not 1:1
void set_motor_power( uint8_t nMotor, int32_t power )
{
    if ( nMotor > 4 )
        return;
    
    if ( !aMotor_armed[nMotor-1] )
        return;
    
    uint16_t input_power = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
    
    set_motor_PWM( nMotor, power_2_PWM( input_power ) );
}

#define PWM_PERIOD      9999 //((FCY/FREQ_CONTROL_SYSTEM/PWM_PRESCALE) - 1)   // 15 bytes | max = 32767

void motors_init()
{
    _TRISE0 = _TRISE2 = _TRISE4 = _TRISE6 = 0;
    _RE0 = _RE2 = _RE4 = _RE6 = 0;
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
    
    set_motors_stopped();
    
    P1TCONbits.PTEN = 1; 
}
