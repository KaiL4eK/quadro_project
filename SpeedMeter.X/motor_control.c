#include "motor_control.h"

/********************************/
/*              PWM             */
/********************************/

#define PWM_PRESCALE    4
#define PWM_FREQ        400
#define PWM_PERIOD      ((FCY/PWM_FREQ/PWM_PRESCALE) - 1)   // 15 bytes | max = 32767 | 32 MHz = 9999
#define PWM_USEC(x)     ((FCY/1000000L)*(x)/PWM_PRESCALE - 1)

// As DC comparison happens with 15-1 bits so it is needed to make shift for 1 bit left / *2
static const uint16_t   esc_max_power = PWM_USEC(1900),
                        esc_min_power = PWM_USEC(1200),
                        esc_stop_power = PWM_USEC(900);

static uint8_t  motor1_armed = 0,
                motor2_armed = 0,
                motor3_armed = 0,
                motor4_armed = 0;

void set_motors_started( uint8_t motor_nums )
{
    if ( motor_nums & MOTOR_1 )
    {
        motor1_armed = 1;
        PDC1 = esc_min_power << 1;
    }
    if ( motor_nums & MOTOR_2 )
    {
        motor2_armed = 1;
        PDC2 = esc_min_power << 1;
    }
    if ( motor_nums & MOTOR_3 )
    {
        motor3_armed = 1;
        PDC3 = esc_min_power << 1;
    }
    if ( motor_nums & MOTOR_4 )
    {
        motor4_armed = 1;
        PDC4 = esc_min_power << 1;
    }
}

void set_motors_stopped()
{
    motor1_armed = motor2_armed = motor3_armed = motor4_armed = 0;
    PDC1 = PDC2 = PDC3 = PDC4 = esc_stop_power << 1;
}

static uint16_t 
map_power( uint32_t val )
{
    return( val * (esc_max_power - esc_min_power)/INPUT_POWER_MAX + esc_min_power );
}

#define check_input_power( power ) power > INPUT_POWER_MAX ? INPUT_POWER_MAX : power < INPUT_POWER_MIN ? INPUT_POWER_MIN : power

// Always shift duty cycle << 1 if prescaler not 1:1
void set_motor1_power( int16_t power ) //Power [0 --- 10000]
{
    if ( !motor1_armed )
        return;
    uint16_t input_power = check_input_power( power );
    uint16_t mapped_power = map_power( input_power );
    PDC1 = mapped_power << 1;
}

void set_motor2_power( int16_t power ) //Power [0 --- 10000]
{
    if ( !motor2_armed )
        return;
    uint16_t input_power = check_input_power( power );    
    uint16_t mapped_power = map_power( input_power );
    PDC2 = mapped_power << 1;
}

void set_motor3_power( int16_t power ) //Power [0 --- 10000]
{
    if ( !motor3_armed)
        return;
    uint16_t input_power = check_input_power( power );
    uint16_t mapped_power = map_power( input_power );
    PDC3 = mapped_power << 1;
}

void set_motor4_power( int16_t power ) //Power [0 --- 10000]
{
    if ( !motor4_armed )
        return;
    uint16_t input_power = check_input_power( power );
    uint16_t mapped_power = map_power( input_power );
    PDC4 = mapped_power << 1;
}

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
    PDC1 = PDC2 = PDC3 = PDC4 = esc_stop_power << 1;
    P1TCONbits.PTEN = 1; 
}
