#include "core.h"
#include "MPU6050.h"
#include "BMP180.h"
#include "HMC5883L.h"
#include "input_control.h"
#include "motor_control.h"
#include "per_proto.h"
#include "file_io.h"

long long fcy() { return( FCY ); }

//#define SD_CARD
#define TEST_MOTOR4
#define NO_CONTROL

#ifdef FREQ_32MHZ
SWITCH_TO_32MHZ
#else

#endif

void control_system_timer_init( void )
{
    T4CONbits.TON = 0;
    T4CONbits.T32 = 1;
    T4CONbits.TCKPS = TIMER_DIV_1;
    _T5IP = 1;
    _T5IE = 1;
    PR5 = (((FCY/FSENS) >> 16) & 0xffff);
    PR4 = ((FCY/FSENS) & 0xffff);
    T4CONbits.TON = 1;
}

#ifdef SD_CARD
static int file_num = 0;
#endif /* SD_CARD */

static float        bmp180_initial_altitude = 0.0;
static uint32_t     //bmp180_initial_press = 0,
                    bmp180_press = 0,
                    bmp180_temp = 0;    // Temperature is multiplied by TEMP_MULTIPLYER
static int32_t      bmp180_altitude = 0;

int main(void) 
{
    OFF_WATCH_DOG_TIMER;
    OFF_ALL_ANALOG_INPUTS;
#ifndef FREQ_32MHZ
    setup_PLL_oscillator();
#endif
    INIT_ERR_L;
    ERR_LIGHT = ERR_LIGHT_ERR;
    init_UART1( 57600 );
    UART_write_string( "/------------------------/\n" );
    UART_write_string( "UART initialized\n" );
#ifdef SD_CARD
    flash_read();
    file_num = flash_get( FILE_NUM );
    file_num = file_num < 0 ? 0 : file_num;
#endif /* SD_CARD */
    UART_write_string( "Flash successfully read\n" );
    motors_init();
    UART_write_string( "Motors initialized\n" );
    init_input_control();
    UART_write_string( "IC initialized\n" );    
#ifndef NO_CONTROL
    ic_find_control();
    UART_write_string( "IC found\n" );
#endif /* NO_CONTROL */
//    ic_make_calibration();
//    UART_write_string("IC calibrated\n");
#ifdef SD_CARD
    spi_init();
    UART_write_string( "SPI initialized\n" );
    init_sd_file_io();
    UART_write_string( "SD initialized\n" );
#endif /* SD_CARD */
    i2c_init( 400000 );
    UART_write_string( "I2C initialized\n" );
    if ( mpu6050_init() != 0 )
    {
        UART_write_string("Failed MPU init\n");
        error_process();
    }
    UART_write_string( "MPU6050 initialized\n" );
    if ( bmp180_init( BMP085_ULTRAHIGHRES ) != 0 )
    {
        UART_write_string("Failed BMP init\n");
        error_process();
    }
    UART_write_string( "BMP180 initialized\n" );
    bmp180_calibrate( &bmp180_press );
//    bmp180_initial_press = bmp180_press;
    bmp180_initial_altitude = bmp180_get_altitude( bmp180_press, 101325 );
//    UART_write_string( "BMP180 calibrated: %ld\n", bmp180_initial_press );
    if ( hmc5883l_init() != 0 )
    {
        UART_write_string("Failed HMC init\n");
        error_process();
    }
    UART_write_string( "HMC5883L initialized\n" );
    control_system_timer_init();
    UART_write_string( "Let`s begin!\n" );
    ERR_LIGHT = ERR_LIGHT_NO_ERR;
    
    while( 1 )
    {   
        file_process_tasks();
    }
    return( 0 );
}

static uint16_t prop_k = 30, integr_k = 10000, differ_k= 3;

static void process_UART_input_command( uint8_t input )
{
    switch ( input )
    {
        case 0:
            return;
        case 'q':
            prop_k++;
            break;
        case 'w':
            prop_k--;
            break;
        case 'a':
            integr_k+=10;
            break;
        case 's':
            integr_k-=10;
            break;
        case 'z':
            differ_k++;
            break;
        case 'x':
            differ_k--;
            break;
    }
    UART_write_string( "Coeffs: %d, %d, %d\n", 
            prop_k, integr_k, differ_k );
}

#ifdef TEST_MOTOR4
uint16_t    test_throttle = 0;
#define THROTTLE_STEP   (INPUT_POWER_MAX/20)
static void process_UART_input_command2( uint8_t input )
{
    switch ( input )
    {
        case 'q':
            test_throttle = test_throttle >= INPUT_POWER_MAX ? INPUT_POWER_MAX : (test_throttle + THROTTLE_STEP);
            break;
        case 'a':
            test_throttle = test_throttle == THROTTLE_MIN ? THROTTLE_MIN : (test_throttle - THROTTLE_STEP);
            break;
        case 'w':
            set_motors_started( MOTOR_4 );
            test_throttle = 0;
            break;
        case 's':
            set_motors_stopped();
            break;
        default:
            return;
    }
    UART_write_string( "P: %d\n", test_throttle );
}
#endif /* TEST_MOTOR4 */

static Control_values_t     control_values;
static gyro_accel_data_t    curr_data_accel_gyro;
static magnetic_data_t      curr_data_mag;
static Angles_t             current_angles = { 0, 0, 0 };
static uint16_t             time_elapsed_us = 0;

// p = p0*exp(-0.0341593/(t+273)*h)
// h = ln(p0/p) * (t+273)/0.0341593

#define RADIANS_TO_DEGREES  (180/3.14159)
#define GYR_COEF            131L // = 65535/2/250
#define ANGLES_COEFF        1000L
#define SENS_TIME           2500L/1000000

static void process_counts( void )
{
    gyro_accel_data_t *c_d = &curr_data_accel_gyro;
    
    // Just for one of arguments for atan2 be not zero
    int32_t acc_x = c_d->value.x_accel == 0 ? 1 : c_d->value.x_accel,
            acc_y = c_d->value.y_accel == 0 ? 1 : c_d->value.y_accel,
            acc_z = c_d->value.z_accel;
    
    current_angles.acc_x = 
                atan2(acc_y, sqrt(acc_x*acc_x + acc_z*acc_z)) * RADIANS_TO_DEGREES * ANGLES_COEFF,
    current_angles.acc_y = 
                atan2(-1 * acc_x, sqrt(acc_y*acc_y + acc_z*acc_z)) * RADIANS_TO_DEGREES * ANGLES_COEFF;
    
    current_angles.gyr_delta_x = (c_d->value.x_gyro*ANGLES_COEFF/GYR_COEF) * SENS_TIME,
    current_angles.gyr_delta_y = (c_d->value.y_gyro*ANGLES_COEFF/GYR_COEF) * SENS_TIME,
    current_angles.gyr_delta_z = (c_d->value.z_gyro*ANGLES_COEFF/GYR_COEF) * SENS_TIME;
    
    current_angles.pitch = (0.995 * (current_angles.gyr_delta_x + current_angles.pitch)) + 0.005 * current_angles.acc_x;
    current_angles.roll  = (0.995 * (current_angles.gyr_delta_y + current_angles.roll)) + 0.005 * current_angles.acc_y;
    current_angles.yaw   = current_angles.gyr_delta_z + current_angles.yaw;  
}

#define START_STOP_COND (   control_values.throttle < THROTTLE_OFF_LIMIT && \
                            control_values.rudder > (-1*START_ANGLES) && \
                            control_values.roll < START_ANGLES && \
                            control_values.pitch > (-1*START_ANGLES) )

#define MAX_CONTROL_ANGLE   45L
#define STOP_LIMIT          1000L
#define INPUT_REG_LIMIT     1000L

inline void process_control_system ( void )
{
#ifdef TEST_MOTOR4
#ifndef NO_CONTROL
    static uint8_t power_flag = TWO_POS_SWITCH_OFF;
    if ( power_flag != control_values.two_pos_switch )
    {
        power_flag = control_values.two_pos_switch;
        if ( power_flag )
        {
            set_motors_started( MOTOR_4 );
            test_throttle = 0;
        }
        else
        {
            set_motors_stopped();
        }
    }
#endif /* NO_CONTROL */
    
//    send_UART_control_values();
    process_UART_input_command2( UART_get_last_received_command() );
//    if ( INPUT_POWER_MAX > THROTTLE_MAX )
    {
        set_motor4_power( test_throttle );
    }
    
    
#else
    static bool     start_stop_flag = false,
                    motors_armed = false;
    static int64_t  integr = 0; 
    static int32_t  diff = 0,
                    prev_pitch = 0, 
                    pitch_error = 0,
                    regul = 0;

    static int16_t  input_control_pitch = 0,
                    stop_counter = 0;
    
    bool sticks_in_start_position = START_STOP_COND;   
    
    if ( sticks_in_start_position != start_stop_flag )
    {
        start_stop_flag = sticks_in_start_position;
        if ( sticks_in_start_position )
        {
            if ( motors_armed )
            {
                set_motors_stopped();
            }
            else
            {
                integr = 0;
                set_motors_started( MOTOR_4 );
            }
            motors_armed = !motors_armed;
        }
    }

    // Each angle presents as integer 30.25 = 30250
    // control pitch [-1000 --- 1000]

    if ( motors_armed )
    {
        if ( control_values.throttle >= THROTTLE_OFF_LIMIT )
        {
            input_control_pitch = control_values.pitch;// > 500 ? 1000 : control_values.pitch < -500 ? -1000 : 0;

            pitch_error = (input_control_pitch*MAX_CONTROL_ANGLE - current_angles.pitch);            
            diff = pitch_error - prev_pitch;
            integr += pitch_error;

            process_UART_input_command( UART_get_last_received_command() );

            regul = (pitch_error/prop_k + integr/integr_k + diff*differ_k);
            regul = regul > INPUT_REG_LIMIT ?       INPUT_REG_LIMIT :
                    regul < (-INPUT_REG_LIMIT) ?    (-INPUT_REG_LIMIT) :
                                                    regul;
            
            prev_pitch = pitch_error;

            set_motor1_power( control_values.throttle*2 + regul);
            set_motor2_power( control_values.throttle*2 + regul);

            set_motor3_power( control_values.throttle*2 - regul);
            set_motor4_power( control_values.throttle*2 - regul);

            stop_counter = 0;
        }
        else
        {
            if ( stop_counter++ == STOP_LIMIT )
            {
                motors_armed = false;
                set_motors_stopped();
            }
        }
    }
#endif /* TEST_MOTOR4 */
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

#define BMP180_EXP_FILTER_PART  0.03D
inline void bmp180_rcv_filtered_data ( void )
{
    float    tmp_altitude;
    bmp180_rcv_press_temp_data( &bmp180_press, &bmp180_temp );
    tmp_altitude = bmp180_get_altitude( bmp180_press, 101325 ) - bmp180_initial_altitude;
    bmp180_altitude = ((tmp_altitude*BMP180_EXP_FILTER_PART) + (bmp180_altitude/1000.0*(1.0-BMP180_EXP_FILTER_PART))) * TEMP_MULTIPLYER;
}

void __attribute__( (__interrupt__, auto_psv) ) _T5Interrupt()
{
//    timer_start();
    bmp180_rcv_filtered_data();
    
    mpu6050_receive_gyro_accel_raw_data();
    mpu6050_get_gyro_accel_raw_data( &curr_data_accel_gyro );
    
    hmc5883l_receive_mag_raw_data();
    hmc5883l_get_scaled_mag_data( &curr_data_mag ); // In mGauss
    
    get_direction_values( &control_values );
    
    process_counts();
    
//    bmp180_altitude = log( bmp180_initial_press*1.0/bmp180_press ) * 1.0 * ((bmp180_temp+273*TEMP_MULTIPLYER) / 0.0341593F);
    
//    UART_write_string( "Pressure: %ld %ld %ld\n", bmp180_altitude, bmp180_press, bmp180_temp );
    
    process_control_system();
#ifdef SD_CARD
    process_saving_data();
#endif /* SD_CARD */
    
    #ifdef HMC5883L
    float angle = atan2( curr_data_mag.y_magnet, curr_data_mag.x_magnet )*RADIANS_TO_DEGREES;

    if ( angle < 0 )
        angle += 360;
    if ( angle > 360 )
        angle -= 360;

    UART_write_float(angle);
#endif /* HMC5883L */ 
    
//    time_elapsed_us = timer_stop()/TIMER_US_TICK;
//    UART_write_string( "T: %d\n", time_elapsed_us );
    _T5IF = 0;
}
