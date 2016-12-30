#include "core.h"
#include "MPU6050.h"
#include "BMP180.h"
#include "HMC5883L.h"
#include "remote_control.h"
#include "motor_control.h"

#include "file_io.h"

#include "pragmas.h"

void control_system_timer_init( void );
void process_UART_frame( void );

//#define SD_CARD
#define PROGRAM_INPUT
#define PID_tuning
//#define TEST_WO_MODULES
//#define MPU6050_DMP
        // Optimize DMP - TODO

#ifdef SD_CARD
static int file_num = 0;
#endif /* SD_CARD */

#ifdef ENABLE_BMP180
static float        bmp180_initial_altitude = 0.0;
static uint32_t     bmp180_press = 0,
                    bmp180_temp = 0;    // Temperature is multiplied by TEMP_MULTIPLYER
static int32_t      bmp180_altitude = 0;
#endif

static Control_values_t     *control_values = NULL;
static quadrotor_state_t    quadrotor_state = {0, 0, 0, { 0, 0, 0, 0 }};
    
int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    INIT_ERR_L;
    ERR_LIGHT = ERR_LIGHT_ERR;
    UART_init( UARTm1, UART_115200, INT_PRIO_MID );
    UART_init( UARTm2, UART_9600, INT_PRIO_HIGHEST );
    
    UART_write_set_endian( UARTm2, UART_little_endian );
    
    cmdProcessor_init( UARTm2 );
    
    UART_write_string( UARTm1, "/------------------------/\n" );
    UART_write_string( UARTm1, "UART initialized\n" );
#ifndef TEST_WO_MODULES 
#ifdef SD_CARD
    flash_read();
    file_num = flash_get( FILE_NUM );
    file_num = file_num < 0 ? 0 : file_num;
    UART_write_string( UARTm1, "Flash successfully read\n" );
#endif /* SD_CARD */

    control_values = remote_control_init();
    UART_write_string( UARTm1, "RC initialized\n" );

    remote_control_find_controller();
    UART_write_string( UARTm1, "RC found\n" );

//    remote_control_make_calibration();
//    UART_write_string( UARTm1, "RC calibrated\n");
#ifdef SD_CARD
    spi_init();
    UART_write_string( UARTm1, "SPI initialized\n" );
    init_sd_file_io();
    UART_write_string( UARTm1, "SD initialized\n" );
#endif /* SD_CARD */
    i2c_init( 400000 );
    UART_write_string( UARTm1, "I2C initialized\n" );

#ifndef MPU6050_DMP
    if ( mpu6050_init() < 0 )
        error_process( "MPU6050 initialization" );
#else
    if ( mpu6050_dmp_init() < 0 )
        error_process( "MPU6050 DMP initialization" );
#endif
    UART_write_string( UARTm1, "MPU6050 initialized\n" );
    
//    mpu6050_calibration();
    
#ifdef ENABLE_BMP180
    if ( bmp180_init( BMP085_ULTRAHIGHRES ) != 0 )
    {
        UART_write_string( UARTm1, "Failed BMP init\n" );
        error_process();
    }
    UART_write_string( UARTm1, "BMP180 initialized\n" );
    bmp180_calibrate( &bmp180_press );
    bmp180_initial_altitude = bmp180_get_altitude( bmp180_press, 101325 );
#endif
    
#ifdef ENABLE_HMC5883
    if ( hmc5883l_init( -48, -440 ) != 0 )
    {
        UART_write_string( UARTm1, "Failed HMC init\n");
        error_process();
    }
    UART_write_string( UARTm1, "HMC5883L initialized\n" );
#endif
#endif // TEST_WO_MODULES
    motor_control_init();
    UART_write_string( UARTm1, "Motors initialized\n" );
    
    control_system_timer_init();
    UART_write_string( UARTm1, "Let`s begin!\n" );
//    ERR_LIGHT = ERR_LIGHT_NO_ERR;
    
    while( 1 ) {
//        UART_write_string( UARTm1, "Hello\n" );
#ifdef SD_CARD
        file_process_tasks();
#endif
        process_UART_frame();
//        delay_ms( 500 );

    }
    
    return( 0 );
}

#ifdef PID_tuning

static void process_UART_PID_tuning()
{
    extern PID_rates_t  roll_rates,
                        pitch_rates;
    
    switch ( UART_get_byte( UARTm1 ) )
    {
        case 0:
            return;
        case 'Q':
        case 'q':
            roll_rates.prop_rev++;
            break;
        case 'W':
        case 'w':
            roll_rates.prop_rev--;
            break;
        case 'A':
        case 'a':
            roll_rates.integr_rev += 10;
            break;
        case 'S':
        case 's':
            roll_rates.integr_rev -= 10;
            break;
        case 'Z':
        case 'z':
            roll_rates.diff++;
            break;
        case 'X':
        case 'x':
            roll_rates.diff--;
            break;
        case 'E':
        case 'e':
            pitch_rates.prop_rev++;
            break;
        case 'R':
        case 'r':
            pitch_rates.prop_rev--;
            break;
        case 'D':
        case 'd':
            pitch_rates.integr_rev += 10;
            break;
        case 'F':
        case 'f':
            pitch_rates.integr_rev -= 10;
            break;
        case 'C':
        case 'c':
            pitch_rates.diff++;
            break;
        case 'V':
        case 'v':
            pitch_rates.diff--;
            break;
            
        case '1':
            control_values->pitch = 1000;
            break;
            
        case '2':
            control_values->pitch = -1000;
            break;
            
        case '0':
            control_values->pitch = 0;
            break;
    }
    UART_write_string( UARTm1, "R: P%d D%d I%d | P: P%d D%d I%d\n", roll_rates.prop_rev, roll_rates.diff, roll_rates.integr_rev, 
                                                                    pitch_rates.prop_rev, pitch_rates.diff, pitch_rates.integr_rev );
}
#endif

// p = p0*exp(-0.0341593/(t+273)*h)
// h = ln(p0/p) * (t+273)/0.0341593

#define START_STOP_COND (   control_values->throttle < THROTTLE_OFF_LIMIT && \
                            control_values->rudder > (-1*START_ANGLES) && \
                            control_values->roll < START_ANGLES && \
                            control_values->pitch > (-1*START_ANGLES) )

volatile bool   start_motors    = false,
                stop_motors     = false;

#define MAX_CONTROL_ANGLE   25L
#define CONTROL_2_ANGLE(x) ((x)/10 * MAX_CONTROL_ANGLE)
#define STOP_LIMIT          1000L   // 1k * 2.5 ms = 2.5 sec - low thrust limit

int32_t motorPower = 0;

inline void process_control_system ( void )
{
    static bool         motors_armed                = false;
#ifndef PROGRAM_INPUT   
    static int16_t      stop_counter                = 0;
    static bool         start_stop_flag             = false;
           bool         sticks_in_start_position    = START_STOP_COND;   

    if ( sticks_in_start_position != start_stop_flag )
    {   // If go from some position to corners (power on position) or from corners to some position
        start_stop_flag = sticks_in_start_position;
        if ( sticks_in_start_position )
        {   // If now in corner position
#else
#ifdef PID_tuning
    process_UART_PID_tuning();
#endif
    if ( start_motors )
    {
#endif
        if ( !motors_armed )
        {
            PID_controller_reset_integral_sums();
            motor_control_set_motors_started();
            UART_write_string( UARTm1, "All motors started with power %d\n", motorPower );
            motors_armed = true;
        }
        start_motors = false;
    }
    if ( stop_motors )
    {
        if ( motors_armed )
        {   // If motors were armed - turn them off
            motor_control_set_motors_stopped();
            UART_write_string( UARTm1, "Motors stopped\n" );
            motors_armed = false;
        }
        stop_motors = false;
    }
#ifndef PROGRAM_INPUT
        }
#endif
    // Each angle presents as integer, ex. 30.25 = 3025
    // control pitch [-1000 --- 1000]
    
    memset( quadrotor_state.motor_power, 0, sizeof( quadrotor_state.motor_power ) );
    
    if ( motors_armed )
    {
        int32_t power = 0;
        motorPower = control_values->throttle;
#ifndef PROGRAM_INPUT
        if ( control_values.throttle >= THROTTLE_OFF_LIMIT )
        {
#endif // PROGRAM_INPUT
            int32_t pitch_control = PID_controller_generate_pitch_control( CONTROL_2_ANGLE(control_values->pitch) - quadrotor_state.pitch );
            int32_t roll_control  = 0; //PID_controller_generate_roll_control( CONTROL_2_ANGLE(control_values->roll) - quadrotor_state.roll );
            
            power = motorPower + pitch_control - roll_control;
            quadrotor_state.motor_power[MOTOR_1] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
            
            power = motorPower + pitch_control + roll_control;
            quadrotor_state.motor_power[MOTOR_2] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
            
            power = motorPower - pitch_control + roll_control;
            quadrotor_state.motor_power[MOTOR_3] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
            
            power = motorPower - pitch_control - roll_control;
            quadrotor_state.motor_power[MOTOR_4] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
            
            motor_control_set_motor_powers( quadrotor_state.motor_power );
#ifndef PROGRAM_INPUT
            stop_counter = 0;
        }
        else
        {
            if ( stop_counter++ == STOP_LIMIT )
            {
                motors_armed = false;
                motor_control_set_motors_stopped();
            }
        }
#endif // PROGRAM_INPUT
    }
}

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
            cmdProcessor_write_cmd( UARTm2, RESPONSE_PREFIX, RESP_NOERROR );
            dataSend = false;
            stop_motors = true;
            UART_write_string( UARTm1, "Connect\n" );
            break;
        case DISCONNECT:
            dataSend = false;
            stop_motors = true;
            UART_write_string( UARTm1, "Disconnect\n" );
            break;
        case DATA_START:
            time_tick_2ms5_count = 0;
            send_timer_divider_count = 0;
            dataSend = true;
            UART_write_string( UARTm1, "DStart\n" );
            break;
        case DATA_STOP:
            dataSend = false;
            UART_write_string( UARTm1, "DStop\n" );
            break;
        case MOTOR_START:
            start_motors = true;
            UART_write_string( UARTm1, "MStart\n" );
            break;
        case MOTOR_STOP:
            stop_motors = true;
            UART_write_string( UARTm1, "MStop\n" );
            break;
        case MOTOR_SET_POWER:
            motorPower = frame->motorPower * INPUT_POWER_MAX / 100L;
            UART_write_string( UARTm1, "MSetPower\n" );
            break;
    }
}

//#define DIRECT_LINK

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
        
        UART_write_byte( UARTm2, DATA_PREFIX );
#ifdef DIRECT_LINK
        UART_write_words( UARTm2, sendBuffer, DATA_FULL_FRAME_SIZE/2 );
#else
        UART_write_words( UARTm2, sendBuffer, DATA_QUADRO_FRAME_SIZE/2 );
#endif  
        if ( ++time_tick_2ms5_count == UINT16_MAX )
        {
            cmdProcessor_write_cmd( UARTm2, RESPONSE_PREFIX, RESP_ENDDATA );
            UART_write_string( UARTm1, "DStop1\n" );
            dataSend = false;
            start_motors = false;
        }
        send_timer_divider_count = 0;
    }
}

#ifdef SD_CARD
static uint8_t writing_flag = 0;

#define WRITE_ALTITUDE_VAL( buf, val, off ) {   buf[(off)]   = ((val) >> 24) & 0xff; \
                                                buf[(off)+1] = ((val) >> 16) & 0xff; \
                                                buf[(off)+2] = ((val) >> 8 ) & 0xff; \
                                                buf[(off)+3] = ((val)      ) & 0xff; }

#define WRITE_TWO_BYTE_VAL( buf, val, off ) {   buf[(off)]   = ((val) >> 8) & 0xff; \
                                                buf[(off)+1] = ((val)     ) & 0xff; }

void process_saving_data ( void )
{
    if ( writing_flag != control_values.two_pos_switch )
    {
        writing_flag = control_values.two_pos_switch;
        if ( writing_flag )
        {
            char filename[16];
            sprintf( filename, "log%d.txt", file_num++ );
            file_open( filename );
            return;
        }
        else
        {
            file_close();
            flash_set( FILE_NUM, file_num );
            flash_flush();
            return;
        }
    }
    
    if ( writing_flag )
    {
        uint8_t buffer[16];
        WRITE_ALTITUDE_VAL( buffer, bmp180_altitude, 0 );
        WRITE_TWO_BYTE_VAL( buffer, curr_data_accel_gyro.value.x_gyro, 4 );
        WRITE_TWO_BYTE_VAL( buffer, curr_data_accel_gyro.value.y_gyro, 6 );
        WRITE_TWO_BYTE_VAL( buffer, curr_data_accel_gyro.value.z_gyro, 8 );
        WRITE_TWO_BYTE_VAL( buffer, curr_data_accel_gyro.value.x_accel, 10 );
        WRITE_TWO_BYTE_VAL( buffer, curr_data_accel_gyro.value.y_accel, 12 );
        WRITE_TWO_BYTE_VAL( buffer, curr_data_accel_gyro.value.z_accel, 14 );
        file_write( buffer, 16 );
    }
}
#endif /* SD_CARD */

#ifdef ENABLE_BMP180
#define BMP180_EXP_FILTER_PART  0.03D
inline void bmp180_rcv_filtered_data ( void )
{
    float    tmp_altitude;
    bmp180_rcv_press_temp_data( &bmp180_press, &bmp180_temp );
    tmp_altitude = bmp180_get_altitude( bmp180_press, 101325 ) - bmp180_initial_altitude;
    bmp180_altitude = ((tmp_altitude*BMP180_EXP_FILTER_PART) + (bmp180_altitude/1000.0*(1.0-BMP180_EXP_FILTER_PART))) * TEMP_MULTIPLYER;
}
#endif

void control_system_timer_init( void )
{
    T4CONbits.TON = 0;
    T4CONbits.T32 = 1;
    T4CONbits.TCKPS = TIMER_DIV_1;
    _T5IP = INT_PRIO_HIGHEST;
    _T5IE = 1;
    PR5 = (((FCY/FREQ_CONTROL_SYSTEM) >> 16) & 0xffff);
    PR4 = ((FCY/FREQ_CONTROL_SYSTEM) & 0xffff);
    T4CONbits.TON = 1;
}

#define ANGLES_COEFF                100L        // each float is represented as integer *100 (2 decimals after point)

//#define MEASURE_INT_TIME

void __attribute__( (__interrupt__, no_auto_psv) ) _T5Interrupt()
{
    static euler_angles_t       euler_angles    = {0, 0, 0}; 
    
#ifdef MEASURE_INT_TIME
    timer_start();
#endif
#ifndef TEST_WO_MODULES

#ifndef MPU6050_DMP
    if ( mpu6050_receive_gyro_accel_raw_data() )
        return;
#endif
    
//    send_UART_mpu6050_data( UARTm1 );
    
#ifdef ENABLE_HMC5883
    int16_t angle_deg = hmc5883l_get_yaw_angle();
#endif
    
#ifndef MPU6050_DMP
    mpu6050_get_euler_angles( &euler_angles );
#else
    if ( mpu6050_dmp_packet_available() )
        mpu6050_dmp_get_euler_angles( &euler_angles );
#endif
    
    quadrotor_state.pitch = euler_angles.pitch * ANGLES_COEFF;
    quadrotor_state.roll  = euler_angles.roll  * ANGLES_COEFF;
    
//    UART_write_string( UARTm1, "R %ld, P %ld\n", quadrotor_state.roll, quadrotor_state.pitch );
    
#ifdef ENABLE_BMP180
    bmp180_rcv_filtered_data();
    
    bmp180_altitude = log( bmp180_initial_press*1.0/bmp180_press ) * 1.0 * ((bmp180_temp+273*TEMP_MULTIPLYER) / 0.0341593F);
    UART_write_string( UARTm1, "Pressure: %ld %ld %ld\n", bmp180_altitude, bmp180_press, bmp180_temp );
#endif
    
#endif // TEST_WO_MODULES
    
    remote_control_update_control_values();
    process_control_system();
    
#ifdef SD_CARD
    process_saving_data();
#endif /* SD_CARD */
    
    process_sending_UART_data();
    
#ifdef MEASURE_INT_TIME
    uint16_t time_elapsed_us = convert_ticks_to_us( timer_stop(), 1 );
    UART_write_string( UARTm1, "%d\n", time_elapsed_us );
#endif
    _T5IF = 0;
}
