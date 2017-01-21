#include "core.h"
#include "MPU6050.h"
#include "remote_control.h"
#include "motor_control.h"

#include "file_io.h"

#include "pragmas.h"

void control_system_timer_init( void );
void process_UART_frame( void );

void set_complementary_filter_rate( float rate_a );

#define SD_CARD
#define PID_tuning
#define RC_CONTROL_ENABLED
//#define TEST_WO_MODULES
//#define MPU6050_DMP
        // Optimize DMP - TODO

#define UART_BT     UARTm1
#define UART_DATA   UARTm2

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
static gyro_accel_data_t    *g_a            = NULL;
static quadrotor_state_t    quadrotor_state = {0, 0, 0, { 0, 0, 0, 0 }};
    
int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    INIT_ERR_L;
    ERR_LIGHT = ERR_LIGHT_ERR;
    UART_init( UART_BT, UART_115200, INT_PRIO_MID );
    UART_init( UART_DATA, UART_115200, INT_PRIO_HIGHEST );
    
    UART_write_set_endian( UART_DATA, UART_big_endian );
    
    cmdProcessor_init( UART_DATA );
        
    UART_write_string( UART_BT, "/------------------------/\n" );
    UART_write_string( UART_BT, "UART initialized\n" );
    
    error_process_init( UART_BT );

#ifndef TEST_WO_MODULES 
#ifdef SD_CARD
    flash_read();
    file_num = flash_get( FILE_NUM );
    file_num = file_num < 0 ? 0 : file_num;
    UART_write_string( UART_BT, "Flash successfully read\n" );
#endif /* SD_CARD */

    control_values = remote_control_init();
    UART_write_string( UART_BT, "RC initialized\n" );
#ifdef RC_CONTROL_ENABLED
    remote_control_find_controller();
    UART_write_string( UART_BT, "RC found\n" );
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

#ifndef MPU6050_DMP
    if ( mpu6050_init() < 0 )
        error_process( "MPU6050 initialization" );
    
    g_a = mpu6050_get_raw_data();
    mpu6050_set_bandwidth( MPU6050_DLPF_BW_20 );
    set_complementary_filter_rate( 0.98f );
#else
    if ( mpu6050_dmp_init() < 0 )
        error_process( "MPU6050 DMP initialization" );
#endif
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
        process_UART_frame();
    }
    
    return( 0 );
}

volatile bool   start_motors    = false,
                stop_motors     = false;

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
        case 'Q':
        case 'q':
            roll_rates.prop_rev     += 0.002;
            pitch_rates.prop_rev    += 0.002;
            break;
        case 'W':
        case 'w':
            roll_rates.prop_rev     -= 0.002;
            pitch_rates.prop_rev    -= 0.002;
            break;
//#define INTEGR_DELTA 200
#define INTEGR_DELTA 0.00001

        case 'A':
        case 'a':
            roll_rates.integr_rev   += INTEGR_DELTA;
            pitch_rates.integr_rev  += INTEGR_DELTA;
            PID_controller_reset_integral_sums();
            break;
        case 'S':
        case 's':
            roll_rates.integr_rev   -= INTEGR_DELTA;
            pitch_rates.integr_rev  -= INTEGR_DELTA;
            PID_controller_reset_integral_sums();
            break;
        case 'Z':
        case 'z':
            roll_rates.diff += 0.1;
            pitch_rates.diff += 0.1;
            break;
        case 'X':
        case 'x':
            roll_rates.diff -= 0.1;
            pitch_rates.diff -= 0.1;
            break;
        case 'R':
        case 'r':
            pitch_offset += OFFSET_DELTA;
            break;
        case 'F':
        case 'f':
            pitch_offset -= OFFSET_DELTA;
            break;
        case 'D':
        case 'd':
            roll_offset += OFFSET_DELTA;
            break;
        case 'G':
        case 'g':
            roll_offset -= OFFSET_DELTA;
            break;
        case 'C':
        case 'c':
            start_motors = true;
            break;
        case 'V':
        case 'v':
            stop_motors = true;
            break;
            
        case '1':
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
    UART_write_string( UART_BT, "P %f D %f I %f OP %f OR %f\n", pitch_rates.prop_rev, pitch_rates.diff, pitch_rates.integr_rev, pitch_offset, roll_offset );

}
#endif

// p = p0*exp(-0.0341593/(t+273)*h)
// h = ln(p0/p) * (t+273)/0.0341593

/********** CONTROL SYSTEM FUNCTIONS **********/

static float    angle_pitch         = 0;
static float    angle_roll          = 0;                            

float           roll_level_adjust   = 0, 
                pitch_level_adjust  = 0;

float           angle_pitch_acc     = 0,
                angle_roll_acc      = 0;
euler_angles_t  euler_angles        = {0, 0, 0}; 


#define START_STOP_COND (   control_values->throttle < THROTTLE_START_LIMIT && \
                            control_values->rudder > (-1*START_ANGLES) && \
                            control_values->roll < START_ANGLES && \
                            control_values->pitch > (-1*START_ANGLES) )

#define MAX_CONTROL_ANGLE       10L
#define CONTROL_2_ANGLE_RATIO   10L
#define STOP_LIMIT              1000L       // 1k * 2.5 ms = 2.5 sec - low thrust limit
#define DEADZONE_LIMIT_ANGLE    16

#define CONTROL_DEADZONE(x)     ((-DEADZONE_LIMIT_ANGLE >= (x) && (x) <= DEADZONE_LIMIT_ANGLE) ? 0 : (x))

const static float control_2_angle_rate = MAX_CONTROL_ANGLE/(float)CONTROL_2_ANGLE_RATIO;

int32_t motorPower = 0;

#define TESTING_

float pid_error_temp;
float pid_i_mem_roll, pid_roll_setpoint, gyro_roll_input, pid_output_roll, pid_last_roll_d_error;
float pid_i_mem_pitch, pid_pitch_setpoint, gyro_pitch_input, pid_output_pitch, pid_last_pitch_d_error;
float pid_i_mem_yaw, pid_yaw_setpoint, gyro_yaw_input, pid_output_yaw, pid_last_yaw_d_error;

float pid_p_gain_roll = 4.62;               //Gain setting for the roll P-controller
float pid_i_gain_roll = 0.14;              //Gain setting for the roll I-controller
float pid_d_gain_roll = 64.0;              //Gain setting for the roll D-controller
int pid_max_roll = 1400;                    //Maximum output of the PID-controller (+/-)

float pid_p_gain_pitch = 4.62;  //Gain setting for the pitch P-controller.
float pid_i_gain_pitch = 0.14;  //Gain setting for the pitch I-controller.
float pid_d_gain_pitch = 64.0;  //Gain setting for the pitch D-controller.
int pid_max_pitch = 1400;          //Maximum output of the PID-controller (+/-)

float pid_p_gain_yaw = 14.2;                //Gain setting for the pitch P-controller. //4.0
float pid_i_gain_yaw = 0.07;               //Gain setting for the pitch I-controller. //0.02
float pid_d_gain_yaw = 0.0;                //Gain setting for the pitch D-controller.
int pid_max_yaw = 1400;                     //Maximum output of the PID-controller (+/-)

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
                    angle_pitch = angle_pitch_acc;
                    angle_roll  = angle_roll_acc;
                    pid_i_mem_roll          = 0;
                    pid_last_roll_d_error   = 0;
                    pid_i_mem_pitch         = 0;
                    pid_last_pitch_d_error  = 0;
                    pid_i_mem_yaw           = 0;
                    pid_last_yaw_d_error    = 0;
        
                    PID_controller_reset_integral_sums();
                    motor_control_set_motors_started();
                    UART_write_string( UART_BT, "All motors started with power %d\n", motorPower );
                    motors_armed = true;
                }
                start_motors = false;
            }
        }
    } else {
//        quadrotor_state.yaw = 0;
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
    // Each angle presents as integer, ex. 30.25 = 3025
    // control pitch [-1000 --- 1000]
    
    if ( motors_armed )
    {
#if 0
        int32_t power = 0;
        error_value_t error = 0;
        int16_t pitch_setpoint  = control_values->pitch;
        int16_t roll_setpoint   = control_values->roll;
        int16_t yaw_setpoint    = 0;
        
        motorPower = control_values->throttle * 1.6f;   // * 32 / 20
        
        pitch_setpoint = CONTROL_DEADZONE( pitch_setpoint );
        error = pitch_setpoint - quadrotor_state.pitch;
        int16_t pitch_control = PID_controller_generate_pitch_control( error );

        roll_setpoint = CONTROL_DEADZONE( roll_setpoint );
        error = roll_setpoint - quadrotor_state.roll;
        int16_t roll_control  = PID_controller_generate_roll_control( error );

        error = yaw_setpoint - quadrotor_state.yaw;
        int16_t yaw_control  = PID_controller_generate_yaw_control( error );

        power = motorPower + pitch_control - roll_control - yaw_control;
        quadrotor_state.motor_power[MOTOR_1] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower + pitch_control + roll_control + yaw_control;
        quadrotor_state.motor_power[MOTOR_2] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower - pitch_control + roll_control - yaw_control;
        quadrotor_state.motor_power[MOTOR_3] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower - pitch_control - roll_control + yaw_control;
        quadrotor_state.motor_power[MOTOR_4] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        motor_control_set_motor_powers( quadrotor_state.motor_power );
#else 
        pid_pitch_setpoint  = control_values->pitch  / 2;
        pid_roll_setpoint   = control_values->roll   / 2;
        pid_yaw_setpoint    = control_values->rudder / 2;
        
        pid_pitch_setpoint  = CONTROL_DEADZONE( pid_pitch_setpoint );
        pid_roll_setpoint   = CONTROL_DEADZONE( pid_roll_setpoint );
        pid_yaw_setpoint    = CONTROL_DEADZONE( pid_yaw_setpoint );
        
        pid_roll_setpoint   -= roll_level_adjust;
        pid_roll_setpoint   /= 3.0;
        
        pid_pitch_setpoint -= pitch_level_adjust;
        pid_pitch_setpoint /= 3.0;
        
          //Roll calculations
        pid_error_temp = gyro_roll_input - pid_roll_setpoint;
        pid_i_mem_roll += pid_i_gain_roll * pid_error_temp;
        if(pid_i_mem_roll > pid_max_roll)pid_i_mem_roll = pid_max_roll;
        else if(pid_i_mem_roll < pid_max_roll * -1)pid_i_mem_roll = pid_max_roll * -1;

        pid_output_roll = pid_p_gain_roll * pid_error_temp + pid_i_mem_roll + pid_d_gain_roll * (pid_error_temp - pid_last_roll_d_error);
        if(pid_output_roll > pid_max_roll)pid_output_roll = pid_max_roll;
        else if(pid_output_roll < pid_max_roll * -1)pid_output_roll = pid_max_roll * -1;

        pid_last_roll_d_error = pid_error_temp;

        //Pitch calculations
        pid_error_temp = gyro_pitch_input - pid_pitch_setpoint;
        pid_i_mem_pitch += pid_i_gain_pitch * pid_error_temp;
        if(pid_i_mem_pitch > pid_max_pitch)pid_i_mem_pitch = pid_max_pitch;
        else if(pid_i_mem_pitch < pid_max_pitch * -1)pid_i_mem_pitch = pid_max_pitch * -1;

        pid_output_pitch = pid_p_gain_pitch * pid_error_temp + pid_i_mem_pitch + pid_d_gain_pitch * (pid_error_temp - pid_last_pitch_d_error);
        if(pid_output_pitch > pid_max_pitch)pid_output_pitch = pid_max_pitch;
        else if(pid_output_pitch < pid_max_pitch * -1)pid_output_pitch = pid_max_pitch * -1;

        pid_last_pitch_d_error = pid_error_temp;

        //Yaw calculations
        pid_error_temp = gyro_yaw_input - pid_yaw_setpoint;
        pid_i_mem_yaw += pid_i_gain_yaw * pid_error_temp;
        if(pid_i_mem_yaw > pid_max_yaw)pid_i_mem_yaw = pid_max_yaw;
        else if(pid_i_mem_yaw < pid_max_yaw * -1)pid_i_mem_yaw = pid_max_yaw * -1;

        pid_output_yaw = pid_p_gain_yaw * pid_error_temp + pid_i_mem_yaw + pid_d_gain_yaw * (pid_error_temp - pid_last_yaw_d_error);
        if(pid_output_yaw > pid_max_yaw)pid_output_yaw = pid_max_yaw;
        else if(pid_output_yaw < pid_max_yaw * -1)pid_output_yaw = pid_max_yaw * -1;

        pid_last_yaw_d_error = pid_error_temp;
        
        int32_t power = 0;
        motorPower = control_values->throttle * 1.6f;   // * 32 / 20
        
        power = motorPower + pid_output_pitch - pid_output_roll - pid_output_yaw;
        quadrotor_state.motor_power[MOTOR_1] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower + pid_output_pitch + pid_output_roll + pid_output_yaw;
        quadrotor_state.motor_power[MOTOR_2] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower - pid_output_pitch + pid_output_roll - pid_output_yaw;
        quadrotor_state.motor_power[MOTOR_3] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower - pid_output_pitch - pid_output_roll + pid_output_yaw;
        quadrotor_state.motor_power[MOTOR_4] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        motor_control_set_motor_powers( quadrotor_state.motor_power );
        
//        UART_write_string( UART_BT, "Control: %d %d\n", (int)pid_pitch_setpoint, (int)pid_roll_setpoint );
        
#endif
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
//        }
#endif // RC_CONTROL_ENABLED
    } 
    else 
        memset( quadrotor_state.motor_power, 0, sizeof( quadrotor_state.motor_power ) );
}

#undef TESTING_

/********** COMMUNICATION FUNCTIONS **********/

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

/******************** FILTERING API ********************/

static float complementary_filter_rate_a = 0.95f;
static float complementary_filter_rate_b = 0.05f;

void set_complementary_filter_rate( float rate_a )
{
    if ( rate_a >= 1.0f )
        return;
    
    complementary_filter_rate_a = rate_a;
    complementary_filter_rate_b = 1.0f - rate_a;
}

#define SENS_TIME                   0.0025f     // 2500L/1000000
#define GYR_COEF                    131.0f      // = 65535/2/250    - Taken from datasheet mpu6050

const static float gyro_rate_raw_2_deg_per_sec    = SENS_TIME/GYR_COEF;
#if 0
static void get_euler_angles()
{    
    float   acc_x               = g_a->value.x_accel,
            acc_y               = g_a->value.y_accel,
            acc_z               = g_a->value.z_accel;
    
    float   gyr_delta_x         = g_a->value.x_gyro * gyro_rate_raw_2_deg_per_sec;
    float   gyr_delta_y         = g_a->value.y_gyro * gyro_rate_raw_2_deg_per_sec;
    float   gyr_delta_z         = g_a->value.z_gyro * gyro_rate_raw_2_deg_per_sec;
    
    float   accel_angle_roll    = 0;
    float   accel_angle_pitch   = 0;
    
    if ( acc_x != 0 && acc_y != 0 )
    {
        accel_angle_roll    = atan2(-acc_x, sqrt(acc_y*acc_y + acc_z*acc_z)) * RADIANS_TO_DEGREES;
        accel_angle_pitch   = atan2( acc_y, sqrt(acc_x*acc_x + acc_z*acc_z)) * RADIANS_TO_DEGREES;
    }
    
    euler_angles->pitch     = (complementary_filter_rate_a * (gyr_delta_x + euler_angles->pitch)) + (complementary_filter_rate_b * accel_angle_pitch);
    euler_angles->roll      = (complementary_filter_rate_a * (gyr_delta_y + euler_angles->roll))  + (complementary_filter_rate_b * accel_angle_roll);
    euler_angles->yaw       = gyr_delta_z + euler_angles->yaw;
    
    euler_angles->pitch   -= pitch_offset;
    euler_angles->roll    -= roll_offset;
}
#endif
static void process_calculations()
{
    gyro_roll_input     = (gyro_roll_input * 0.7)   + ((g_a->value.y_gyro / GYR_COEF) * 0.3);   //Gyro pid input is deg/sec.
    gyro_pitch_input    = (gyro_pitch_input * 0.7)  + ((g_a->value.x_gyro / GYR_COEF) * 0.3);   //Gyro pid input is deg/sec.
    gyro_yaw_input      = (gyro_yaw_input * 0.7)    + ((g_a->value.z_gyro / GYR_COEF) * 0.3);   //Gyro pid input is deg/sec.  
    
    angle_pitch += g_a->value.x_gyro * 0.000019084;                                    //Calculate the traveled pitch angle and add this to the angle_pitch variable.
    angle_roll  += g_a->value.y_gyro * 0.000019084;                                      //Calculate the traveled roll angle and add this to the angle_roll variable.

    float   acc_x               = g_a->value.x_accel,
            acc_y               = g_a->value.y_accel,
            acc_z               = g_a->value.z_accel;
    
    //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
//    angles->pitch   -= angles->roll * sin(g_a->value.z_gyro * 0.000001066);                  //If the IMU has yawed transfer the roll angle to the pitch angel.
//    angles->roll    += angles->pitch * sin(g_a->value.z_gyro * 0.000001066);                  //If the IMU has yawed transfer the pitch angle to the roll angel.

    //Accelerometer angle calculations
    float acc_total_vector = sqrt((acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z));       //Calculate the total accelerometer vector.

    if(abs(acc_y) < acc_total_vector) {                                         //Prevent the asin function to produce a NaN
        angle_pitch_acc = asin((float)acc_y/acc_total_vector)* 57.296;          //Calculate the pitch angle.
    }
    
    if(abs(acc_x) < acc_total_vector) {                                         //Prevent the asin function to produce a NaN
        angle_roll_acc  = asin((float)acc_x/acc_total_vector)* -57.296;          //Calculate the roll angle.
    }

    //Place the MPU-6050 spirit level and note the values in the following two lines for calibration.
    angle_pitch_acc -= 0.0;                                                     //Accelerometer calibration value for pitch.
    angle_roll_acc  -= 0.0;                                                     //Accelerometer calibration value for roll.

    angle_pitch = angle_pitch   * 0.9996 + angle_pitch_acc  * 0.0004;              //Correct the drift of the gyro pitch angle with the accelerometer pitch angle.
    angle_roll  = angle_roll    * 0.9996 + angle_roll_acc   * 0.0004;                 //Correct the drift of the gyro roll angle with the accelerometer roll angle.

    pitch_level_adjust  = angle_pitch * 15;                                     //Calculate the pitch angle correction
    roll_level_adjust   = angle_roll * 15;                                      //Calculate the roll angle correction
}

/******************** FILTERING API END ********************/

#ifdef SD_CARD
static uint8_t writing_flag = 0;

#define WRITE_4_BYTE_VAL( buf, val, off ) {   buf[(off)]   = ((val) >> 24) & 0xff; \
                                              buf[(off)+1] = ((val) >> 16) & 0xff; \
                                              buf[(off)+2] = ((val) >> 8 ) & 0xff; \
                                              buf[(off)+3] = ((val)      ) & 0xff; }

#define WRITE_2_BYTE_VAL( buf, val, off ) {   buf[(off)]   = ((val) >> 8) & 0xff; \
                                              buf[(off)+1] = ((val)     ) & 0xff; }

#define SD_DIVIDER  4

void process_saving_data ( void )
{
    static uint8_t counter = 0;
    
    if ( writing_flag != control_values->two_pos_switch )
    {
        writing_flag = control_values->two_pos_switch;
        if ( writing_flag )
        {
            char filename[16];
            sprintf( filename, "log%d.txt", file_num++ );
            file_open( filename );
            counter = 0;
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
    
    if ( writing_flag && ++counter == SD_DIVIDER )
    {
        extern float        integr_sum_pitch;
        extern float        integr_sum_roll;
        extern float        integr_sum_yaw;
        int16_t             integr_part;
        
        uint8_t             buffer[32];
        
        WRITE_2_BYTE_VAL( buffer, g_a->value.x_gyro,                0 );
        WRITE_2_BYTE_VAL( buffer, g_a->value.y_gyro,                2 );
        WRITE_2_BYTE_VAL( buffer, g_a->value.z_gyro,                4 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.pitch,            6 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.roll,             8 );
        WRITE_2_BYTE_VAL( buffer, quadrotor_state.yaw,              10 );
        integr_part = integr_sum_pitch;
        WRITE_2_BYTE_VAL( buffer, integr_part,                      12 );
        integr_part = integr_sum_roll;
        WRITE_2_BYTE_VAL( buffer, integr_part,                      14 );
        integr_part = integr_sum_yaw;
        WRITE_2_BYTE_VAL( buffer, integr_part,                      16 );
        WRITE_2_BYTE_VAL( buffer, control_values->throttle,         18 );
        WRITE_2_BYTE_VAL( buffer, control_values->pitch,            20 );
        WRITE_2_BYTE_VAL( buffer, control_values->roll,             22 );
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

/******************** INTERRUPT HANDLER ********************/

#define MEASURE_INT_TIME

void __attribute__( (__interrupt__, no_auto_psv) ) _T5Interrupt()
{    
#ifdef MEASURE_INT_TIME
    timer_start();
#endif
#ifndef TEST_WO_MODULES

#ifndef MPU6050_DMP
    if ( mpu6050_receive_gyro_accel_raw_data() )
        return;
#endif
    
#ifdef ENABLE_HMC5883
    int16_t angle_deg = hmc5883l_get_yaw_angle();
#endif
    
#ifdef MPU6050_DMP
    if ( mpu6050_dmp_packet_available() )
        mpu6050_dmp_get_euler_angles( &euler_angles );
#endif
    
//    get_euler_angles();
    process_calculations();
    
    quadrotor_state.pitch   = euler_angles.pitch * ANGLES_COEFF;
    quadrotor_state.roll    = euler_angles.roll  * ANGLES_COEFF;
    quadrotor_state.yaw     = euler_angles.yaw   * ANGLES_COEFF;
    
//    UART_write_string( UART_BT, "Angles: %02.1f, %02.1f\n", angle_pitch, angle_roll );
#ifdef ENABLE_BMP180
    bmp180_rcv_filtered_data();
    
    bmp180_altitude = log( bmp180_initial_press*1.0/bmp180_press ) * 1.0 * ((bmp180_temp+273*TEMP_MULTIPLYER) / 0.0341593F);
    UART_write_string( UART_BT, "Pressure: %ld %ld %ld\n", bmp180_altitude, bmp180_press, bmp180_temp );
#endif
    
#endif // TEST_WO_MODULES
    
#ifdef RC_CONTROL_ENABLED
    remote_control_update_control_values();
#endif // RC_CONTROL_ENABLED
    process_control_system();
    
#ifdef SD_CARD
    process_saving_data();
#endif /* SD_CARD */
    
    process_UART_PID_tuning();
//    process_sending_UART_data();
    
#ifdef MEASURE_INT_TIME
    uint16_t time_elapsed_us = convert_ticks_to_us( timer_stop(), 1 );
    UART_write_string( UART_BT, "%d\n", time_elapsed_us );
#endif
    _T5IF = 0;
}
