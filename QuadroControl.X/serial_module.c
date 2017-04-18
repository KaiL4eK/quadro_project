/*
 * File:   serial_module.c
 * Author: alex
 *
 * Created on February 22, 2017, 2:06 PM
 */

#include "core.h"


// p = p0*exp(-0.0341593/(t+273)*h)
// h = ln(p0/p) * (t+273)/0.0341593

/********** COMMUNICATION FUNCTIONS **********/
#ifdef INTERFACE_COMMUNICATION

#include "serial_protocol.h"

volatile bool         dataSend                  = false;
volatile uint8_t      send_timer_divider_count  = 0;
volatile uint16_t     time_tick_2ms5_count      = 0;

void process_UART_frame( void )
{
    UART_frame_t *frame = cmdProcessor_rcvFrame();
    switch ( frame->command )
    {
        case NO_COMMAND:
        case UNKNOWN_COMMAND:
            break;
        case CONNECT:
            cmdProcessor_write_cmd( UART_DATA, RESPONSE_PREFIX, RESP_NOERROR );
            dataSend = false;
            stop_motors = true;
            UART_write_string( uart_debug, "Connect\n" );
            break;
        case DISCONNECT:
            dataSend = false;
            stop_motors = true;
            UART_write_string( uart_debug, "Disconnect\n" );
            break;
        case DATA_START:
            time_tick_2ms5_count = 0;
            send_timer_divider_count = 0;
            dataSend = true;
            UART_write_string( uart_debug, "DStart\n" );
            break;
        case DATA_STOP:
            dataSend = false;
            UART_write_string( uart_debug, "DStop\n" );
            break;
        case MOTOR_START:
            start_motors = true;
            UART_write_string( uart_debug, "MStart\n" );
            break;
        case MOTOR_STOP:
            stop_motors = true;
            UART_write_string( uart_debug, "MStop\n" );
            break;
        case MOTOR_SET_POWER:
            motorPower = frame->motorPower * INPUT_POWER_MAX / 100L;
            UART_write_string( uart_debug, "MSetPower\n" );
            break;
    }
}

#define DIRECT_LINK

#define POWER_2_PERCENT(x) ( (uint8_t)((x)*100L/INPUT_POWER_MAX) )

void process_sending_UART_data( void )
{
    if ( dataSend && ++send_timer_divider_count == 12 )
    {
#ifdef DIRECT_LINK
        uint16_t sendBuffer[DATA_FULL_FRAME_SIZE/2];
#else
        uint16_t sendBuffer[DATA_QUADRO_FRAME_SIZE/2];
#endif
        // angle * 100
        sendBuffer[0] = quadrotor_state.roll;
        sendBuffer[1] = quadrotor_state.pitch;
        sendBuffer[2] = time_tick_2ms5_count;
        sendBuffer[3] = (POWER_2_PERCENT(quadrotor_state.motor_power[0]) << 8) | POWER_2_PERCENT(quadrotor_state.motor_power[1]);
        sendBuffer[4] = (POWER_2_PERCENT(quadrotor_state.motor_power[2]) << 8) | POWER_2_PERCENT(quadrotor_state.motor_power[3]);
        
        UART_write_byte( UART_DATA, DATA_PREFIX );
#ifdef DIRECT_LINK
        UART_write_words( UART_DATA, sendBuffer, DATA_FULL_FRAME_SIZE/2 );
#else
        UART_write_words( UART_DATA, sendBuffer, DATA_QUADRO_FRAME_SIZE/2 );
#endif  
        if ( ++time_tick_2ms5_count == UINT16_MAX )
        {
            cmdProcessor_write_cmd( UART_DATA, RESPONSE_PREFIX, RESP_ENDDATA );
            UART_write_string( uart_debug, "DStop1\n" );
            dataSend = false;
            start_motors = false;
        }
        send_timer_divider_count = 0;
    }
}
#endif 

#ifdef SD_CARD
static uint8_t writing_flag = 0;

#define WRITE_2_BYTE_VAL( buf, val, off ) {   int16_t num = (val); \
                                              buf[(off)]   = ((num) >> 8) & 0xff; \
                                              buf[(off)+1] = ((num)     ) & 0xff; }

#define SD_DIVIDER  1

void process_saving_data ( void )
{
    static uint8_t counter = 0;
    static uint8_t _switch = 0;

    _switch = control_values->two_pos_switch;
//    _switch = SD_write;
    
    if ( writing_flag != _switch )
    {
        writing_flag = _switch;
        if ( writing_flag )
        {
            file_open( "log%d.txt" );
            UART_write_string( uart_debug, "SD file opened\n" );
            counter = 0;
            return;
        }
        else
        {
            file_close();
            UART_write_string( uart_debug, "SD file closed\n" );
            return;
        }
    }
    
    if ( writing_flag )
    {
//        extern float        integr_sum_pitch;
//        extern float        integr_sum_roll;
        extern float        integr_sum_yaw;

        uint8_t             buffer[16];
        
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.pitch_rate,       0 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.roll_rate,        2 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.yaw_rate,         4 );
        
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.pitch,            6 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.roll,             8 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.yaw,              10 );
        
//        WRITE_2_BYTE_VAL( buffer, pitch_control,                  12 );
        WRITE_2_BYTE_VAL( buffer, yaw_control,                      12 );
        WRITE_2_BYTE_VAL( buffer, integr_sum_yaw,                   14 );
        
//        WRITE_2_BYTE_VAL( buffer, control_values->throttle,         18 );
//        WRITE_2_BYTE_VAL( buffer, pitch_setpoint,                   20 );
//        WRITE_2_BYTE_VAL( buffer, roll_setpoint,                    22 );
//        
//        WRITE_2_BYTE_VAL( buffer, quadrotor_state.motor_power[0],   24 );
//        WRITE_2_BYTE_VAL( buffer, quadrotor_state.motor_power[1],   26 );
//        WRITE_2_BYTE_VAL( buffer, quadrotor_state.motor_power[2],   28 );
//        WRITE_2_BYTE_VAL( buffer, quadrotor_state.motor_power[3],   30 );
        
        file_write( buffer, sizeof( buffer ) );
        counter = 0;
        
        uint8_t sd_load = file_get_buffers_loaded_count();
        if ( sd_load > 2 )
            UART_write_string( uart_debug, "Load SD %d\n", sd_load );
    }
}
#endif /* SD_CARD */


