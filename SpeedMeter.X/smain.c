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
    init_input_capture();
    
    while ( 1 ) 
    { 
        process_UART_input_command2( UART_get_last_received_command() );
    }
    
    return( 0 );
}

uint16_t    test_throttle = 0;
uint32_t    time_sum = 0;
uint8_t     first_check = 1,
            stop_flag = 1;
#define THROTTLE_MAX    2000L    //[0 --- 2000]
#define THROTTLE_MIN    0L
#define THROTTLE_STEP   (INPUT_POWER_MAX/20)

void startup_init()
{
    if ( stop_flag )
    {
        set_motors_started( MOTOR_4 );
        stop_flag = 0;
    }
    set_motor4_power( test_throttle );
}

static void process_UART_input_command2( uint8_t input )
{
    switch ( input )
    {
        case '0':
            test_throttle = 0; // Here we set stable value
            startup_init();
            break;
        case '1':
            test_throttle = 1000; // Here we set stable value
            startup_init();
            break;
        case '2':
            test_throttle = 2000; // Here we set stable value
            startup_init();
            break;
        case '3':
            test_throttle = 3000; // Here we set stable value
            startup_init();
            break;
        case '4':
            test_throttle = 4000; // Here we set stable value
            startup_init();
            break;
        case '5':
            test_throttle = 5000; // Here we set stable value
            startup_init();
            break;
        case '6':
            test_throttle = 6000; // Here we set stable value
            startup_init();
            break;
        case '7':
            test_throttle = 7000; // Here we set stable value
            startup_init();
            break;
        case '8':
            test_throttle = 8000; // Here we set stable value
            startup_init();
            break;
        case 's':
            set_motors_stopped();
            init_input_capture();
            stop_flag = 1;
            first_check = 1;
            time_sum = 0;
            break;
        default:
            return;
    }
}

uint32_t    timer_divider = 0,
            next_timer_divider = 0;



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

uint16_t send_rotor_array[4];
#define SEND_ROTOR_DATA_COUNT (sizeof(send_rotor_array)/sizeof(send_rotor_array[0]))

void __attribute__( (__interrupt__, auto_psv) ) _IC5Interrupt() 
{
    uint32_t half_round_time = 0;
    uint32_t current_timer_divider = timer_divider;
    
    if ( next_timer_divider != timer_divider )
    {
        set_next_timer_divider();
    }
    
    TMR2 = 0;
    
    if ( !first_check && !stop_flag )
    {
        half_round_time = IC5BUF*current_timer_divider/TIMER_US_TICK; // last is timer scale coeff
        time_sum += half_round_time/1000;

        if ( timer_divider > 1 && half_round_time < 1L*UINT16_MAX/TIMER_US_TICK*2/3 ) //half is just for safe
        {
            next_timer_divider = 1;
        }
        else if ( timer_divider > 8 && half_round_time < 8L*UINT16_MAX/TIMER_US_TICK*2/3 )
        {
            next_timer_divider = 8;
        }

        uint32_t round_speed = 30*1000000L/half_round_time;

        send_rotor_array[0] = round_speed >> 16;
        send_rotor_array[1] = round_speed;
        send_rotor_array[2] = time_sum >> 16;
        send_rotor_array[3] = time_sum;
        
        UART_write_words( send_rotor_array, SEND_ROTOR_DATA_COUNT );
//        UART_write_string("%ld,%ld,%lld\r\n", current_timer_divider, 30*1000000L/half_round_time, time_sum); // 1000 cause we count in msecs
    }
    else
    {
        uint16_t tmp_garbage = IC5BUF;
        first_check = 0;
    }
 
    _IC5IF = 0;
}
