/*
 * File:   main.c
 * Author: alex
 *
 * Created on March 16, 2017, 2:52 PM
 */

#include <dsp.h>

#include <per_proto.h>
#include <pragmas.h>

#include <math_proto.h>
#include <filters.h>
#include <MPU6050.h>

#define UART_INTERFACE 1
static uart_module_t        uart_interface      = NULL;
static gyro_accel_data_t    *g_a                = NULL;
static float                gyro_sensitivity    = 0.0;

void process_inv_sqrt_benchmark ( void );
void process_mpu_filter_benchmark ( bool manual );
void dsp_tect_benchmark ( void );

#define HELP_MESSAGE    "\nBenchmark is ready:\n\t'1' to start <inv_sqrt> benchmark\n" \
                                               "\t'2' to start <filters> benchmark\n" \
                                               "\t'3' to start <filters_man> benchmark\n" \
                                               "\t'4' to start <dsp_test> benchmark\n" \
                                               "\t'5' to ititialize MPU6050 on main (AD0 - low) channel\n"

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;

    uart_interface = UART_init( UART_INTERFACE, UART_460800, INT_PRIO_HIGHEST );
    UART_write_set_big_endian_mode( uart_interface, true );
    bool show_notif = false;
    
    i2c_init( 1, 400000 );
    UART_write_string( uart_interface, "I2C initialized\n" );
    
    while ( 1 )
    {
        if ( !show_notif )
        {
            UART_write_string( uart_interface, HELP_MESSAGE );
            show_notif = true;
        }
        
        if ( UART_bytes_available( uart_interface ) )
        {
            uint8_t input = UART_get_byte( uart_interface );
            if ( input == '1' )
            {
                process_inv_sqrt_benchmark();

                UART_clean_input( uart_interface );
                show_notif = false;
            } else if ( input == '2' )
            {
                if ( g_a == NULL )
                {
                    UART_write_string( uart_interface, "MPU6050 is not initialized" );
                    continue;
                }
                
                process_mpu_filter_benchmark( false );

                UART_clean_input( uart_interface );
                show_notif = false;
                
            } else if ( input == '3' )
            {
                if ( g_a == NULL )
                {
                    UART_write_string( uart_interface, "MPU6050 is not initialized" );
                    continue;
                } 
                
                process_mpu_filter_benchmark( true );

                UART_clean_input( uart_interface );
                show_notif = false;
                
            } else if ( input == '4' )
            {
                dsp_tect_benchmark();
                show_notif = false;
            } else if ( input == '5' )
            {
                if ( g_a != NULL )
                {
                    UART_write_string( uart_interface, "MPU6050 is already initialized" );
                    continue;
                }
                
                if ( mpu6050_init( NULL, uart_interface ) < 0 )
                {
                    UART_write_string( uart_interface, "MPU6050 initialization failed" );
                } else {
//                    mpu6050_calibration();
                    mpu6050_offsets_t mpu6050_offsets = { -3728, 1209, 1927, 24, -13, 80 };     // MPU

                    mpu6050_set_bandwidth( MPU6050_DLPF_BW_20 );
                    mpu6050_set_offsets( &mpu6050_offsets );
                    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_500 );
                    mpu6050_set_accel_fullscale( MPU6050_ACCEL_FS_8 );

                    g_a                 = mpu6050_get_raw_data();
                    gyro_sensitivity    = mpu6050_get_gyro_sensitivity_rate();

                    filter_initialize( 1.0/1000 );
                    madgwick_filter_set_angle_rate( 0.9 );
                }
            } else
                UART_write_string( uart_interface, HELP_MESSAGE );
        }
    }
    
    return 0;
}

#define DSP_TEST_SAMPLES_COUNT  10000

void dsp_tect_benchmark ( void )
{
    uint32_t            dsp_time_max_us     = 0;
    uint32_t            man_time_max_us     = 0;
    int                 dsp_max_index       = 0;
    int                 man_max_index       = 0;
    fractional          dsp_max             = INT16_MIN;
    fractional          man_max             = INT16_MIN;
    
#if 1
    fractional  test_data[DSP_TEST_SAMPLES_COUNT];
    uint16_t    i;
    for ( i = 0; i < DSP_TEST_SAMPLES_COUNT; i++ )
    {
        float rand_float = rand() * 1.0 / INT16_MAX;
        test_data[i] = Float2Fract( rand_float );
    }
#endif
    
    timer_start();
    dsp_max = VectorMax( DSP_TEST_SAMPLES_COUNT, test_data, &dsp_max_index );
    timer_stop();
    dsp_time_max_us = timer_get_us();
    
    timer_start();
    for ( i = 0; i < DSP_TEST_SAMPLES_COUNT; i++ )
        if ( test_data[i] > man_max ) {
            man_max         = test_data[i];
            man_max_index   = i;
        }
    timer_stop();
    man_time_max_us = timer_get_us();
    
    // Float2Fract -- [-1.0 / 1.0]
//    fractional test_data[4] = {Float2Fract(0.1), Float2Fract(1.0), Float2Fract(0.7), Float2Fract(0.3)};
//    
    
//    UART_write_string( uart_interface, "Input: %d\n", test_data[0] );
    
//    fractional res = VectorPower( 4, test_data );
//    fractional max_ = VectorMax( 4, test_data, &max_index );

    UART_write_string( uart_interface, "DSP Max time: %ld\n", dsp_time_max_us );
    UART_write_string( uart_interface, "Manual Max time: %ld\n", man_time_max_us );
    UART_write_string( uart_interface, "DSP Max: %d / index: %d\n", dsp_max, dsp_max_index );
    UART_write_string( uart_interface, "Manual Max: %d / index: %d\n", man_max, man_max_index );
}

void process_mpu_filter_benchmark ( bool manual )
{
#define MPU_READINGS_COUNT 10000
    
    uint16_t            reading_counter         = 0;
    uint32_t            time_elapsed_us         = 0;
    uint32_t            max_time_complementary  = 0;
    uint32_t            max_time_madgwick       = 0;
    uint32_t            max_mpu_time            = 0;
    imu_filter_input_t  imu_input;
    euler_angles_t      complementary_angles;
    euler_angles_t      madgwick_angles;    
    
//    madgwick_filter_reset_values();
    madgwick_filter_set_inv_sqrt_method_manual( manual );
    
    UART_write_string( uart_interface, "\nStarting <filters> benchmark\n" );
    if ( manual )
        UART_write_string( uart_interface, "With manual inverse sqrt calculation\n" );
    
    for ( reading_counter = 0; reading_counter < MPU_READINGS_COUNT; reading_counter++ )
    {
        timer_start();
        mpu6050_receive_gyro_accel_raw_data();
                
        imu_input.acc_x = g_a->value.x_accel;
        imu_input.acc_y = g_a->value.y_accel;
        imu_input.acc_z = g_a->value.z_accel;
        imu_input.gyr_x = g_a->value.x_gyro * gyro_sensitivity;
        imu_input.gyr_y = g_a->value.y_gyro * gyro_sensitivity;
        imu_input.gyr_z = g_a->value.z_gyro * gyro_sensitivity;
        
        timer_stop();
        time_elapsed_us = timer_get_us();
        max_mpu_time = max( max_mpu_time, time_elapsed_us );
        
        timer_start();
        complementary_filter_position_execute( &imu_input, &complementary_angles );
        timer_stop();
        time_elapsed_us = timer_get_us();
        max_time_complementary = max( max_time_complementary, time_elapsed_us );
        
        timer_start();
        madgwick_filter_position_execute( &imu_input, &madgwick_angles );
        timer_stop();
        time_elapsed_us = timer_get_us();
        max_time_madgwick = max( max_time_madgwick, time_elapsed_us );
        
        if ( reading_counter % 100 == 0 )
        {
            UART_write_string( uart_interface, "Read data: %05d, %05d, %05d, %f, %f, %f\n",   
                                                                                    g_a->value.x_accel, 
                                                                                    g_a->value.y_accel, 
                                                                                    g_a->value.z_accel, 
                                                                                    g_a->value.x_gyro * gyro_sensitivity,
                                                                                    g_a->value.y_gyro * gyro_sensitivity,
                                                                                    g_a->value.z_gyro * gyro_sensitivity );
            UART_write_string( uart_interface, "Angles complementary: %f, %f\n",    complementary_angles.pitch, 
                                                                                    complementary_angles.roll );
            UART_write_string( uart_interface, "Angles madgwick: %f, %f\n",         madgwick_angles.pitch, 
                                                                                    madgwick_angles.roll );
        }
    }
    
    UART_write_string( uart_interface, "\nMax reading time = %ld us\n", max_mpu_time );
    UART_write_string( uart_interface, "Max complementary time = %ld us\n", max_time_complementary );
    UART_write_string( uart_interface, "Max madgwick time = %ld us\n", max_time_madgwick );
}


void process_inv_sqrt_benchmark ( void )
{
#define COUNT_INV_SQRT_LIMIT    65000.0
#define COUNT_DELTA             1

    uint32_t time_elapsed_us = 0;
    uint32_t max_time_inv_sqrt = 0;
    uint32_t max_time_opt_inv_sqrt = 0;
    
    float curr_error    = 0;
    float max_error     = 0;
    float prev_error    = 0;
    float value         = 0;
    
    for ( value = COUNT_DELTA; value < COUNT_INV_SQRT_LIMIT; value += COUNT_DELTA )
    {
        uint16_t int_val = value;
        bool show_data = false;
        if ( int_val % 1000 == 0 )
            show_data = true;
        
        timer_start();
        float inv_sqrt_val = 1.0/sqrt( value );
        timer_stop();
        time_elapsed_us = timer_get_us();
        max_time_inv_sqrt = max( max_time_inv_sqrt, time_elapsed_us );
        
        timer_start();
        float opt_inv_sqrt_val = inv_sqrt( value );
        timer_stop();
        time_elapsed_us = timer_get_us();
        max_time_opt_inv_sqrt = max( max_time_opt_inv_sqrt, time_elapsed_us );

        curr_error = inv_sqrt_val - opt_inv_sqrt_val;
        prev_error = max_error;
        max_error = max( max_error, curr_error );
        
        
        if ( show_data || prev_error != max_error )
            UART_write_string( uart_interface, "Data: %f -> %f / %f -> %f\n", value, inv_sqrt_val, opt_inv_sqrt_val, curr_error );
    }
        
    UART_write_string( uart_interface, "Max manual time = %ld us\n", max_time_inv_sqrt );
    UART_write_string( uart_interface, "Max optimized time = %ld us\n", max_time_opt_inv_sqrt );
    UART_write_string( uart_interface, "Max error = %f\n", max_error );
    
    
}
