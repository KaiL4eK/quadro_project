#include "core.h"

/********************************/
/*              PWM             */
/********************************/

#define PWM_PRESCALE    16
#define PWM_FREQ        51
#define PWM_PERIOD      ((FCY/PWM_FREQ/PWM_PRESCALE) - 1)   // 15 bytes | max = 32767 | 32 MHz = 19606
#define PWM_USEC(x)     ((FCY/1000000L)*(x)/PWM_PRESCALE - 1)

/***** PITCH *****/

static const uint16_t   out_min_45_pitch = PWM_USEC(1035), //32 MHz = 1029
                        out_max_45_pitch = PWM_USEC(1875); //32 MHz = 1869

static int16_t map_angle_pitch( int16_t val )
{
    return( (val+45) * (int32_t)(out_max_45_pitch - out_min_45_pitch)/(90L) + out_min_45_pitch );
}

// Always shift duty cycle << 1 if prescaler not 1:1
void set_pitch_angle( int16_t angle ) //Angle [-45 --- 45]
{
    int16_t input_power = angle > 45 ? 45 : angle < -45 ? -45 : angle;
    int16_t mapped_power = map_angle_pitch( input_power );
    PDC2 = mapped_power << 1;
}

/***** ROLL *****/

static const uint16_t   out_min_45_roll = PWM_USEC(1236),
                        out_max_45_roll = PWM_USEC(1737);

static int16_t map_angle_roll( int16_t val )
{
    return( (val+45) * (int32_t)(out_max_45_roll - out_min_45_roll)/(90L) + out_min_45_roll );
}

// Always shift duty cycle << 1 if prescaler not 1:1
void set_roll_angle( int16_t angle ) //Angle [-45 --- 45]
{
    int16_t input_power = angle > 45 ? 45 : angle < -45 ? -45 : angle;
    int16_t mapped_power = map_angle_roll( input_power );
    PDC1 = mapped_power << 1;
}

/***** YAW *****/

static const uint16_t   out_min_45_yaw = PWM_USEC(1018),
                        out_max_45_yaw = PWM_USEC(1856);

static int16_t map_angle_yaw( int16_t val )
{
    return( (val+45) * (int32_t)(out_max_45_yaw - out_min_45_yaw)/(90L) + out_min_45_yaw );
}

// Always shift duty cycle << 1 if prescaler not 1:1
void set_yaw_angle( int16_t angle ) //Angle [-45 --- 45]
{
    int16_t input_power = angle > 45 ? 45 : angle < -45 ? -45 : angle;
    int16_t mapped_power = map_angle_yaw( input_power );
    PDC4 = mapped_power << 1;
}

/***** THROTTLE *****/

static const uint16_t   out_min_throttle = PWM_USEC(1068),
                        out_max_throttle = PWM_USEC(1907);

#define MAX_THROTTLE_POWER 100L

static int16_t map_throttle( int32_t val ) 
{
    return( val * (int32_t)(out_max_throttle - out_min_throttle)/MAX_THROTTLE_POWER + out_min_throttle );
}

// Always shift duty cycle << 1 if prescaler not 1:1
void set_throttle( int16_t power )
{
    int16_t input_power = power > MAX_THROTTLE_POWER ? MAX_THROTTLE_POWER : power < 0 ? 0 : power;
    int16_t mapped_power = map_throttle( input_power );
    PDC3 = mapped_power << 1;
}

/***** INITIALIZATION *****/

void naza_test_init()
{
    _TRISE0 = _TRISE2 = _TRISE4 = _TRISE6 = 0;
    _RE0 = _RE2 = _RE4 = _RE6 = 0;
    /* PWM period = Tcy * prescale * PTPER */
    /* PTPER = Fcy / Fpwm / prescale - 1 */
    P1TCONbits.PTCKPS = 0b10;   //<<<Prescale 1:16
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
//    set_pitch_angle( 0 );
    set_roll_angle( 0 );
    set_yaw_angle( 0 );
    set_throttle( 0 );
    P1TCONbits.PTEN = 1; 
}
