#include "core.h"
#include "MPU6050.h"
#include "BMP180.h"
#include "HMC5883L.h"
#include "input_control.h"
#include "motor_control.h"
#include "per_proto.h"
#include "file_io.h"
#include "math_proto.h"

#include "pragmas.h"

//#define SD_CARD
#define PROGRAM_INPUT
#define PID_tuning

void control_system_timer_init( void )
{
    T4CONbits.TON = 0;
    T4CONbits.T32 = 1;
    T4CONbits.TCKPS = TIMER_DIV_1;
    _T5IP = 1;
    _T5IE = 1;
    PR5 = (((FCY/FREQ_CONTROL_SYSTEM) >> 16) & 0xffff);
    PR4 = ((FCY/FREQ_CONTROL_SYSTEM) & 0xffff);
    T4CONbits.TON = 1;
}

#ifdef SD_CARD
static int file_num = 0;
#endif /* SD_CARD */

static float        bmp180_initial_altitude = 0.0;
static uint32_t     bmp180_press = 0,
                    bmp180_temp = 0;    // Temperature is multiplied by TEMP_MULTIPLYER
static int32_t      bmp180_altitude = 0;

int main(void) 
{
    OFF_ALL_ANALOG_INPUTS;
    INIT_ERR_L;
    ERR_LIGHT = ERR_LIGHT_ERR;
    UART_init( UARTm1, UART_115200 );
    UART_write_string( UARTm1, "/------------------------/\n" );
#ifdef PID_tuning
    UART_set_receive_mode( UARTm1, UARTr_interrupt );
    UART_write_string( UARTm1, "UART initialized to interrupt mode\n" );
#else
    UART_set_receive_mode( UARTm1, UARTr_polling );
    UART_write_string( UARTm1, "UART initialized to polling mode\n" );
#endif
#ifdef SD_CARD
    flash_read();
    file_num = flash_get( FILE_NUM );
    file_num = file_num < 0 ? 0 : file_num;
    UART_write_string( UARTm1, "Flash successfully read\n" );
#endif /* SD_CARD */
    motors_init();
    UART_write_string( UARTm1, "Motors initialized\n" );
#ifndef PROGRAM_INPUT
    ic_init();
    UART_write_string( UARTm1, "IC initialized\n" );
    ic_find_control();
    UART_write_string( UARTm1, "IC found\n" );
#endif // PROGRAM_INPUT
//    ic_make_calibration();
//    UART_write_string("IC calibrated\n");
#ifdef SD_CARD
    spi_init();
    UART_write_string( UARTm1, "SPI initialized\n" );
    init_sd_file_io();
    UART_write_string( UARTm1, "SD initialized\n" );
#endif /* SD_CARD */
    i2c_init( 400000 );
    UART_write_string( UARTm1, "I2C initialized\n" );
    if ( mpu6050_init() != 0 )
    {
        UART_write_string( UARTm1, "Failed MPU init\n" );
        error_process();
    }
    UART_write_string( UARTm1, "MPU6050 initialized\n" );
    
    if ( bmp180_init( BMP085_ULTRAHIGHRES ) != 0 )
    {
        UART_write_string( UARTm1, "Failed BMP init\n" );
        error_process();
    }
    UART_write_string( UARTm1, "BMP180 initialized\n" );
    bmp180_calibrate( &bmp180_press );
    bmp180_initial_altitude = bmp180_get_altitude( bmp180_press, 101325 );
    if ( hmc5883l_init( -48, -440 ) != 0 )
    {
        UART_write_string( UARTm1, "Failed HMC init\n");
        error_process();
    }
    UART_write_string( UARTm1, "HMC5883L initialized\n" );
    control_system_timer_init();
    UART_write_string( UARTm1, "Let`s begin!\n" );
    ERR_LIGHT = ERR_LIGHT_NO_ERR;
//    mpu6050_calibration();
    
    while( 1 )
    {   
        file_process_tasks();
    }
    return( 0 );
}

static uint16_t prop_r = 30, integr_r = 10000, differ_r = 70,
                prop_p = 20, integr_p = 6250, differ_p = 40;
bool start_motors = false;

static void process_UART_input_command( uint8_t input )
{
    switch ( input )
    {
        case 0:
            return;
        case 'b':
            start_motors = true;
            break;
        case 'q':
            prop_r++;
            break;
        case 'w':
            prop_r--;
            break;
        case 'a':
            integr_r+=10;
            break;
        case 's':
            integr_r-=10;
            break;
        case 'z':
            differ_r++;
            break;
        case 'x':
            differ_r--;
            break;
        case 'e':
            prop_p++;
            break;
        case 'r':
            prop_p--;
            break;
        case 'd':
            integr_p+=10;
            break;
        case 'f':
            integr_p-=10;
            break;
        case 'c':
            differ_p++;
            break;
        case 'v':
            differ_p--;
            break;
    }
    UART_write_string( UARTm1, "P%d,D%d,I%d;P%d,D%d,I%d\n", prop_r, integr_r, differ_r, prop_p, integr_p, differ_p );
}

static Control_values_t     control_values;
static gyro_accel_data_t    curr_data_accel_gyro;
static Angles_t             current_angles = { 0, 0, 0 };
static bool                 motors_armed = false;

// p = p0*exp(-0.0341593/(t+273)*h)
// h = ln(p0/p) * (t+273)/0.0341593

#define GYR_COEF            131L // = 65535/2/250
#define ANGLES_COEFF        1000L   // each float is represented as integer *1000 (3 decimals after point)
#define SENS_TIME           2500L/1000000

static void process_counts( void )
{
    gyro_accel_data_t *c_d = &curr_data_accel_gyro;
    
    // Just for one of arguments for atan2 be not zero
    int32_t acc_x = c_d->value.x_accel == 0 ? 1 : c_d->value.x_accel,
            acc_y = c_d->value.y_accel == 0 ? 1 : c_d->value.y_accel,
            acc_z = c_d->value.z_accel;
#undef MATH_LIB_
    current_angles.acc_x = 
#ifdef MATH_LIB_
                atan2_fp(acc_y, sqrt(acc_x*acc_x + acc_z*acc_z)) * ANGLES_COEFF,
#else
                atan2(acc_y, sqrt(acc_x*acc_x + acc_z*acc_z)) * RADIANS_TO_DEGREES * ANGLES_COEFF,
#endif
    current_angles.acc_y = 
#ifdef MATH_LIB_
                atan2_fp(-1 * acc_x, sqrt(acc_y*acc_y + acc_z*acc_z)) * ANGLES_COEFF;
#else
                atan2(-1 * acc_x, sqrt(acc_y*acc_y + acc_z*acc_z)) * RADIANS_TO_DEGREES * ANGLES_COEFF;
#endif
    
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
#define STOP_LIMIT          1000L   // 2.5 sec - low thrust limit
#define REGULATION_LIMIT    10000L

static int64_t  integrPitch = 0,
                integrRoll = 0;
static int32_t  prevPitch = 0,
                prevRoll = 0;
       
static int16_t  stop_counter = 0;

inline int16_t process_controller_pitch( int32_t error )
{
    int32_t diff = error - prevPitch;
    integrPitch += error/10;    // Tricky way <<<

    int32_t regul = (error/prop_p + integrPitch/integr_p + diff*differ_p);
    regul = regul > REGULATION_LIMIT ?       REGULATION_LIMIT :
            regul < (-REGULATION_LIMIT) ?    (-REGULATION_LIMIT) :
                                            regul;

    prevPitch = error;

    return( regul );
}

inline int16_t process_controller_roll( int32_t error )
{
    int32_t diff = error - prevRoll;
    integrRoll += error/10;     // Tricky way <<<

    int32_t regul = (error/prop_r + integrRoll/integr_r + diff*differ_r);
    regul = regul > REGULATION_LIMIT ?       REGULATION_LIMIT :
            regul < (-REGULATION_LIMIT) ?    (-REGULATION_LIMIT) :
                                            regul;

    prevRoll = error;

    return( regul );
}

uint8_t receivedByte = 0;
void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    receivedByte = U1RXREG;
    _U1RXIF = 0;
}

inline void process_control_system ( void )
{
#ifndef PROGRAM_INPUT   
    static bool     start_stop_flag = false;
    bool sticks_in_start_position = START_STOP_COND;   

    if ( sticks_in_start_position != start_stop_flag )
    {   // If go from some position to corners (power on position) or from corners to some position
        start_stop_flag = sticks_in_start_position;
        if ( sticks_in_start_position )
        {   // If now in corner position
#else
#ifdef PID_tuning
    process_UART_input_command( receivedByte );
    receivedByte = 0;
#endif
    if ( start_motors )
    {
#endif
            if ( motors_armed )
            {   // If motors were armed - turn them off
                set_motors_stopped();
                UART_write_string( UARTm1, "Motors stopped\n" );
            }
            else
            {   // If not armed - turn on and zero integral part of controller
                integrPitch = integrRoll = 0;
                set_motors_started( MOTORS_ALL);
                UART_write_string( UARTm1, "All motors started\n" );
            }
            motors_armed = !motors_armed;
#ifndef PROGRAM_INPUT
        }
#else
        start_motors = false;
#endif
    }
        
    // Each angle presents as integer 30.25 = 30250
    // control pitch [-1000 --- 1000]

    if ( motors_armed )
    {
        control_values.throttle = 0;
#ifndef PROGRAM_INPUT
        if ( control_values.throttle >= THROTTLE_OFF_LIMIT )
        {
#endif // PROGRAM_INPUT
            set_motor1_power( control_values.throttle
//                                + process_controller_pitch( -current_angles.pitch )
                                - process_controller_roll( -current_angles.roll )
                            );
            set_motor2_power( control_values.throttle
//                                + process_controller_pitch( -current_angles.pitch )
                                + process_controller_roll( -current_angles.roll )
                            );
            set_motor3_power( control_values.throttle
//                                - process_controller_pitch( -current_angles.pitch )
                                + process_controller_roll( -current_angles.roll )
                            );
            set_motor4_power( control_values.throttle
//                                - process_controller_pitch( -current_angles.pitch )
                                - process_controller_roll( -current_angles.roll )
                            );

            stop_counter = 0;
#ifndef PROGRAM_INPUT
        }
        else
        {
            if ( stop_counter++ == STOP_LIMIT )
            {
                motors_armed = false;
                set_motors_stopped();
            }
        }
#endif // PROGRAM_INPUT
    }
}
    
bool dataSend = false;
uint32_t timeMoments = 0;
uint32_t timeStep10 = 25;
    
inline void process_sending_UART_data( void )
{
    switch ( receive_command() )
    {
        case NO_COMMAND:
            break;
        case CONNECT:
            UART_write_string( UARTm1, "ok!" );
            break;
        case DATA_START:
            UART_write_string( UARTm1, "ok!" );
            timeMoments = 0;
            dataSend = true;
            break;
        case DATA_STOP:
            UART_write_string( UARTm1, "ok!" );
            dataSend = false;
            break;
    }
    
    if ( dataSend )
    {
        uint16_t sendBuffer[6];
        sendBuffer[0] = current_angles.roll >> 16;
        sendBuffer[1] = current_angles.roll & 0xffff;
        sendBuffer[2] = current_angles.pitch >> 16;
        sendBuffer[3] = current_angles.pitch & 0xffff;
        sendBuffer[4] = timeMoments >> 16;
        sendBuffer[5] = timeMoments & 0xffff;
        
        UART_write_words( UARTm1, sendBuffer, 6 );
        
        timeMoments += timeStep10;
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

#define BMP180_EXP_FILTER_PART  0.03D
inline void bmp180_rcv_filtered_data ( void )
{
    float    tmp_altitude;
    bmp180_rcv_press_temp_data( &bmp180_press, &bmp180_temp );
    tmp_altitude = bmp180_get_altitude( bmp180_press, 101325 ) - bmp180_initial_altitude;
    bmp180_altitude = ((tmp_altitude*BMP180_EXP_FILTER_PART) + (bmp180_altitude/1000.0*(1.0-BMP180_EXP_FILTER_PART))) * TEMP_MULTIPLYER;
}

//static uint16_t time_elapsed_us = 0;

void __attribute__( (__interrupt__, no_auto_psv) ) _T5Interrupt()
{
//    timer_start();
    bmp180_rcv_filtered_data();
    
    mpu6050_receive_gyro_accel_raw_data();
    mpu6050_get_gyro_accel_raw_data( &curr_data_accel_gyro );
    
//    int16_t angle_deg = hmc5883l_get_yaw_angle();
    
    get_control_values( &control_values );
    process_counts();
    
//    bmp180_altitude = log( bmp180_initial_press*1.0/bmp180_press ) * 1.0 * ((bmp180_temp+273*TEMP_MULTIPLYER) / 0.0341593F);
//    UART_write_string( UARTm1, "Pressure: %ld %ld %ld\n", bmp180_altitude, bmp180_press, bmp180_temp );
    process_control_system();
#ifndef PID_tuning
    process_sending_UART_data();
#endif
#ifdef SD_CARD
    process_saving_data();
#endif /* SD_CARD */
    
//    time_elapsed_us = convert_ticks_to_us( timer_stop(), 1 );
//    UART_write_string( UARTm1, "%s, %ld, %ld\n\r", 
//            motors_armed ? "Armed" : "Disarmed", current_angles.pitch, current_angles.roll );
    _T5IF = 0;
}
