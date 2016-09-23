#include "core.h"

uint32_t    timer_divider = 0;
bool        dummy_read = true,
            started_flag = false;

void tacho_init ( void )
{
    _TRISD12 = 1;
    IC5CONbits.ICM = IC_CE_MODE_DISABLED;
    IC5CONbits.ICTMR = IC_TIMER_2;
    IC5CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC5CONbits.ICM = IC_CE_MODE_FALLING_EDGE;
    _IC5IP = INT_PRIO_HIGHEST;
    _IC5IF = 0;
    _IC5IE = 1;
    
    T2CONbits.TCKPS = TIMER_DIV_64;
    timer_divider = 64;
    
    T2CONbits.TON = 0;
    TMR2 = 0;
    PR2 = UINT16_MAX;
    T2CONbits.TON = 1;
}

uint16_t round_speed = 0;

void tacho_start_cmd ( void )
{
    round_speed = 0;
    started_flag = true;
    dummy_read = true;
    while ( IC5CONbits.ICBNE ) {
        uint16_t tmp_garbage = IC5BUF;
    }
    
    _IC5IE = 1;
}

void tacho_stop_cmd ( void )
{
    dummy_read = true;
    started_flag = false;
    _IC5IE = 0;
}

void __attribute__( (__interrupt__, auto_psv) ) _IC5Interrupt() 
{
    TMR2 = 0; 
    if ( started_flag )
    {
        if ( !dummy_read )
        {
            uint32_t half_round_time_us = convert_ticks_to_us( IC5BUF, timer_divider ); // last is timer scale coeff
//            UART_write_string( UARTm1, "%ld\n", half_round_time_us );
            round_speed = 30000000L/half_round_time_us;    // rpm
        } else {
            uint16_t tmp_garbage = IC5BUF;
            dummy_read = false;
        }
    }

    _IC5IF = 0;
}

uint16_t tacho_get_round_speed ( void )
{
    return( round_speed );
}
