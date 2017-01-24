#include "core.h"
#include "MPU6050.h"
#include "remote_control.h"
#include "motor_control.h"

#include <file_io.h>

#include "pragmas.h"

void control_system_timer_init( void );
void process_UART_frame( void );

void complementary_filter_set_angle_rate( float rate_a );
void complementary_filter_set_rotation_speed_rate( float rate_a );

#define SD_CARD
#define PID_tuning
#define RC_CONTROL_ENABLED
//#define TEST_WO_MODULES

#define UART_BT     2
#define UART_PYT    1

#ifdef INTERFACE_COMMUNICATION
    #define UART_DATA   2
#endif

#ifdef ENABLE_BMP180
static float        bmp180_initial_altitude = 0.0;
static uint32_t     bmp180_press = 0,
                    bmp180_temp = 0;    // Temperature is multiplied by TEMP_MULTIPLYER
static int32_t      bmp180_altitude = 0;
#endif

static Control_values_t     *control_values = NULL;
static gyro_accel_data_t    *g_a            = NULL;
static quadrotor_state_t    quadrotor_state = {0, 0, 0, { 0, 0, 0, 0 }};
static euler_angles_t       euler_angles    = {0, 0, 0};
static struct gyro_rates { float pitch, roll, yaw; }
                            gyro_rates      = {0, 0, 0};
    
int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    INIT_ERR_L;
    ERR_LIGHT = ERR_LIGHT_ERR;
    UART_init( UART_BT, UART_460800, INT_PRIO_MID );
    
#ifdef UART_PYT
    UART_init( UART_PYT, UART_460800, INT_PRIO_HIGH );
    UART_write_set_endian( UART_PYT, UART_big_endian );
#endif
    
#ifdef INTERFACE_COMMUNICATION
    UART_init( UART_DATA, UART_115200, INT_PRIO_HIGHEST );
    UART_write_set_endian( UART_DATA, UART_big_endian );
    cmdProcessor_init( UART_DATA );
#endif
    
    UART_write_string( UART_BT, "/------------------------/\n" );
    UART_write_string( UART_BT, "UART initialized\n" );
    
    error_process_init( UART_BT );

#ifndef TEST_WO_MODULES 
    control_values = remote_control_init();
    UART_write_string( UART_BT, "RC initialized\n" );
#ifdef RC_CONTROL_ENABLED
//    while ( !remote_control_find_controller() ) {
//        UART_write_string( UART_BT, "RC search\n" );
//        delay_ms( 500 );
//    }
//    UART_write_string( UART_BT, "RC found\n" );
#endif // RC_CONTROL_ENABLED
    
//    remote_control_make_calibration( UART_BT );

#ifdef SD_CARD
    spi_init( 0 );
    UART_write_string( UART_BT, "SPI initialized\n" );
    file_io_initialize( UART_BT );
    UART_write_string( UART_BT, "SD initialized\n" );
#endif /* SD_CARD */
    i2c_init( 400000 );
    UART_write_string( UART_BT, "I2C initialized\n" );

    if ( mpu6050_init() < 0 )
        error_process( "MPU6050 initialization" );
    
    g_a = mpu6050_get_raw_data();
    mpu6050_set_bandwidth( MPU6050_DLPF_BW_42 );
    complementary_filter_set_angle_rate( 0.9f );
    complementary_filter_set_rotation_speed_rate( 0.0f );
    UART_write_string( UART_BT, "MPU6050 initialized\n" );
    
//    mpu6050_calibration( UART_BT );
    
#ifdef ENABLE_BMP180
    if ( bmp180_init( BMP085_ULTRAHIGHRES ) != 0 )
    {
        UART_write_string( UART_BT, "Failed BMP init\n" );
        error_process();
    }
    UART_write_string( UART_BT, "BMP180 initialized\n" );
    bmp180_calibrate( &bmp180_press );
    bmp180_initial_altitude = bmp180_get_altitude( bmp180_press, 101325 );
#endif
    
#ifdef ENABLE_HMC5883
    if ( hmc5883l_init( -48, -440 ) != 0 )
    {
        UART_write_string( UART_BT, "Failed HMC init\n");
        error_process();
    }
    UART_write_string( UART_BT, "HMC5883L initialized\n" );
#endif
#endif // TEST_WO_MODULES
    motor_control_init();
    UART_write_string( UART_BT, "Motors initialized\n" );
    
    control_system_timer_init();
    UART_write_string( UART_BT, "Let`s begin!\n" );
//    ERR_LIGHT = ERR_LIGHT_NO_ERR;
    
    while( 1 ) {
#ifdef SD_CARD
        file_process_tasks();
#endif

#ifdef INTERFACE_COMMUNICATION
        process_UART_frame();
#endif
    }
    
    return( 0 );
}

volatile bool   start_motors    = false,
                stop_motors     = false;

bool    SD_write    = false;

#ifdef PID_tuning

float pitch_offset                  = 0.0f;
float roll_offset                   = 0.0f;
#define OFFSET_DELTA                0.1;

static void process_UART_PID_tuning()
{
    extern PID_rates_float_t    roll_rates,
                                pitch_rates;
    
    switch ( UART_get_byte( UART_BT ) )
    {
        case 0:
            return;
        case 'Q': case 'q':
            roll_rates.prop     += 0.002;
            pitch_rates.prop    += 0.002;
            break;
        case 'W': case 'w':
            roll_rates.prop     -= 0.002;
            pitch_rates.prop    -= 0.002;
            break;
//#define INTEGR_DELTA 200
#define INTEGR_DELTA 0.00001
        case 'A': case 'a':
            roll_rates.integr   += INTEGR_DELTA;
            pitch_rates.integr  += INTEGR_DELTA;
            PID_controller_reset_integral_sums();
            break;
        case 'S': case 's':
            roll_rates.integr   -= INTEGR_DELTA;
            pitch_rates.integr  -= INTEGR_DELTA;
            PID_controller_reset_integral_sums();
            break;
        case 'Z': case 'z':
            roll_rates.diff += 0.1;
            pitch_rates.diff += 0.1;
            break;
        case 'X': case 'x':
            roll_rates.diff -= 0.1;
            pitch_rates.diff -= 0.1;
            break;
        case 'R': case 'r':
            pitch_offset += OFFSET_DELTA;
            break;
        case 'F': case 'f':
            pitch_offset -= OFFSET_DELTA;
            break;
        case 'D': case 'd':
            roll_offset += OFFSET_DELTA;
            break;
        case 'G': case 'g':
            roll_offset -= OFFSET_DELTA;
            break;
        case 'C': case 'c':
            start_motors = true;
            break;
        case 'V': case 'v':
            stop_motors = true;
            break;
        
            
        case '1':
            SD_write = !SD_write;
//            control_values->pitch = 1000;
            break;
            
        case '2':
//            control_values->pitch = -1000;
            break;
            
        case '0':
//            control_values->pitch = 0;
            break;
    }
    
//    UART_write_string( UART_BT, "P%d D%d I%d\n", pitch_rates.prop_rev, pitch_rates.diff, pitch_rates.integr_rev );
    UART_write_string( UART_BT, "P %d D %d I %ld OP %d OR %d\n", (uint16_t)(pitch_rates.prop * 1000), (uint16_t)(pitch_rates.diff * 10), (uint32_t)(pitch_rates.integr * 100000L), 
                                                                 (int16_t)(pitch_offset * 10), (int16_t)(roll_offset * 10) );
}
#endif

// p = p0*exp(-0.0341593/(t+273)*h)
// h = ln(p0/p) * (t+273)/0.0341593

/********** CONTROL SYSTEM FUNCTIONS **********/

/*** Input ***/
int16_t pitch_setpoint  = 0;
int16_t roll_setpoint   = 0;
int16_t yaw_setpoint    = 0;

int16_t pitch_adjust    = 0;
int16_t roll_adjust     = 0;

/*** Output ***/
int16_t pitch_control = 0;
int16_t roll_control  = 0;
int16_t yaw_control   = 0;

#define ANGLE_ADJUST_RATE           100

// Max angular speed = 40 deg/sec   (1000/25)
const static float CONTROL_2_ANGLE_SPEED_RATE = 1.0f/25;

void calculate_PID_controls ()
{
    pitch_control   = PID_controller_generate_pitch_control( pitch_setpoint * CONTROL_2_ANGLE_SPEED_RATE - gyro_rates.pitch, gyro_rates.pitch );

//    roll_control    = PID_controller_generate_roll_control( roll_setpoint * CONTROL_2_ANGLE_SPEED_RATE - gyro_rates.roll, gyro_rates.roll );

//    yaw_control     = PID_controller_generate_yaw_control( yaw_setpoint * CONTROL_2_ANGLE_SPEED_RATE - gyro_rates.yaw );
}

#define START_STOP_COND (   control_values->throttle < THROTTLE_START_LIMIT && \
                            control_values->rudder > (-1*START_ANGLES) && \
                            control_values->roll < START_ANGLES && \
                            control_values->pitch > (-1*START_ANGLES) )

#define MAX_CONTROL_ANGLE       10L
#define CONTROL_2_ANGLE_RATIO   10L
#define STOP_LIMIT              1000L       // 1k * 2.5 ms = 2.5 sec - low thrust limit
#define DEADZONE_LIMIT_ANGLE    30

#define CONTROL_DEADZONE(x)     ((-DEADZONE_LIMIT_ANGLE >= (x) && (x) <= DEADZONE_LIMIT_ANGLE) ? 0 : (x))

const static float control_2_angle_rate = MAX_CONTROL_ANGLE/(float)CONTROL_2_ANGLE_RATIO;

int32_t motorPower = 0;

#define TESTING_

void process_control_system ( void )
{
    static bool         motors_armed                = false;
 
    static int16_t      stop_counter                = 0;
    static bool         sticks_changed              = false;
           bool         sticks_in_start_position    = START_STOP_COND;   
#ifndef TESTING_   
    if ( sticks_in_start_position != sticks_changed )
    {   // If go from some position to corners (power on position) or from corners to some position
        sticks_changed = sticks_in_start_position;
        if ( sticks_in_start_position )
        {   // If now in corner position
            if ( !motors_armed )
            {
                PID_controller_reset_integral_sums();
                motor_control_set_motors_started();
                UART_write_string( UART_BT, "All motors started with power %d\n", motorPower );
                motors_armed = true;
            }
            start_motors = false;
        }
    }
#else
    if ( control_values->two_pos_switch == TWO_POS_SWITCH_ON )
    {
        if ( sticks_in_start_position != sticks_changed )
        {   // If go from some position to corners (power on position) or from corners to some position
            sticks_changed = sticks_in_start_position;
            if ( sticks_in_start_position )
            {   // If now in corner position
                if ( !motors_armed )
                {
                    PID_controller_reset_integral_sums();
                    motor_control_set_motors_started();
                    UART_write_string( UART_BT, "All motors started with power %d\n", motorPower );
                    motors_armed = true;
                }
                start_motors = false;
            }
        }
//        if ( start_motors && !motors_armed )
//        {
//            PID_controller_reset_integral_sums();
//            motor_control_set_motors_started();
//            UART_write_string( UART_BT, "All motors started with power %ld\n", motorPower );
//            motors_armed = true;
//        }
//        
//        if ( stop_motors && motors_armed )
//        {
//            motor_control_set_motors_stopped();
//            UART_write_string( UART_BT, "All motors stopped\n", motorPower );
//            motors_armed = false;            
//        }
    } else {
        quadrotor_state.yaw = 0;
        if ( motors_armed )
        {
            motor_control_set_motors_stopped();
            UART_write_string( UART_BT, "All motors stopped\n", motorPower );
            motors_armed = false;
        }
    }
           
//    start_motors = false;
//    stop_motors = false;
#endif
    // -1000 --- 1000
    pitch_setpoint  = CONTROL_DEADZONE( control_values->pitch );
    roll_setpoint   = CONTROL_DEADZONE( control_values->roll );
    yaw_setpoint    = CONTROL_DEADZONE( control_values->rudder );
           
    if ( motors_armed )
    {
        calculate_PID_controls();

        // 0 --- 2000
        motorPower      = control_values->throttle; // * 1.6f;   // * 32 / 20
        
        int32_t power = 0;
        power = motorPower + pitch_control - roll_control - yaw_control;
        quadrotor_state.motor_power[MOTOR_1] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower + pitch_control + roll_control + yaw_control;
        quadrotor_state.motor_power[MOTOR_2] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower - pitch_control + roll_control - yaw_control;
        quadrotor_state.motor_power[MOTOR_3] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower - pitch_control - roll_control + yaw_control;
        quadrotor_state.motor_power[MOTOR_4] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        motor_control_set_motor_powers( quadrotor_state.motor_power );
        
#ifdef RC_CONTROL_ENABLED

        if ( control_values->throttle <= THROTTLE_START_LIMIT )
        {
            if ( stop_counter++ >= STOP_LIMIT )
            {
                UART_write_string( UART_BT, "All motors stopped\n" );
                motors_armed = false;
                motor_control_set_motors_stopped();
                stop_counter = 0;
            }
        } else {
            stop_counter = 0;       
        }
#endif // RC_CONTROL_ENABLED
    } 
    else 
        memset( quadrotor_state.motor_power, 0, sizeof( quadrotor_state.motor_power ) );
}

#undef TESTING_

/******************** FILTERING API ********************/

static float complementary_filter_angle_rate_a = 0.95f;
static float complementary_filter_angle_rate_b = 0.05f;

static float complementary_filter_rotation_speed_rate_a = 0.7f;
static float complementary_filter_rotation_speed_rate_b = 0.3f;

void complementary_filter_set_angle_rate( float rate_a )
{
    if ( rate_a >= 1.0f )
        return;
    
    complementary_filter_angle_rate_a = rate_a;
    complementary_filter_angle_rate_b = 1.0f - rate_a;
}

void complementary_filter_set_rotation_speed_rate( float rate_a )
{
    if ( rate_a >= 1.0f )
        return;
    
    complementary_filter_rotation_speed_rate_a = rate_a;
    complementary_filter_rotation_speed_rate_b = 1.0f - rate_a;
}

#define SENS_TIME_MS       0.0025f          // 2500L/1000000
#define GYR_COEF           131.0f           // INT16_MAX/250 - Taken from datasheet mpu6050

const static float gyro_rate_raw_2_deg_per_sec    = 1.0f/GYR_COEF;
const static float gyro_rate_raw_2_degree         = SENS_TIME_MS/GYR_COEF;

static void compute_IMU_data( void )
{    
    float   acc_x               = g_a->value.x_accel,
            acc_y               = g_a->value.y_accel,
            acc_z               = g_a->value.z_accel;
    
    gyro_rates.pitch            = complementary_filter_rotation_speed_rate_a * gyro_rates.pitch + complementary_filter_rotation_speed_rate_b * (g_a->value.x_gyro * gyro_rate_raw_2_deg_per_sec);
    gyro_rates.roll             = complementary_filter_rotation_speed_rate_a * gyro_rates.roll  + complementary_filter_rotation_speed_rate_b * (g_a->value.y_gyro * gyro_rate_raw_2_deg_per_sec);
    gyro_rates.yaw              = complementary_filter_rotation_speed_rate_a * gyro_rates.yaw   + complementary_filter_rotation_speed_rate_b * (g_a->value.z_gyro * gyro_rate_raw_2_deg_per_sec);    
    
    float   accel_angle_roll    = 0;
    float   accel_angle_pitch   = 0;
    
    if ( acc_x != 0 && acc_y != 0 )
    {
        accel_angle_roll    = atan2(-acc_x, sqrt(acc_y*acc_y + acc_z*acc_z)) * RADIANS_TO_DEGREES;
        accel_angle_pitch   = atan2( acc_y, sqrt(acc_x*acc_x + acc_z*acc_z)) * RADIANS_TO_DEGREES;
    }
    
    euler_angles.pitch   = (complementary_filter_angle_rate_a * (g_a->value.x_gyro * gyro_rate_raw_2_degree + euler_angles.pitch)) + (complementary_filter_angle_rate_b * accel_angle_pitch);
    euler_angles.roll    = (complementary_filter_angle_rate_a * (g_a->value.y_gyro * gyro_rate_raw_2_degree + euler_angles.roll))  + (complementary_filter_angle_rate_b * accel_angle_roll);
    euler_angles.yaw     =                                       g_a->value.z_gyro * gyro_rate_raw_2_degree + euler_angles.yaw;

    euler_angles.pitch      -= pitch_offset;
    euler_angles.roll       -= roll_offset;
}

/******************** FILTERING API END ********************/

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
            UART_write_string( UART_BT, "Connect\n" );
            break;
        case DISCONNECT:
            dataSend = false;
            stop_motors = true;
            UART_write_string( UART_BT, "Disconnect\n" );
            break;
        case DATA_START:
            time_tick_2ms5_count = 0;
            send_timer_divider_count = 0;
            dataSend = true;
            UART_write_string( UART_BT, "DStart\n" );
            break;
        case DATA_STOP:
            dataSend = false;
            UART_write_string( UART_BT, "DStop\n" );
            break;
        case MOTOR_START:
            start_motors = true;
            UART_write_string( UART_BT, "MStart\n" );
            break;
        case MOTOR_STOP:
            stop_motors = true;
            UART_write_string( UART_BT, "MStop\n" );
            break;
        case MOTOR_SET_POWER:
            motorPower = frame->motorPower * INPUT_POWER_MAX / 100L;
            UART_write_string( UART_BT, "MSetPower\n" );
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
            UART_write_string( UART_BT, "DStop1\n" );
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

#define ANGLES_COEFF                100L        // each float is represented as integer *100 (2 decimals after point)

void process_saving_data ( void )
{
    static uint8_t counter = 0;
//    static uint8_t _switch = control_values->two_pos_switch;
    static uint8_t _switch = 0;

    _switch = SD_write;
    
    if ( writing_flag != _switch )
    {
        writing_flag = _switch;
        if ( writing_flag )
        {
            file_open( "log%d.txt" );
            UART_write_string( UART_BT, "SD file opened\n" );
            counter = 0;
            return;
        }
        else
        {
            file_close();
            UART_write_string( UART_BT, "SD file closed\n" );
            return;
        }
    }
    
    if ( writing_flag )
    {
        extern float        integr_sum_pitch;
        extern float        integr_sum_roll;
        extern float        integr_sum_yaw;

        uint8_t             buffer[32];
        
        WRITE_2_BYTE_VAL( buffer, gyro_rates.pitch,                 0 );
        WRITE_2_BYTE_VAL( buffer, gyro_rates.roll,                  2 );
        WRITE_2_BYTE_VAL( buffer, gyro_rates.yaw,                   4 );
        
        quadrotor_state.pitch   = euler_angles.pitch * ANGLES_COEFF;
        quadrotor_state.roll    = euler_angles.roll  * ANGLES_COEFF;
        quadrotor_state.yaw     = euler_angles.yaw   * ANGLES_COEFF;
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.pitch,            6 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.roll,             8 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.yaw,              10 );
        
        WRITE_2_BYTE_VAL( buffer, integr_sum_pitch,                 12 );
        WRITE_2_BYTE_VAL( buffer, integr_sum_roll,                  14 );
        WRITE_2_BYTE_VAL( buffer, integr_sum_yaw,                   16 );
        
        WRITE_2_BYTE_VAL( buffer, control_values->throttle,         18 );
        WRITE_2_BYTE_VAL( buffer, pitch_setpoint,                   20 );
        WRITE_2_BYTE_VAL( buffer, roll_setpoint,                    22 );
        
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.motor_power[0],   24 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.motor_power[1],   26 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.motor_power[2],   28 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.motor_power[3],   30 );
        
        file_write( buffer, sizeof( buffer ) );
        counter = 0;
        
//        UART_write_string( UART_BT, "Writed SD %d\n", file_get_buffer_load() );
    }
}
#endif /* SD_CARD */

#ifdef UART_PYT
void send_serial_data ( void )
{
    static bool    data_switch = false;
    
    uint8_t byte    = 0;
    
    if ( data_switch )
    {
#define DATA_SZ 1
        uint16_t buffer[DATA_SZ];
        
        buffer[0] = euler_angles.pitch * 100;
        
        UART_write_words( UART_PYT, buffer, DATA_SZ );
    }
    
    if ( !UART_bytes_available( UART_PYT ) )
        return;
    else
    {
        byte = UART_get_byte( UART_PYT );
        UART_write_string( UART_BT, "Check: 0x%x\n", byte );
    }
    
    if ( byte == '1' )
    {
        data_switch = !data_switch;
        UART_write_string( UART_BT, "Serial data changed state to %s\n", data_switch ? "online" : "offline" );
        UART_write_byte( UART_PYT, '0' );
    }
    

}

#endif

#ifdef ENABLE_BMP180
#define BMP180_EXP_FILTER_PART  0.03D
void bmp180_rcv_filtered_data ( void )
{
    float    tmp_altitude;
    bmp180_rcv_press_temp_data( &bmp180_press, &bmp180_temp );
    tmp_altitude = bmp180_get_altitude( bmp180_press, 101325 ) - bmp180_initial_altitude;
    bmp180_altitude = ((tmp_altitude*BMP180_EXP_FILTER_PART) + (bmp180_altitude/1000.0*(1.0-BMP180_EXP_FILTER_PART))) * TEMP_MULTIPLYER;
}
#endif

// Generates interrupt each 2.5 msec
void control_system_timer_init( void )
{
    T4CONbits.TON   = 0;
    T4CONbits.T32   = 1;
    T4CONbits.TCKPS = TIMER_DIV_1;
    _T5IP           = INT_PRIO_HIGHEST;
    _T5IE           = 1;
    PR5             = (((FCY/FREQ_CONTROL_SYSTEM) >> 16) & 0xffff);
    PR4             = ((FCY/FREQ_CONTROL_SYSTEM) & 0xffff);
    T4CONbits.TON   = 1;
}

/******************** INTERRUPT HANDLER ********************/

#define MEASURE_INT_TIME
#define CHECK_SD_LOAD

void __attribute__( (__interrupt__, no_auto_psv) ) _T5Interrupt()
{    
#ifdef MEASURE_INT_TIME
    static uint16_t max_time = 0;
    timer_start();
#endif

#ifndef MPU6050_DMP
    if ( mpu6050_receive_gyro_accel_raw_data() )
        return;
#endif
    
#ifdef ENABLE_HMC5883
    int16_t angle_deg = hmc5883l_get_yaw_angle();
#endif
    
    compute_IMU_data();
    
#ifdef ENABLE_BMP180
    bmp180_rcv_filtered_data();
    
    bmp180_altitude = log( bmp180_initial_press*1.0/bmp180_press ) * 1.0 * ((bmp180_temp+273*TEMP_MULTIPLYER) / 0.0341593F);
    UART_write_string( UART_BT, "Pressure: %ld %ld %ld\n", bmp180_altitude, bmp180_press, bmp180_temp );
#endif
    
#ifdef RC_CONTROL_ENABLED
    remote_control_update_control_values();
#endif // RC_CONTROL_ENABLED
    process_control_system();
    
//    UART_write_string( UART_BT, "Setpoints: %03d %03d %03d\n", pitch_setpoint, roll_setpoint, yaw_setpoint );
//    UART_write_string( UART_BT, "Control: %03d %03d %03d\n", control_values->pitch, control_values->roll, control_values->rudder );
//    UART_write_string( UART_BT, "Angles: %d, %d\n", quadrotor_state.roll, quadrotor_state.pitch );
    
#ifdef CHECK_SD_LOAD
    uint8_t load = file_get_buffers_loaded_count();
    if ( load >= 2 )
        UART_write_string( UART_BT, "Buffers loaded: %d\n", load );
#endif
    
    process_UART_PID_tuning();
    
#ifdef UART_PYT
    send_serial_data();
#endif
    
#ifdef SD_CARD
    process_saving_data();
#endif /* SD_CARD */
    
#ifdef INTERFACE_COMMUNICATION
    process_sending_UART_data();
#endif
    
#ifdef MEASURE_INT_TIME
    timer_stop();
    uint16_t time_elapsed_us = timer_get_us();
    uint16_t prev_max        = max_time;
    max_time = max( max_time, time_elapsed_us );
    if ( prev_max != max_time )
        UART_write_string( UART_BT, "Time: new maximum = %d\n", max_time );
#endif
    _T5IF = 0;
}
