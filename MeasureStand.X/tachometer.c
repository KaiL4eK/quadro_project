#include <stdint.h>
#include "tachometer.h"
#include "per_proto.h"

uint32_t    timer_divider = 0;
int         stop_flag = 1,
            started_flag = 0;

void tacho_init ( void )
{
    _TRISD12 = 1;
    IC5CONbits.ICM = IC_CE_MODE_DISABLED;
    IC5CONbits.ICTMR = IC_TIMER_2;
    IC5CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC5CONbits.ICM = IC_CE_MODE_RISING_EDGE;
    _IC5IP = 7;     //Priority 7
    _IC5IF = 0;     // Zero interrupt flag
    _IC5IE = 1;     // Enable interrupt
    
    T2CONbits.TCKPS = TIMER_DIV_64;
    timer_divider = 64;
    
    T2CONbits.TON = 0;
    TMR2 = 0;
    PR2 = UINT16_MAX;
    T2CONbits.TON = 1;
}

int16_t round_speed = -1;

void tacho_start_cmd ( void )
{
    started_flag = 1;
}

void tacho_stop_cmd ( void )
{
    stop_flag = 1;
    started_flag = 0;
}

void __attribute__( (__interrupt__, auto_psv) ) _IC5Interrupt() 
{
    TMR2 = 0;
    if ( started_flag )
    {
        if ( !stop_flag )
        {
            uint32_t half_round_time = convert_ticks_to_us( IC5BUF, timer_divider ); // last is timer scale coeff
            round_speed = 30000000L/half_round_time;
        }
        else
        {
            uint16_t tmp_garbage = IC5BUF;
            stop_flag = 0;
        }
    }

    _IC5IF = 0;
}

int16_t tacho_get_round_speed ( void )
{
    int16_t cur_round_speed = round_speed;
    round_speed = -1;
    return( cur_round_speed );
}
