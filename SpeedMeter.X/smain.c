#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xc.h>
#include "per_proto.h"
#include "motor_control.h"

_FOSCSEL(FNOSC_PRI & IESO_OFF);
_FOSC(POSCMD_HS & OSCIOFNC_OFF & FCKSM_CSECMD);
_FWDT(FWDTEN_OFF);              // Watchdog Timer Enabled/disabled by user software

#define FOSC        32000000ULL
#define FCY         (FOSC/2)
long long fcy() { return( FCY ); }

static void process_UART_input_command2( uint8_t input );
void init_input_capture();

int main ( void ) {
    OFF_WATCH_DOG_TIMER;
    OFF_ALL_ANALOG_INPUTS;
    
    init_UART1( 57600 );
    UART_write_string("UART initialized\r\n");
    motors_init();
    delay_ms(1000);
    init_input_capture();
    
    while ( 1 ) 
    { 
        process_UART_input_command2( UART_get_last_received_command() );
    }
    
    return( 0 );
}

uint16_t    test_throttle = 0;
uint64_t    time_sum = 0;
#define THROTTLE_MAX    2000L    //[0 --- 2000]
#define THROTTLE_MIN    0L
#define THROTTLE_STEP   (INPUT_POWER_MAX/20)
static void process_UART_input_command2( uint8_t input )
{
    switch ( input )
    {
        case 'w':
            set_motors_started( MOTOR_4 );
            test_throttle = 0; // Here we set stable value
            set_motor4_power( test_throttle );
            TMR2 = 0;
            time_sum = 0;
            break;
        case 's':
            set_motors_stopped();
            init_input_capture();
            break;
        default:
            return;
    }
//    UART_write_string( "P: %d\n\r", test_throttle );
}

uint32_t    timer_divider = 0,
            next_timer_divider = 0;

uint8_t     first_check = 0;

void init_input_capture()
{
    _TRISD12 = 1;
    IC5CONbits.ICM = IC_CE_MODE_DISABLED;
    IC5CONbits.ICTMR = IC_TIMER_2;
    IC5CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC5CONbits.ICM = IC_CE_MODE_RISING_EDGE;
    _IC5IP = 7;     //Priority 7
    _IC5IF = 0;     // Zero interrupt flag
    _IC5IE = 1;     // Enable interrupt
    
    T2CONbits.TON = 0;
    T2CONbits.TCKPS = TIMER_DIV_64;
    timer_divider = next_timer_divider = 64;
    TMR2 = 0;
    PR2 = UINT16_MAX;
    T2CONbits.TON = 1;
    
    first_check = 1;
}

void set_next_timer_divider()
{
    switch ( next_timer_divider )
    {
        case 1:
           T2CONbits.TCKPS = TIMER_DIV_1;
           break;
        case 8:
           T2CONbits.TCKPS = TIMER_DIV_8;
           break;
    }
    timer_divider = next_timer_divider;
}

void __attribute__( (__interrupt__, auto_psv) ) _IC5Interrupt() 
{
    uint32_t half_round_time = 0;
    uint32_t current_timer_divider = timer_divider;
    if ( next_timer_divider != timer_divider )
    {
        set_next_timer_divider();
    }
    TMR2 = 0;
    
    if ( !first_check )
    {
        half_round_time = IC5BUF*current_timer_divider/TIMER_US_TICK; // last is timer scale coeff
        time_sum += half_round_time;
    
//        if ( timer_divider > 1 && half_round_time < 1L*UINT16_MAX*TIMER_US_TICK )
//        {
//            next_timer_divider = 1;
//        }
//        else if ( timer_divider > 8 && half_round_time < 8L*UINT16_MAX*TIMER_US_TICK )
//        {
//            next_timer_divider = 8;
//        }
        
        UART_write_string("%ld, %lld\r\n", 30*1000000L/half_round_time, time_sum); // 1000 cause we count in msecs
    }
    else
    {
        uint16_t tmp_garbage = IC5BUF;
        first_check = 0;
    }
 
    _IC5IF = 0;
}
