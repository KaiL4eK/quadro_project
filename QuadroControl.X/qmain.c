#include "core.h"

#include "remote_control.h"
#include <file_io.h>

#include "pragmas_wd.h"

/************** INFO **************/
// Timer 4-5    - Interrupt
// Timer 8-9    - Time counter (periphery library)
/**********************************/

//#define SD_CARD
#define PID_tuning
#define RC_CONTROL_ENABLED
//#define MPU_DATA_COLLECTION

#define UART_BT     1
#define UART_SERIAL 2

#if 1
#define UART_DEBUG  UART_SERIAL
#else
#define UART_DEBUG  UART_BT
#endif

#define UART_PYT    1

const  float                SAMPLE_PERIOD_S = 5.0/1000;

static Control_values_t     *control_values = NULL;
static gyro_accel_data_t    *g_a            = NULL;
static quadrotor_state_t    quadrotor_state;
static euler_angles_t       euler_angles    = { 0, 0, 0 };
static euler_angles_t       gyro_rates      = { 0, 0, 0 };
static float                gyro_sensitivity;    

volatile static uart_module_t       uart_debug      = NULL;
volatile static uart_module_t       uart_interface  = NULL;

void enable_wdt ( void )
{
    _SWDTEN = 1;
}

void clear_wdt ( void )
{
    ClrWdt();
}

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    INIT_ERR_L;
    ERR_LIGHT = ERR_LIGHT_ERR;
#if UART_DEBUG == UART_BT
    uart_debug = UART_init( UART_DEBUG, UART_115200, INT_PRIO_HIGH );
#else
    uart_debug = UART_init( UART_DEBUG, UART_460800, INT_PRIO_HIGH );
#endif 
    
#ifdef UART_PYT
    uart_interface = UART_init( UART_PYT, UART_460800, INT_PRIO_HIGH );
    UART_write_set_big_endian_mode( uart_interface, true );
#endif
    
    UART_write_string( uart_debug, "/------------------------/\n" );
    UART_write_string( uart_debug, "UART initialized\n" );
    
    error_process_init( uart_debug );
    battery_charge_initialize();
    battery_charge_set_filter_value( 0.2f );
    
    control_values = remote_control_init();
    UART_write_string( uart_debug, "RC initialized\n" );
#ifdef RC_CONTROL_ENABLED
//    while ( !remote_control_find_controller() ) {
//        UART_write_string( uart_debug, "RC search\n" );
//        delay_ms( 500 );
//    }
//    UART_write_string( uart_debug, "RC found\n" );
#endif // RC_CONTROL_ENABLED
    
//    remote_control_make_calibration( uart_debug );

#ifdef SD_CARD
    spi_init( 0 );
    UART_write_string( uart_debug, "SPI initialized\n" );
    file_io_initialize( uart_debug );
    UART_write_string( uart_debug, "SD initialized\n" );
#endif /* SD_CARD */
    i2c_init( 1, 400000 );
    UART_write_string( uart_debug, "I2C initialized\n" );
    
    if ( mpu6050_init( NULL, uart_debug ) < 0 )
        error_process( "MPU6050 initialization" );
    
    mpu6050_offsets_t mpu6050_offsets = { -3893, 302, 1656, 111, -13, -19 };     // Quadro data

    mpu6050_set_bandwidth( MPU6050_DLPF_BW_20 );
    mpu6050_set_offsets( &mpu6050_offsets );
    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_500 );
    mpu6050_set_accel_fullscale( MPU6050_ACCEL_FS_8 );
    
    g_a              = mpu6050_get_raw_data();
    gyro_sensitivity = mpu6050_get_gyro_sensitivity_rate();
    UART_write_string( uart_debug, "MPU6050 initialized\n" );
    
    madgwick_filter_set_inv_sqrt_method_manual( true );
    madgwick_filter_set_angle_rate( 0.5f );
    complementary_filter_set_angle_rate( 0.99f );
    lowpass_filter_set_velocity_rate( 0.7f );
    filter_initialize( SAMPLE_PERIOD_S );
    
//    mpu6050_calibration();
    
    motor_control_init();
    UART_write_string( uart_debug, "Motors initialized\n" );
    
    // Read battery value with low filter rate
    int i;
    for (i = 0; i < 100; i++)
        battery_charge_read_value();
    UART_write_string( uart_debug, "Battery status x10: %d V\n", battery_charge_get_voltage_x10() );
    battery_charge_set_filter_value( 0.95f );
    
    control_system_timer_init();
    enable_wdt();
    UART_write_string( uart_debug, "Let`s begin!\n" );
//    ERR_LIGHT = ERR_LIGHT_NO_ERR;
    
    while( 1 ) {
#ifdef SD_CARD
        file_process_tasks();
#endif
    }
    
    return( 0 );
}

/********** CONTROL SYSTEM FUNCTIONS **********/

/*** Input ***/
int16_t pitch_setpoint      = 0;
int16_t roll_setpoint       = 0;
int16_t yaw_setpoint        = 0;

float pitch_rate_setpoint   = 0;
float roll_rate_setpoint    = 0;
float yaw_rate_setpoint     = 0;

int16_t pitch_adjust        = 0;
int16_t roll_adjust         = 0;

/*** Output ***/
int16_t pitch_control       = 0;
int16_t roll_control        = 0;
int16_t yaw_control         = 0;

#define ANGLE_ADJUST_RATE           50.0f

// Max angular speed = 40 deg/sec   (1000/25)
const static float CONTROL_2_ANGLE_SPEED_RATE = 1.0f/25;

void calculate_PID_controls ( void )
{   
#if 0
    pitch_rate_setpoint = pitch_setpoint * CONTROL_2_ANGLE_SPEED_RATE;
#else
    pitch_rate_setpoint = (pitch_setpoint - euler_angles.pitch * ANGLE_ADJUST_RATE) * CONTROL_2_ANGLE_SPEED_RATE;
#endif
    pitch_control       = PID_controller_generate_pitch_control( pitch_rate_setpoint - gyro_rates.pitch, gyro_rates.pitch );

    
    roll_rate_setpoint  = (roll_setpoint - euler_angles.roll * ANGLE_ADJUST_RATE) * CONTROL_2_ANGLE_SPEED_RATE;
    
    roll_control        = PID_controller_generate_roll_control( roll_rate_setpoint - gyro_rates.roll, gyro_rates.roll );

    
    yaw_rate_setpoint   = yaw_setpoint * CONTROL_2_ANGLE_SPEED_RATE;
    
    yaw_control         = PID_controller_generate_yaw_control( yaw_rate_setpoint - gyro_rates.yaw );
}

#define STOP_LIMIT              1000L       // 1k * 2.5 ms = 2.5 sec - low thrust limit
#define DEADZONE_LIMIT          30

#define CONTROL_DEADZONE(x)     ((-DEADZONE_LIMIT <= (x) && (x) <= DEADZONE_LIMIT) ? 0 : (x))

int16_t motorPower = 0;

volatile bool   start_motors    = false,
                stop_motors     = false;

#define HANDS_START
//#define VOLTAGE_COMPENSATION

#define THROTLE_OFF_LIMIT      (800)
#define THROTLE_START_LIMIT    (200)
#define START_ANGLES            (-800)

#if 0
#define START_STOP_COND (   control_values->throttle < THROTTLE_START_LIMIT && \
                            control_values->rudder > (-1*START_ANGLES) && \
                            control_values->roll < START_ANGLES && \
                            control_values->pitch > (-1*START_ANGLES) )
#else
#define START_STOP_COND (   control_values->throttle < THROTLE_START_LIMIT && \
                            control_values->rudder > (-1*START_ANGLES) )
#endif

void process_control_system ( void )
{ 
    static int16_t      stop_counter                = 0;
    static bool         sticks_changed              = false;
           bool         sticks_in_start_position    = START_STOP_COND;
#ifdef VOLTAGE_COMPENSATION
    static float        voltage_rate                = 0;
#endif
    
    if ( control_values->two_pos_switch == TWO_POS_SWITCH_ON )
    {
#ifdef HANDS_START
        if ( sticks_in_start_position != sticks_changed )
        {   // If go from some position to corners (power on position) or from corners to some position
            sticks_changed = sticks_in_start_position;
            if ( sticks_in_start_position )
            {   // If now in corner position
                if ( !motor_control_is_armed() && control_values->throttle < THROTLE_START_LIMIT )
                {
                    PID_controller_reset_integral_sums();
                    motor_control_set_motors_started();
                    UART_write_string( uart_debug, "All motors started with power %d\n", motorPower );
                }
            }
        }
#else
        if ( start_motors && !motor_control_is_armed() )
        {
            PID_controller_reset_integral_sums();
            motor_control_set_motors_started();
            UART_write_string( uart_debug, "All motors started with power %ld\n", motorPower );
        }
        
        if ( stop_motors && motor_control_is_armed() )
        {
            motor_control_set_motors_stopped();
            UART_write_string( uart_debug, "All motors stopped\n", motorPower );        
        }
        
        start_motors = false;
        stop_motors = false;
#endif
    } else {
        if ( motor_control_is_armed() )
        {
            motor_control_set_motors_stopped();
            UART_write_string( uart_debug, "All motors stopped\n", motorPower );
        }
    }
    // -1000 --- 1000
           
    pitch_setpoint  = CONTROL_DEADZONE( control_values->pitch );
    roll_setpoint   = CONTROL_DEADZONE( control_values->roll );
    yaw_setpoint    = CONTROL_DEADZONE( control_values->rudder );
    
    if ( motor_control_is_armed() )
    {
        if ( control_values->throttle > THROTLE_START_LIMIT )
            calculate_PID_controls();
        else
            pitch_control = roll_control = yaw_control = 0;
            
        motorPower      = control_values->throttle;   // * 32 / 20
#ifdef VOLTAGE_COMPENSATION        
        voltage_rate    = 157.0 / battery_charge_get_voltage_x10(); // 143 * 1.1
#endif
        
        int16_t power = 0;
        
        power = motorPower + pitch_control - roll_control - yaw_control;
#ifdef VOLTAGE_COMPENSATION
        power *= voltage_rate;
#endif
        quadrotor_state.motor_power[MOTOR_1] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower + pitch_control + roll_control + yaw_control;
#ifdef VOLTAGE_COMPENSATION
        power *= voltage_rate; 
#endif
        quadrotor_state.motor_power[MOTOR_2] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower - pitch_control + roll_control - yaw_control;
#ifdef VOLTAGE_COMPENSATION
        power *= voltage_rate;
#endif
        quadrotor_state.motor_power[MOTOR_3] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

        power = motorPower - pitch_control - roll_control + yaw_control;
#ifdef VOLTAGE_COMPENSATION
        power *= voltage_rate;
#endif
        quadrotor_state.motor_power[MOTOR_4] = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );

#ifdef MPU_DATA_COLLECTION
        quadrotor_state.motor_power[MOTOR_1] =
        quadrotor_state.motor_power[MOTOR_2] =
        quadrotor_state.motor_power[MOTOR_3] =
        quadrotor_state.motor_power[MOTOR_4] = motorPower;
#endif
        
        motor_control_set_motor_powers( quadrotor_state.motor_power );
        
#ifdef HANDS_START
        if ( control_values->throttle <= THROTLE_START_LIMIT )
        {
            if ( stop_counter++ >= STOP_LIMIT )
            {
                UART_write_string( uart_debug, "All motors stopped\n" );
                motor_control_set_motors_stopped();
                stop_counter = 0;
            }
        } else {
            stop_counter = 0;       
        }
#endif
    } 
    else 
        memset( quadrotor_state.motor_power, 0, sizeof( quadrotor_state.motor_power ) );
}

#undef TESTING_

bool    SD_write        = false;

float   pitch_offset    = 0.01f;
float   roll_offset     = -0.02f;

#define SENS_TIME_MS       0.005f          // 2500L/1000000

void UART_debug_interface( uart_module_t uart )
{
    extern PID_rates_float_t    roll_rates,
                                pitch_rates,
                                yaw_rates;
    
    if ( UART_bytes_available( uart ) == 0 )
        return;
    
    switch ( UART_get_byte( uart ) )
    {
#define PROP_DELTA 0.1
        case 'Q': case 'q':
            roll_rates.prop     += PROP_DELTA;
            pitch_rates.prop    += PROP_DELTA;
            UART_write_string( uart, "R/P: P %d D %d I %d\n", (uint16_t)(pitch_rates.prop * 10), (uint16_t)(pitch_rates.diff * 10), (uint16_t)(pitch_rates.integr * 1000) );
            
            break;
        case 'W': case 'w':
            roll_rates.prop     -= PROP_DELTA;
            pitch_rates.prop    -= PROP_DELTA;
            UART_write_string( uart, "R/P: P %d D %d I %d\n", (uint16_t)(pitch_rates.prop * 10), (uint16_t)(pitch_rates.diff * 10), (uint16_t)(pitch_rates.integr * 1000) );
            
            break;
        case 'O': case 'o':
            yaw_rates.prop      += PROP_DELTA;
            UART_write_string( uart, "Yaw: P %d D %d I %d\n", (uint16_t)(yaw_rates.prop * 10), (uint16_t)(yaw_rates.diff * 10), (uint16_t)(yaw_rates.integr * 1000) );
            
            break;
        case 'P': case 'p':
            yaw_rates.prop      -= PROP_DELTA;
            UART_write_string( uart, "Yaw: P %d D %d I %d\n", (uint16_t)(yaw_rates.prop * 10), (uint16_t)(yaw_rates.diff * 10), (uint16_t)(yaw_rates.integr * 1000) );
            
            break;
#define INTEGR_DELTA 0.001
        case 'A': case 'a':
            roll_rates.integr   += INTEGR_DELTA;
            pitch_rates.integr  += INTEGR_DELTA;
//            PID_controller_reset_integral_sums();
            UART_write_string( uart, "R/P: P %d D %d I %d\n", (uint16_t)(pitch_rates.prop * 10), (uint16_t)(pitch_rates.diff * 10), (uint16_t)(pitch_rates.integr * 1000) );
            
            break;
        case 'S': case 's':
            roll_rates.integr   -= INTEGR_DELTA;
            pitch_rates.integr  -= INTEGR_DELTA;
//            PID_controller_reset_integral_sums();
            UART_write_string( uart, "R/P: P %d D %d I %d\n", (uint16_t)(pitch_rates.prop * 10), (uint16_t)(pitch_rates.diff * 10), (uint16_t)(pitch_rates.integr * 1000) );
            
            break;
        case 'K': case 'k':
            yaw_rates.integr    += INTEGR_DELTA;
            UART_write_string( uart, "Yaw: P %d D %d I %d\n", (uint16_t)(yaw_rates.prop * 10), (uint16_t)(yaw_rates.diff * 10), (uint16_t)(yaw_rates.integr * 1000) );
            
            break;
        case 'L': case 'l':
            yaw_rates.integr    -= INTEGR_DELTA;
            UART_write_string( uart, "Yaw: P %d D %d I %d\n", (uint16_t)(yaw_rates.prop * 10), (uint16_t)(yaw_rates.diff * 10), (uint16_t)(yaw_rates.integr * 1000) );
            
            break;
#define DIFF_DELTA 0.1
        case 'Z': case 'z':
            roll_rates.diff += DIFF_DELTA;
            pitch_rates.diff += DIFF_DELTA;
            UART_write_string( uart, "R/P: P %d D %d I %d\n", (uint16_t)(pitch_rates.prop * 10), (uint16_t)(pitch_rates.diff * 10), (uint16_t)(pitch_rates.integr * 1000) );
            
            break;
        case 'X': case 'x':
            roll_rates.diff -= DIFF_DELTA;
            pitch_rates.diff -= DIFF_DELTA;
            UART_write_string( uart, "R/P: P %d D %d I %d\n", (uint16_t)(pitch_rates.prop * 10), (uint16_t)(pitch_rates.diff * 10), (uint16_t)(pitch_rates.integr * 1000) );
            
            break;
#define OFFSET_DELTA                0.01;
        case 'R': case 'r':
            pitch_offset += OFFSET_DELTA;
            UART_write_string( uart, "OP %d OR %d\n", (int16_t)(pitch_offset * 100), (int16_t)(roll_offset * 100) );
            break;
        case 'F': case 'f':
            pitch_offset -= OFFSET_DELTA;
            UART_write_string( uart, "OP %d OR %d\n", (int16_t)(pitch_offset * 100), (int16_t)(roll_offset * 100) );            
            break;
        case 'D': case 'd':
            roll_offset += OFFSET_DELTA;
            UART_write_string( uart, "OP %d OR %d\n", (int16_t)(pitch_offset * 100), (int16_t)(roll_offset * 100) );
            break;
        case 'G': case 'g':
            roll_offset -= OFFSET_DELTA;
            UART_write_string( uart, "OP %d OR %d\n", (int16_t)(pitch_offset * 100), (int16_t)(roll_offset * 100) );
            break;
        case 'C': case 'c':
            start_motors = true;
            break;
        case 'V': case 'v':
            stop_motors = true;
            break;
            
        case '1':
            UART_write_string( uart, "Setpoints: %03d %03d %03d\n", pitch_setpoint, roll_setpoint, yaw_setpoint );
            return;
            
        case '2':
            UART_write_string( uart, "Control: %03d %03d %03d\n", control_values->pitch, control_values->roll, control_values->rudder );
            return;
            
        case '3':
            UART_write_string( uart, "Angles: %03d, %03d, %03d\n", quadrotor_state.roll, quadrotor_state.pitch, quadrotor_state.yaw );
            return;
            
        case '4':
            UART_write_string( uart, "Rates: %d, %d, %d\n", quadrotor_state.roll_rate, quadrotor_state.pitch_rate, quadrotor_state.yaw_rate );
            return;
            
        case '5':
            UART_write_string( uart, "R/P: P %d D %d I %d\n", (uint16_t)(pitch_rates.prop * 10), (uint16_t)(pitch_rates.diff * 10), (uint16_t)(pitch_rates.integr * 1000) );
            return;
            
        case '6':
            UART_write_string( uart, "Yaw: P %d D %d I %d\n", (uint16_t)(yaw_rates.prop * 10), (uint16_t)(yaw_rates.diff * 10), (uint16_t)(yaw_rates.integr * 1000) );
            return;
            
        case '7':
            UART_write_string( uart, "OP %d OR %d\n", (int16_t)(pitch_offset * 100), (int16_t)(roll_offset * 100) );
            return;
            
        case '0':
            UART_write_string( uart, "Battery: %d V\n", battery_charge_get_voltage_x10() );
            return;
    }
}

// Replace to distinct module
void send_data_package_quadro_state ( uart_module_t uart, quadrotor_state_t *q_state )
{
    extern int16_t      pitch_control;
    extern int16_t      roll_control;
    extern int16_t      yaw_control;
    extern int16_t      motorPower;
    extern float        integr_sum_pitch;
    extern float        integr_sum_roll;
    extern float        integr_sum_yaw;
 
    int     i = 0;
    int16_t buffer[14];

    buffer[i++] = q_state->pitch;
    buffer[i++] = q_state->roll;
    buffer[i++] = q_state->yaw;
    buffer[i++] = q_state->pitch_rate;
    buffer[i++] = q_state->roll_rate;
    buffer[i++] = q_state->yaw_rate;
    buffer[i++] = integr_sum_pitch;
    buffer[i++] = integr_sum_roll;
    buffer[i++] = integr_sum_yaw;
    buffer[i++] = pitch_control;
    buffer[i++] = roll_control;
    buffer[i++] = yaw_control;
    buffer[i++] = motorPower;
    buffer[i++] = battery_charge_get_voltage_x10();

    UART_write_words( uart, (uint16_t *)buffer, sizeof(buffer) / 2 );
}

#define MPU_GYRO_STATE_MULT_RATE     100

void send_data_package_imu_state ( uart_module_t uart, gyro_accel_data_t *imu_state )
{
    int     i = 0;
    int16_t buffer[7];

    buffer[i++] = imu_state->value.x_accel;
    buffer[i++] = imu_state->value.y_accel;
    buffer[i++] = imu_state->value.z_accel;
    buffer[i++] = imu_state->value.x_gyro * gyro_sensitivity * MPU_GYRO_STATE_MULT_RATE;
    buffer[i++] = imu_state->value.y_gyro * gyro_sensitivity * MPU_GYRO_STATE_MULT_RATE;
    buffer[i++] = imu_state->value.z_gyro * gyro_sensitivity * MPU_GYRO_STATE_MULT_RATE;
    buffer[i++] = motorPower;    // Free variable

    UART_write_words( uart, (uint16_t *)buffer, sizeof(buffer) / 2 );
}


#ifdef UART_PYT
void send_serial_data_full ( uart_module_t uart, uart_module_t debug, quadrotor_state_t *q_state )
{
    static bool         data_switch = false;
    
    uint8_t             byte    = 0;
    
    if ( data_switch )
    {
#ifndef MPU_DATA_COLLECTION
        send_data_package_quadro_state( uart, q_state );
#else
        send_data_package_imu_state( uart, g_a );
#endif
    }
    
    if ( UART_bytes_available( uart ) )
    {
        byte = UART_get_byte( uart );
        if ( byte )
            UART_write_string( debug, "Check: 0x%x\n", byte );
        
        if ( byte == '1' )
        {
            data_switch = !data_switch;
            UART_write_string( debug, "Serial data changed state to %s\n", data_switch ? "online" : "offline" );
            UART_write_byte( uart, '0' );
        }
    }
}

#endif

// Generates interrupt each 5 msec
void control_system_timer_init( void )
{
    uint32_t timer_counter_limit = FCY * SAMPLE_PERIOD_S;
    UART_write_string( uart_debug, "Timer setup: %ld\n", timer_counter_limit );
    
    T4CONbits.TON   = 0;
    T4CONbits.T32   = 1;
    T4CONbits.TCKPS = TIMER_DIV_1;
    _T5IP           = INT_PRIO_HIGHEST;
    _T5IE           = 1;
    PR5             = ((timer_counter_limit >> 16) & 0xffff);
    PR4             = (timer_counter_limit & 0xffff);
    T4CONbits.TON   = 1;
}

/******************** INTERRUPT HANDLER ********************/

#define MEASURE_INT_TIME
//#define CHECK_SD_LOAD

uint16_t            time_elapsed_us = 0;
imu_filter_input_t  filter_input;

void __attribute__( (__interrupt__, no_auto_psv) ) _T5Interrupt()
{        
#if 0
    timer_stop();
    time_elapsed_us = timer_get_us();
    timer_start();
    UART_write_string( uart_debug, "Time: interrupt = %d\n", time_elapsed_us );
#endif
    clear_wdt();
    
#ifdef MEASURE_INT_TIME
    static uint16_t max_time = 0;
    timer_start();
#endif
    mpu6050_receive_gyro_accel_raw_data();
    
    filter_input.acc_x = g_a->value.x_accel;
    filter_input.acc_y = g_a->value.y_accel;
    filter_input.acc_z = g_a->value.z_accel;
    filter_input.gyr_x = g_a->value.x_gyro * gyro_sensitivity;
    filter_input.gyr_y = g_a->value.y_gyro * gyro_sensitivity;
    filter_input.gyr_z = g_a->value.z_gyro * gyro_sensitivity;
    
//    complementary_filter_position_execute( &filter_input, &euler_angles );
    madgwick_filter_position_execute( &filter_input, &euler_angles );
    lowpass_filter_velocity_execute( &filter_input, &gyro_rates );
    
    euler_angles.pitch   -= pitch_offset;
    euler_angles.roll    -= roll_offset;

#ifdef RC_CONTROL_ENABLED
    remote_control_update_control_values();
#endif
    process_control_system();
       
#ifdef CHECK_SD_LOAD
    uint8_t load = file_get_buffers_loaded_count();
    if ( load >= 2 )
        UART_write_string( uart_debug, "Buffers loaded: %d\n", load );
#endif
    
    UART_debug_interface( uart_debug );
    
    // Set to integer values
    quadrotor_state.pitch       = euler_angles.pitch * 100;
    quadrotor_state.roll        = euler_angles.roll  * 100;
    quadrotor_state.yaw         = euler_angles.yaw   * 100;  
    quadrotor_state.pitch_rate  = gyro_rates.pitch   * 100;
    quadrotor_state.roll_rate   = gyro_rates.roll    * 100;
    quadrotor_state.yaw_rate    = gyro_rates.yaw     * 100;
    battery_charge_read_value();
    
#ifdef UART_PYT
    send_serial_data_full( uart_interface, uart_debug, &quadrotor_state );
#endif
    
#ifdef SD_CARD
    process_saving_data();
#endif /* SD_CARD */

#ifdef MEASURE_INT_TIME
    timer_stop();
    time_elapsed_us = timer_get_us();
    uint16_t prev_max        = max_time;
    max_time = max( max_time, time_elapsed_us );
    if ( prev_max != max_time )
        UART_write_string( uart_debug, "Time: new maximum = %d\n", max_time );
#endif
    _T5IF = 0;
    
}
