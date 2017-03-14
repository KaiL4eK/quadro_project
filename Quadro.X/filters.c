/*
 * File:   filters.c
 * Author: alex
 *
 * Created on March 14, 2017, 5:29 PM
 */

#include "filters.h"

static float m_sample_rate                      = 0;
static float gyro_rate_raw_2_deg_per_sec        = 0;
static float gyro_rate_raw_2_degree             = 0;

static void (* filter)( imu_filter_input_t *, euler_angles_t * );

void filter_initialize ( filter_type_t type, float sample_rate )
{
    if ( type == COMPLEMENTARY )
    {
        filter = complementary_filter_position_execute;
    } else {
        filter = madgwick_filter_position_execute;
    }
    
    m_sample_rate = sample_rate;
    gyro_rate_raw_2_deg_per_sec = 1 / mpu6050_get_gyro_sensitivity_rate();
    gyro_rate_raw_2_degree = m_sample_rate * gyro_rate_raw_2_deg_per_sec;
}

static float complementary_filter_angle_rate_a  = 0.95f;
static float complementary_filter_angle_rate_b  = 0.05f;

void complementary_filter_set_angle_rate( float rate_a )
{
    if ( rate_a >= 1.0f )
        return;
    
    complementary_filter_angle_rate_a = rate_a;
    complementary_filter_angle_rate_b = 1.0f - rate_a;
}

void complementary_filter_position_execute( imu_filter_input_t *g_a, euler_angles_t *angles )
{    
    float   accel_angle_roll    = 0;
    float   accel_angle_pitch   = 0;
    
    if ( g_a->acc_x != 0 && g_a->acc_y != 0 )
    {
        accel_angle_roll    = atan2(-g_a->acc_x, sqrt(g_a->acc_y * g_a->acc_y + g_a->acc_z * g_a->acc_z)) * RADIANS_TO_DEGREES;
        accel_angle_pitch   = atan2( g_a->acc_y, sqrt(g_a->acc_x * g_a->acc_x + g_a->acc_z * g_a->acc_z)) * RADIANS_TO_DEGREES;
    }
    
    angles->pitch   = (complementary_filter_angle_rate_a * (g_a->gyr_x * gyro_rate_raw_2_degree + angles.pitch)) + 
                            (complementary_filter_angle_rate_b * accel_angle_pitch);
    angles->roll    = (complementary_filter_angle_rate_a * (g_a->gyr_y * gyro_rate_raw_2_degree + angles.roll))  + 
                            (complementary_filter_angle_rate_b * accel_angle_roll);
    angles->yaw     =                                       g_a->gyr_z * gyro_rate_raw_2_degree + angles.yaw;
}

static float complementary_filter_velocity_rate_a = 0.7f;
static float complementary_filter_velocity_rate_b = 0.3f;

void complementary_filter_set_velocity_rate( float rate_a )
{
    if ( rate_a >= 1.0f )
        return;
    
    complementary_filter_velocity_rate_a = rate_a;
    complementary_filter_velocity_rate_b = 1.0f - rate_a;
}

void complementary_filter_velocity_execute ( imu_filter_input_t *g_a, euler_angles_t *velocity )
{
    velocity.pitch            = complementary_filter_velocity_rate_a * velocity.pitch + complementary_filter_velocity_rate_b * (g_a->gyr_x * gyro_rate_raw_2_deg_per_sec);
    velocity.roll             = complementary_filter_velocity_rate_a * velocity.roll  + complementary_filter_velocity_rate_b * (g_a->gyr_y * gyro_rate_raw_2_deg_per_sec);
    velocity.yaw              = complementary_filter_velocity_rate_a * velocity.yaw   + complementary_filter_velocity_rate_b * (g_a->gyr_z * gyro_rate_raw_2_deg_per_sec);    
}

void madgwick_filter_position_execute ( imu_filter_input_t *g_a, euler_angles_t *angles )
{
    
}



