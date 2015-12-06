#include "core.h"
#include "MPU6050.h"
#include "BMP180.h"
#include "HMC5883L.h"
#include "input_control.h"
#include "motor_control.h"
#include "per_proto.h"
#include "FAT32.h"
#include "SDcard.h"
#include "file_io.h"
// For flash storage
#include <libpic30.h>

long long get_clock_freq() { return( FCY ); }

#ifdef FREQ_32MHZ
_FOSCSEL(FNOSC_PRI & IESO_OFF);
#else
_FOSCSEL(FNOSC_PRIPLL & IESO_OFF);
#endif
_FOSC(POSCMD_HS & OSCIOFNC_OFF & FCKSM_CSECMD);
_FWDT(FWDTEN_OFF);              // Watchdog Timer Enabled/disabled by user software

#ifdef NOT_USED
static void 
send_UART_sensors_data( void )
{
#ifndef UART_SENSORS_DEBUG
    uint16_t buffer[8];
    UART_write_byte( CONTROL_BYTE ); // Control byte - make control summ
//    buffer[0] = curr_data_accel_gyro.value.x_gyro;
//    buffer[1] = curr_data_accel_gyro.value.y_gyro;
//    buffer[2] = curr_data_accel_gyro.value.z_gyro;
//    buffer[3] = curr_data_accel_gyro.value.x_accel; 
//    buffer[4] = curr_data_accel_gyro.value.y_accel; 
//    buffer[5] = curr_data_accel_gyro.value.z_accel;
//    buffer[6] = time_elapsed_us;
//    buffer[7] = potenc_value;
    buffer[0] = current_angles.pitch;
//    buffer[1] = potenc_value;
    buffer[1] = control_values.pitch*MAX_CONTROL_ANGLE;//input_control_pitch*MAX_CONTROL_ANGLE;
//    buffer[3] = current_angles.acc_x;
//    buffer[4] = current_angles.gyr_delta_x;
    UART_write_words( buffer, 2 );
    UART_write_byte( CONTROL_BYTE ); // Control byte - make control summ
//    UART_write_int16( current_angles.pitch );
#else
//    send_UART_mpu6050_data();
    char buffer_s[256];
    sprintf( buffer_s, "#G:%05d,%05d,%05d#A:%05d,%05d,%05d#R:%05d",
                curr_data_accel_gyro.value.x_gyro, 
                curr_data_accel_gyro.value.y_gyro, 
                curr_data_accel_gyro.value.z_gyro,
                curr_data_accel_gyro.value.x_accel, 
                curr_data_accel_gyro.value.y_accel, 
                curr_data_accel_gyro.value.z_accel,
                potenc_value
            );
    UART_writeln_string(buffer_s);
#endif
}
#endif

void sensors_timer_init( void )
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

int main(void) 
{
    int err_state = 0;
    OFF_WATCH_DOG_TIMER;
    OFF_ALL_ANALOG_INPUTS;
#ifndef FREQ_32MHZ
    setup_PLL_oscillator();
#endif
    init_UART1( 57600 );
    debug( "/------------------------/" );
//    ADC_init();
//    motors_init();
//    debug( "Motors initialized" );
//    init_input_control();
//    debug( "IC initialized" );    
//    ic_find_control();
//    debug( "IC found" );
//    ic_make_calibration();
//    debug("IC calibrated");
    spi_init();
    debug( "SPI initialized" );
    spi_set_speed( FREQ_125K );
    if ( init_FAT32() < 0 )
    {
        debug("FAT32 not found!");
        while(1);
    }
    debug( "FAT initialized" );
    spi_set_speed( FREQ_16000K );
    i2c_init( 400000 );
    debug( "I2C initialized" );
    mpu6050_init();
//    mpu6050_calibration();
    debug( "MPU6050 initialized" );
    if ( bmp180_init(BMP085_STANDARD) != 0 )
    {
        debug("Bad init");
        while(1);
    }
    debug( "BMP180 initialized" );
    hmc5883l_init();
    debug( "HMC5883L initialized" );
    sensors_timer_init();
    
    debug( "Let`s begin!" );
    
/* FLASH write read code */
//int testdata[_FLASH_ROW];
//_prog_addressT p_eds_address = 0x21000;
//_memcpy_p2d16(testdata, p_eds_address, sizeof(testdata));
//UART_write_int(testdata[0]);
//if ( testdata[0] == 0xffff )
//{
//UART_writeln_string("Write values");
//_erase_flash(p_eds_address);
//int i;
//    for (i=0; i<(_FLASH_ROW);i++) {
//        testdata[i]=0x32;
//    }
//_write_flash16(p_eds_address, testdata);
//}
//_memcpy_p2d16(testdata, p_eds_address, sizeof(testdata));
//UART_write_hint(testdata[0]);
    
    while( 1 )
    {   
        file_process_tasks();
    }
    return( 0 );
}

static uint16_t prop_k = 30, integr_k = 10000, differ_k= 3;

static void process_UART_input_command( uint8_t input )
{
    char buffer_s[256];
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
    sprintf( buffer_s, "%d, %d, %d", 
                        prop_k, integr_k, differ_k
            );
    UART_writeln_string( buffer_s );
}

uint16_t    test_throttle = 0;
#define THROTTLE_STEP   1000
static void process_UART_input_command2( uint8_t input )
{
    switch ( input )
    {
        case 0:
            return;
        case 'q':
            test_throttle = test_throttle >= INPUT_POWER_RANGE ? INPUT_POWER_RANGE : (test_throttle + THROTTLE_STEP);
            break;
        case 'a':
            test_throttle = test_throttle == 0 ? 0 : (test_throttle - THROTTLE_STEP);
            break;
    }
    idebug( "P", test_throttle );
}

static Control_values_t     control_values;
static gyro_accel_data_t    curr_data_accel_gyro;
static magnetic_data_t      curr_data_mag;
static Angles_t             current_angles = { 0, 0, 0 };
static uint16_t             potenc_value = 0;
static uint16_t             time_elapsed_us = 0;

static BMP180_Stage_t       bmp_rcv_data_stage = TEMP;
static float                bmp180_temp = 0;
static uint32_t             bmp180_press = 0;

#define MAX_CONTROL_ANGLE   45L
static int16_t input_control_pitch = 0;
static int32_t regul = 0;

#define RADIANS_TO_DEGREES  (180/3.14159)
#define GYR_COEF            131L // = 65535/2/250
#define ANGLES_COEFF        1000L
#define SENS_TIME           2500L/1000000

static void process_angles_counts( void )
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

static bool     sticks_used = false, 
                new_data_flag = false;          
static int64_t  integr = 0; 
static int32_t  diff = 0, prev_pitch = 0, pitch_error = 0;

#define LOWER_LEFT_STICKS ( control_values.throttle < THROTTLE_OFF_LIMIT && \
                            control_values.rudder < START_ANGLES && \
                            control_values.roll < START_ANGLES && \
                            control_values.pitch < START_ANGLES )

static uint8_t power_flag = 0;

inline void process_control_system ( void )
{
    if ( power_flag != control_values.two_pos_switch )
    {
        power_flag = control_values.two_pos_switch;
        if ( control_values.two_pos_switch )
            set_motors_started( MOTOR_4 );
        else
            set_motors_stopped();
    }
    
    process_UART_input_command2( UART_get_last_received_command() );
    set_motor4_power( test_throttle );
#ifdef MAGNET
    float angle = atan2( curr_data_mag.y_magnet, curr_data_mag.x_magnet )*RADIANS_TO_DEGREES;

    if ( angle < 0 )
        angle += 360;
    if ( angle > 360 )
        angle -= 360;

    UART_write_float(angle);
#endif /* MAGNET */ 
            
/*
// Make flag of armed/disarmed in main loop
    if ( LOWER_LEFT_STICKS && !sticks_used )
    {
        if ( get_motors_state() == 0 )
        {
            integr = 0;
            set_motors_started();   
        }
        else
        {
            set_motors_stopped();
        }
        sticks_used = true;
        continue;
    }
    else if ( !LOWER_LEFT_STICKS )
    {
        sticks_used = false;
    }
    else if ( LOWER_LEFT_STICKS && sticks_used )
    {
        continue;
    }

    // Each angle presents as integer 30.25 = 30250
    // control pitch [-1000 --- 1000]

    input_control_pitch = control_values.pitch;// > 500 ? 1000 : control_values.pitch < -500 ? -1000 : 0;

    pitch_error = (input_control_pitch*MAX_CONTROL_ANGLE - current_angles.pitch);            
    diff = pitch_error - prev_pitch;
    integr += pitch_error;
//#define COMMAND

#ifdef COMMAND
    process_UART_input_command( UART_get_last_received_command() );
#endif
    regul = (pitch_error/prop_k + integr/integr_k + diff*differ_k);

    prev_pitch = pitch_error;

    control_values.throttle = 1000;

    set_motor1_power( control_values.throttle*2 + regul);
    set_motor2_power( control_values.throttle*2 + regul);

    set_motor3_power( control_values.throttle*2 - regul);
    set_motor4_power( control_values.throttle*2 - regul);
#ifndef COMMAND
    send_UART_sensors_data( );
#endif
*/
}

uint8_t writing_flag = 0;

char    filename[16],
        buffer_tmp[32];

inline void write_data_to_SD ( void )
{
    static uint16_t iter = 0;
    static uint8_t  file_num = 0;
    if ( iter == 0 )
    {
        sprintf( filename, "log%02d.txt", file_num );
        file_open( filename );
    }
    else if ( iter == 1000 )
    {
        file_close();
        if ( file_num++ == 30 )
        {
            writing_flag = 0;
            debug( "Done" );
        }
        else
        {
            iter = 0;
            _T5IF = 0;
            return;
        }
    }
    else
    {
        sprintf( buffer_tmp, "%06d test str\n", iter );
        file_write( buffer_tmp, 16 );
    }
    iter++;
}

void __attribute__( (__interrupt__, auto_psv) ) _T5Interrupt()
{
//    timer_start();
    switch ( bmp_rcv_data_stage )
    {
        case TEMP:
            if ( bmp180_data_ready() )
            {
                bmp180_temp = bmp180_read_temperature_C();
                bmp180_send_pressure_signal();
                bmp_rcv_data_stage = PRESS;
            }
            break;
        case PRESS:
            if ( bmp180_data_ready() )
            {
                bmp180_press = bmp180_read_pressure();
                bmp180_send_temperature_signal();
                bmp_rcv_data_stage = TEMP;
            }
            break;
    }
    mpu6050_receive_gyro_accel_raw_data();
    mpu6050_get_gyro_accel_raw_data( &curr_data_accel_gyro );
//    hmc5883l_receive_mag_raw_data();
//    hmc5883l_get_scaled_mag_data( &curr_data_mag ); // In mGauss
//    get_direction_values( &control_values );
//    potenc_value = ADC_read() - 3655; // 3655 - mid of construction
    process_angles_counts();
//    process_control_system();
    if ( writing_flag )
        write_data_to_SD();
    
//    time_elapsed_us = timer_stop()/TIMER_US_TICK;
//    idebug( "T", time_elapsed_us );
    _T5IF = 0;
}
