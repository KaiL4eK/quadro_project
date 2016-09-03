#include "core.h"
#include "MPU6050.h"
#include "BMP180.h"
#include "HMC5883L.h"
#include "input_control.h"
#include "motor_control.h"

#include "file_io.h"

#include "pragmas.h"

void control_system_timer_init( void );

//#define SD_CARD
#define PROGRAM_INPUT
#define PID_tuning
//#define TEST_WO_MODULES

#ifdef SD_CARD
static int file_num = 0;
#endif /* SD_CARD */

#ifdef ENABLE_BMP180
static float        bmp180_initial_altitude = 0.0;
static uint32_t     bmp180_press = 0,
                    bmp180_temp = 0;    // Temperature is multiplied by TEMP_MULTIPLYER
static int32_t      bmp180_altitude = 0;
#endif

quadrotor_state_t    quadrotor_state;
Control_values_t     *control_values;
gyro_accel_data_t    *gyro_accel_data;

int main(void) 
{
    OFF_ALL_ANALOG_INPUTS;
    INIT_ERR_L;
    ERR_LIGHT = ERR_LIGHT_ERR;
    UART_init( UARTm1, UART_115200, true );
    UART_init( UARTm2, UART_38400, false );
    cmdProcessor_init();
    UART_write_string( UARTm1, "/------------------------/\n" );
    UART_write_string( UARTm1, "UART initialized to interrupt mode\n" );
#ifndef TEST_WO_MODULES 
#ifdef SD_CARD
    flash_read();
    file_num = flash_get( FILE_NUM );
    file_num = file_num < 0 ? 0 : file_num;
    UART_write_string( UARTm1, "Flash successfully read\n" );
#endif /* SD_CARD */

    control_values = ic_init();
    UART_write_string( UARTm1, "IC initialized\n" );
//#ifndef PROGRAM_INPUT
//    ic_find_control();
//    UART_write_string( UARTm1, "IC found\n" );
//#endif // PROGRAM_INPUT
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
    
    if ( (gyro_accel_data = mpu6050_init()) == NULL )
    {
        UART_write_string( UARTm1, "Failed MPU init\n" );
        error_process( "MPU6050 initialization" );
    }
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
    motors_init();
    UART_write_string( UARTm1, "Motors initialized\n" );
    
    control_system_timer_init();
    UART_write_string( UARTm1, "Let`s begin!\n" );
    ERR_LIGHT = ERR_LIGHT_NO_ERR;
    
    while( 1 ) {
//        UART_write_string( UARTm1, "Hello\n" );
        file_process_tasks();
//        delay_ms( 500 );
    }
    
    return( 0 );
}

static uint16_t prop_r = 7, integr_r = 4000, differ_r = 25,
                prop_p = 7,  integr_p = 4000, differ_p = 25;

bool start_motors = false,
     stop_motors = false;

#ifdef PID_tuning
uint8_t receivedByte = 0;
void __attribute__( (__interrupt__, auto_psv) ) _U1RXInterrupt()
{
    receivedByte = U1RXREG;
    _U1RXIF = 0;
}

static void process_UART_PID_tuning()
{
    switch ( receivedByte )
    {
        case 0:
            return;
//        case 'B':
//        case 'b':
//            start_motors = true;
//            break;
//        case 'N':
//        case 'n':
//            stop_motors = true;
//            break;
        case 'Q':
        case 'q':
            prop_r++;
            break;
        case 'W':
        case 'w':
            prop_r--;
            break;
        case 'A':
        case 'a':
            integr_r+=10;
            break;
        case 'S':
        case 's':
            integr_r-=10;
            break;
        case 'Z':
        case 'z':
            differ_r++;
            break;
        case 'X':
        case 'x':
            differ_r--;
            break;
        case 'E':
        case 'e':
            prop_p++;
            break;
        case 'R':
        case 'r':
            prop_p--;
            break;
        case 'D':
        case 'd':
            integr_p+=10;
            break;
        case 'F':
        case 'f':
            integr_p-=10;
            break;
        case 'C':
        case 'c':
            differ_p++;
            break;
        case 'V':
        case 'v':
            differ_p--;
            break;
    }
    UART_write_string( UARTm1, "P%d D%d I%d | P%d D%d I%d\n", prop_r, differ_r, integr_r, prop_p, differ_p, integr_p );
    receivedByte = 0;
}
#endif

// p = p0*exp(-0.0341593/(t+273)*h)
// h = ln(p0/p) * (t+273)/0.0341593

#define START_STOP_COND (   control_values.throttle < THROTTLE_OFF_LIMIT && \
                            control_values.rudder > (-1*START_ANGLES) && \
                            control_values.roll < START_ANGLES && \
                            control_values.pitch > (-1*START_ANGLES) )

#define MAX_CONTROL_ANGLE   25L
#define CONTROL_2_ANGLE(x) ((x)/10 * MAX_CONTROL_ANGLE)
#define STOP_LIMIT          1000L   // 1k * 2.5 ms = 2.5 sec - low thrust limit
//#define REGULATION_LIMIT    10000L

static int64_t  integrRoll = 0;
static int64_t  integrPitch = 0;

int32_t process_controller_pitch( int32_t error )
{
    static int32_t  prevPitch = 0;
    
    integrPitch += error/10;

    int32_t regul = (error/prop_p + integrPitch/integr_p + (error - prevPitch)*differ_p);

    prevPitch = error;

    return( regul );
}

int32_t process_controller_roll( int32_t error )
{
    static int32_t  prevRoll = 0;
    
    integrRoll += error/10;

    int32_t regul = (error/prop_r + integrRoll/integr_r + (error - prevRoll)*differ_r);

    prevRoll = error;

    return( regul );
}

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
            integrPitch = integrRoll = 0;
            set_motors_started( MOTORS_ALL);
            UART_write_string( UARTm1, 
                    "All motors started with power %d\n", motorPower );
            motors_armed = true;
        }
        start_motors = false;
    }
    if ( stop_motors )
    {
        if ( motors_armed )
        {   // If motors were armed - turn them off
            set_motors_stopped();
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
    int32_t power = 0;
    quadrotor_state.motor1_power =
    quadrotor_state.motor2_power =
    quadrotor_state.motor3_power =
    quadrotor_state.motor4_power = 0;
    
    if ( motors_armed )
    {
//        control_values.throttle = motorPower;
#ifndef PROGRAM_INPUT
        if ( control_values.throttle >= THROTTLE_OFF_LIMIT )
        {
#endif // PROGRAM_INPUT
            power = motorPower // control_values.throttle
                                + process_controller_pitch( CONTROL_2_ANGLE(control_values->pitch) - quadrotor_state.pitch )
                                - process_controller_roll( CONTROL_2_ANGLE(control_values->roll) - quadrotor_state.roll );
            
            quadrotor_state.motor1_power = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
            set_motor_power( 1, quadrotor_state.motor1_power );
            
            power = motorPower // control_values.throttle
                                + process_controller_pitch( CONTROL_2_ANGLE(control_values->pitch) - quadrotor_state.pitch )
                                + process_controller_roll( CONTROL_2_ANGLE(control_values->roll) - quadrotor_state.roll );
            
            quadrotor_state.motor2_power = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
            set_motor_power( 2, quadrotor_state.motor2_power );
            
            power = motorPower // control_values.throttle
                                - process_controller_pitch( CONTROL_2_ANGLE(control_values->pitch) - quadrotor_state.pitch )
                                + process_controller_roll( CONTROL_2_ANGLE(control_values->roll) - quadrotor_state.roll );
            
            quadrotor_state.motor3_power = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
            set_motor_power( 3, quadrotor_state.motor3_power );
            
            power = motorPower // control_values.throttle
                                - process_controller_pitch( CONTROL_2_ANGLE(control_values->pitch) - quadrotor_state.pitch )
                                - process_controller_roll( CONTROL_2_ANGLE(control_values->roll) - quadrotor_state.roll );
            
            quadrotor_state.motor4_power = clip_value( power, INPUT_POWER_MIN, INPUT_POWER_MAX );
            set_motor_power( 4, quadrotor_state.motor4_power );
#ifndef PROGRAM_INPUT
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
#endif // PROGRAM_INPUT
    }
}

#define POWER_2_PERCENT(x) ( (uint8_t)((x)*100L/INPUT_POWER_MAX) )
           
void process_sending_UART_data( void )
{
    static bool         dataSend        = false;
    static uint8_t      counterSend     = 0;
    static uint16_t     timeMoments     = 0;
    
    UART_frame_t *frame = cmdProcessor_rcvFrame();
    switch ( frame->command )
    {
        case NO_COMMAND:
        case UNKNOWN_COMMAND:
            break;
        case CONNECT:
            cmdProcessor_write_cmd_resp( UARTm2, 
                    RESPONSE_PREFIX, RESP_NOERROR );
            dataSend = false;
            stop_motors = true;
            UART_write_string( UARTm1, "Connect\n" );
            break;
        case DATA_START:
            timeMoments = 0;
            counterSend = 0;
            dataSend = true;
            UART_write_string( UARTm1, "DStart\n" );
            break;
        case DATA_STOP:
            dataSend = false;
            UART_write_string( UARTm1, "DStop\n" );
            break;
        case MOTOR_START:
            start_motors = true;
            motorPower = frame->motorPower * INPUT_POWER_MAX / 100L;
            UART_write_string( UARTm1, "MStart\n" );
            break;
        case MOTOR_STOP:
            stop_motors = true;
            UART_write_string( UARTm1, "MStop\n" );
            break;
    }
    
    if ( dataSend && ++counterSend == 4 )
    {
//        quadrotor_state.roll = quadrotor_state.pitch = 
//                45*ANGLES_COEFF*sin(timeMoments/100.0);
        uint16_t sendBuffer[5];
        // angle * 100
        sendBuffer[0] = quadrotor_state.roll;
        sendBuffer[1] = quadrotor_state.pitch;
        sendBuffer[2] = timeMoments;
        sendBuffer[3] = POWER_2_PERCENT(quadrotor_state.motor1_power);
        sendBuffer[3] <<= 8;
        sendBuffer[3] |= POWER_2_PERCENT(quadrotor_state.motor2_power);
        sendBuffer[4] = POWER_2_PERCENT(quadrotor_state.motor3_power);
        sendBuffer[4] <<= 8;
        sendBuffer[4] |= POWER_2_PERCENT(quadrotor_state.motor4_power);
        
        UART_write_byte( UARTm2, DATA_PREFIX );
        UART_write_words( UARTm2, sendBuffer, 5 );

        if ( ++timeMoments == UINT16_MAX )
        {
            cmdProcessor_write_cmd_resp( UARTm2, RESPONSE_PREFIX, RESP_ENDDATA );
            UART_write_string( UARTm1, "DStop1\n" );
            dataSend = false;
            start_motors = false;
        }
        counterSend = 0;
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

#define GYR_COEF                    131.0f      // = 65535/2/250
#define ANGLES_COEFF                100L        // each float is represented as integer *100 (2 decimals after point)
#define SENS_TIME                   0.0025f     // 2500L/1000000

#define COMPLIMENTARY_COEFFICIENT   0.95f

static void process_counts()
{
    gyro_accel_data_t *c_d = gyro_accel_data;
    
    // Just for one of arguments for atan2 be not zero
    int32_t acc_x = c_d->value.x_accel == 0 ? 1 : c_d->value.x_accel,
            acc_y = c_d->value.y_accel == 0 ? 1 : c_d->value.y_accel,
            acc_z = c_d->value.z_accel;

    quadrotor_state.acc_x = atan2( acc_y, sqrt(acc_x*acc_x + acc_z*acc_z)) * RADIANS_TO_DEGREES * ANGLES_COEFF,
    quadrotor_state.acc_y = atan2(-acc_x, sqrt(acc_y*acc_y + acc_z*acc_z)) * RADIANS_TO_DEGREES * ANGLES_COEFF;
    
    int32_t gyr_delta_x = (c_d->value.x_gyro*ANGLES_COEFF/GYR_COEF) * SENS_TIME;
    int32_t gyr_delta_y = (c_d->value.y_gyro*ANGLES_COEFF/GYR_COEF) * SENS_TIME;
    int32_t gyr_delta_z = (c_d->value.z_gyro*ANGLES_COEFF/GYR_COEF) * SENS_TIME;
    
    quadrotor_state.pitch = (COMPLIMENTARY_COEFFICIENT * (gyr_delta_x + quadrotor_state.pitch)) 
                            + (1.0f-COMPLIMENTARY_COEFFICIENT) * quadrotor_state.acc_x;
    quadrotor_state.roll  = (COMPLIMENTARY_COEFFICIENT * (gyr_delta_y + quadrotor_state.roll)) 
                            + (1.0f-COMPLIMENTARY_COEFFICIENT) * quadrotor_state.acc_y;
    quadrotor_state.yaw   = gyr_delta_z + quadrotor_state.yaw;
}

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

//#define MEASURE_INT_TIME

void __attribute__( (__interrupt__, no_auto_psv) ) _T5Interrupt()
{
#ifdef MEASURE_INT_TIME
    timer_start();
#endif
#ifndef TEST_WO_MODULES

    if ( mpu6050_receive_gyro_accel_raw_data() )
        return;
    
//    send_UART_mpu6050_data( UARTm1 );
    
#ifdef ENABLE_HMC5883
    int16_t angle_deg = hmc5883l_get_yaw_angle();
#endif
    
    get_control_values();
//    send_UART_control_values();
//    send_UART_calibration_data();
    
    process_counts();
//    UART_write_string( UARTm1, "R %ld, P %ld\n", quadrotor_state.roll, quadrotor_state.pitch );
    
#ifdef ENABLE_BMP180
    bmp180_rcv_filtered_data();
    
    bmp180_altitude = log( bmp180_initial_press*1.0/bmp180_press ) * 1.0 * ((bmp180_temp+273*TEMP_MULTIPLYER) / 0.0341593F);
    UART_write_string( UARTm1, "Pressure: %ld %ld %ld\n", bmp180_altitude, bmp180_press, bmp180_temp );
#endif
    
#endif // TEST_WO_MODULES
    
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
