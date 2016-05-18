#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xc.h>
#include "motor_control.h"
#include "input_signal.h"
#include "hx711.h"
#include "tachometer.h"
#include "core.h"
#include "ad7705.h"

_FOSCSEL(FNOSC_PRI & IESO_OFF);
_FOSC(POSCMD_HS & OSCIOFNC_OFF & FCKSM_CSECMD);
_FWDT(FWDTEN_OFF);              // Watchdog Timer Enabled/disabled by user software

long long fcy() { return( FCY ); }

void init_control_system_interrupt ( void );
static void process_UART_input_command2 ( uint8_t input );

int16_t tenzo_data = 0,
        current_data = 0;

int main ( void ) {
    OFF_WATCH_DOG_TIMER;
    OFF_ALL_ANALOG_INPUTS;
    
//    ADC_init( 5 );
//    init_hx711();
    spi_init(); // SPI: SDO - RG8
//                      SDI - RG7
//                      SCK - RG6
//                      CS  - RA0
    init_UART1( UART_115200 );
    UART_write_string("UART initialized\r\n");
//    motors_init();
//    tacho_init();
//    init_control_system_interrupt();
    int res = 0;
    while ( ( res = ad7705_init() ) < 0 )   // bad way =(
    {
        UART_write_string( "AD7705 initialization failed, %d\n", res );
        delay_ms( 5000 );
//        while ( 1 );
    }
    UART_write_string( "AD7705 initialized and calibrate\n" );
//    spi_set_speed( FREQ_16000K );
//    if ( init_sin_table( 1000, 1000, 1000 ) != 0 )
    if ( init_square( 0, 8000, 2000 ) != 0 )
    {
        UART_write_string( "Init signal table failed\n" );
        while(1);
    }
    
//    _TRISE6 = 1;
    
    while ( 1 )
    {
        uint32_t ad_result0 = ad7705_read_register( COMMUNICATION );
        UART_write_string( "Communication: 0x%lx\n", ad_result0 );
        uint32_t ad_result1 = ad7705_read_register( CLOCK );
        UART_write_string( "Clock: 0x%lx\n", ad_result1 );
        uint32_t ad_result2 = ad7705_read_register( SETUP );
        UART_write_string( "Setup: 0x%lx\n", ad_result2 );
        
        uint32_t data_res = ad7705_read_register( DATA );
        UART_write_string( "Data: %ld\n", data_res );
        
        delay_ms( 1000 );
//        tenzo_data = read_calibrated_tenzo_data();
//        current_data = ADC_read();
//        process_UART_input_command2( UART_get_last_received_command() );
    }
    
    return( 0 );
}

void init_control_system_interrupt( void )
{
    T4CONbits.TON = 0;
    T4CONbits.TCKPS = TIMER_DIV_1;
    PR4 = ( ( FCY/SYS_FREQ ) & 0xffff );
    _T4IP = 3; // 1 - lowest prio
    _T4IE = 1;
    _T4IF = 0;
}

void start_control_system_interrupt ( void )
{
    TMR4 = 0;
    T4CONbits.TON = 1;
}

void stop_control_system_interrupt ( void )
{
    T4CONbits.TON = 0;
}

uint16_t    throttle_power = 0;
int         input_signal_flag = 0;

//#define THROTTLE_MAX    2000L    //[0 --- 2000]
//#define THROTTLE_MIN    0L
//#define THROTTLE_STEP   (INPUT_POWER_MAX/20)

void start_system ( void )
{
    set_motors_started( MOTOR_4 );
    start_control_system_interrupt();
    tacho_start_cmd();
}

void stop_system ( void )
{
    set_motors_stopped();
    stop_control_system_interrupt();
    input_signal_flag = 0;
    tacho_stop_cmd();
}

static void process_UART_input_command2 ( uint8_t input )
{
    switch ( input )
    {
        case 'w':
            start_system();
            break;
        case 'q':
            input_signal_flag = 1;
            break;
        case 'a':
            input_signal_flag = 0;
            break;
        case 's':
            stop_system();
            break;
        default:
            return;
    }
}

#define ROTOR_DATA_COUNT (sizeof(send_rotor_array)/sizeof(send_rotor_array[0]))

void send_UART_motor_data ( uint16_t speed, int16_t thrust, uint16_t inputSignal, uint16_t current )
{
    uint16_t send_rotor_array[] = { speed, thrust < 0 ? 0 : thrust, inputSignal/*, current */};
    UART_write_words( send_rotor_array, ROTOR_DATA_COUNT );
}

void __attribute__( (__interrupt__, auto_psv) ) _T4Interrupt()
{
    uint16_t tmp_round_speed = tacho_get_round_speed();
    
    send_UART_motor_data( tmp_round_speed, tenzo_data, throttle_power, current_data );
    
    if ( input_signal_flag )
    {
        throttle_power = get_next_signal_value();
    }
    else
    {
        throttle_power = get_signal_zero_lvl();
    }
    
    set_motor4_power( throttle_power );
    
    _T4IF = 0;
}
