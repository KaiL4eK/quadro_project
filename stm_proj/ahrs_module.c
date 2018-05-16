#define CORE_DEBUG_ENABLED
#include <core.h>

#define AHRS_PREFIX     "[ahrs]: "

euler_angles_t               euler_angles    = { 0, 0, 0 };
euler_angles_t               gyro_rates      = { 0, 0, 0 };


/*===========================================================================*/
/* MPU6050 driver related.                                                   */
/*===========================================================================*/

static thread_t *mpu_drdy_tp;

static void mpu_drdy_cb( EXTDriver *extp, expchannel_t channel )
{
   (void)extp;
   (void)channel;

   chSysLockFromISR();
   chEvtSignalI(mpu_drdy_tp, (eventmask_t)1);
   chSysUnlockFromISR();

}

/* 200 Hz */
const  float                        SAMPLE_PERIOD_S     = CONTROL_SYSTEM_PERIOD_MS/1000.0;

static float                        gyro_sensitivity; 
static gy_521_gyro_accel_data_t     *mpu_data;

static THD_WORKING_AREA(waAHRSThread, 1024);
static THD_FUNCTION(AHRSThread, arg)
{
    arg = arg;

#ifdef AHRS_TIME_MEASUREMENT_DEBUG
    time_measurement_t      filter_tm;
    chTMObjectInit( &filter_tm );
#endif

    mpu_drdy_tp         = chThdGetSelfX();

    /* Filters */
    madgwick_filter_set_angle_rate( 0.5f );
    complementary_filter_set_angle_rate( 0.995f );
    lowpass_filter_set_velocity_rate( 0.7f );
    filter_initialize( SAMPLE_PERIOD_S );

    madgwick_filter_set_inv_sqrt_method_manual( true );

    imu_filter_input_t  filter_input;

    while ( 1 )
    {
        chEvtWaitAny((eventmask_t)1);

#ifdef AHRS_TIME_MEASUREMENT_DEBUG
        chTMStartMeasurementX( &filter_tm );
#endif

        /* 376 - 377 us */
        mpu6050_receive_gyro_accel_raw_data();

#ifdef AHRS_TIME_MEASUREMENT_DEBUG
        chTMStopMeasurementX( &filter_tm );
#endif

        filter_input.acc_x = mpu_data->x_accel;
        filter_input.acc_y = mpu_data->y_accel;
        filter_input.acc_z = mpu_data->z_accel;
        filter_input.gyr_x = mpu_data->x_gyro * gyro_sensitivity;
        filter_input.gyr_y = mpu_data->y_gyro * gyro_sensitivity;
        filter_input.gyr_z = mpu_data->z_gyro * gyro_sensitivity;
        
#ifdef AHRS_TIME_MEASUREMENT_DEBUG
        chTMStartMeasurementX( &filter_tm );
#endif

        /* 79 - 102 us */
        // madgwick_filter_position_execute( &filter_input, &euler_angles );

        /* 1 - 57 us */
        complementary_filter_position_execute( &filter_input, &euler_angles );

#ifdef AHRS_TIME_MEASUREMENT_DEBUG
        chTMStopMeasurementX( &filter_tm );
        dprintf( "Best: %d, Worst: %d\n", RTC2US(SYSTEM_FREQUENCY, filter_tm.best), RTC2US(SYSTEM_FREQUENCY, filter_tm.worst) );
#endif

        lowpass_filter_velocity_execute( &filter_input, &gyro_rates );

// dprintf( "Rates: %06d %06d %06d\n",
//                             (int)(gyro_rates.roll * INT_ACCURACY_MULTIPLIER),
//                             (int)(gyro_rates.pitch * INT_ACCURACY_MULTIPLIER),
//                             (int)(gyro_rates.yaw * INT_ACCURACY_MULTIPLIER) );

// dprintf( "Accel: %06d %06d %06d\n", mpu_data->x_accel, mpu_data->y_accel, mpu_data->z_accel );
// dprintf( "Gyro : %06d %06d %06d\n", mpu_data->x_gyro, mpu_data->y_gyro, mpu_data->z_gyro );
    

    }
}

/*
 *  MPU6050 - 14 bytes receive time = 407us (400kHz) / 1.56ms (100 kHz)
 */

static const mpu6050_offsets_t mpu_offsets = {
    -1750,
    -626,
    1295,
    50,
    -23,
    -1
};


/* I2C Hardware config */
static I2CDriver            *mpudrvr    = &I2CD1;
static const I2CConfig      mpuconf     = {
#ifndef F446RE
    .timingr  = STM32_TIMINGR_PRESC(0U) |
                STM32_TIMINGR_SCLDEL(11U) | STM32_TIMINGR_SDADEL(0U) |
                // STM32_TIMINGR_SCLH(61U)  | STM32_TIMINGR_SCLL(61U),
                STM32_TIMINGR_SCLH(48U)  | STM32_TIMINGR_SCLL(74U),
    .cr1      = 0,
    .cr2      = 0
#else
    .op_mode        = OPMODE_I2C,
    .clock_speed    = 400000,
    // .duty_cycle     = STD_DUTY_CYCLE
    .duty_cycle     = FAST_DUTY_CYCLE_2
#endif
};

void ahrs_module_init ( tprio_t prio )
{
    /* I2C hardware init */
    palSetPadMode( GPIOB, 8, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN );    // SCL = PB_8
    palSetPadMode( GPIOB, 9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN );    // SDA = PB_9
    i2cStart( mpudrvr, &mpuconf );

    /* AHRS initialization */
    if ( mpu6050_init( mpudrvr ) != EOK )
    {
        const char *reason = "MPU6050 init failed\n"; 
        dprintf( reason );
        chSysHalt( reason );
    }
    
    mpu6050_set_offsets( &mpu_offsets );

    mpu6050_set_bandwidth( MPU6050_DLPF_BW_42 );
    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_500 );
    mpu6050_set_accel_fullscale( MPU6050_ACCEL_FS_2 );
    mpu6050_set_bypass_mode( true );
    mpu6050_set_interrupt_data_rdy_bit( true );
    mpu6050_set_sample_rate_divider( 4 );

    mpu_data            = mpu6050_get_raw_data();
    gyro_sensitivity    = mpu6050_get_gyro_sensitivity_rate();

    dprintf_mod( AHRS_PREFIX, "MPU6050 ready\n" );

    /* AHRS EXT init */
    /* Enable EXT on PC5 pin - MPU6050 */
    EXTChannelConfig ch_cfg = { .mode = EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC, .cb = mpu_drdy_cb };
    extSetChannelMode( &EXTD1, 5, &ch_cfg );

    /* AHRS Thread init */
    chThdCreateStatic( waAHRSThread, sizeof(waAHRSThread), prio, AHRSThread, NULL );


}
