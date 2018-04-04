#include <core.h>

#include <MPU6050.h>
#include <filters.h>

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


static SerialDriver         *debug_serial = &SD4;
BaseSequentialStream        *debug_stream = NULL;
static const SerialConfig   sdcfg = {
    .speed      = 460800    ,
    .cr1        = 0,
    .cr2        = USART_CR2_LINEN,
    .cr3        = 0
};

/*===========================================================================*/
/* EXT driver related.                                                       */
/*===========================================================================*/

static thread_t *mpu_drdy_tp;

static void mpu_drdy_cb( EXTDriver *extp, expchannel_t channel )
{
   (void)extp;
   (void)channel;

   chSysLockFromISR();                   // ISR Critical Area
   chEvtSignalI(mpu_drdy_tp, (eventmask_t)1);
   chSysUnlockFromISR();                 // Close ISR Critical Area

}

static EXTConfig extcfg =
{
   {
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
  }
};

/*===========================================================================*/
/* System.                                                                   */
/*===========================================================================*/

static euler_angles_t               euler_angles    = { 0, 0, 0 };
static euler_angles_t               gyro_rates      = { 0, 0, 0 };

int16_t pitch_setpoint      = 0;
int16_t roll_setpoint       = 0;
int16_t yaw_setpoint        = 0;



static motor_power_t    *motor_powers       = NULL;

// #define ANGLE_ADJUST_RATE           50.0f
// Max angular speed = 40 deg/sec   (1000/25)
static const float CONTROL_2_ANGLE_SPEED_RATE   = 2.0f;
static const float CONTROL_2_ANGLE_RATE         = 0;        // ????



static THD_WORKING_AREA(waControlThread, 128);
static THD_FUNCTION(ControlThread, arg)
{
    arg = arg;

    time_measurement_t      control_tm;
    chTMObjectInit( &control_tm );

    int16_t pitch_control       = 0;
    int16_t roll_control        = 0;
    int16_t yaw_control         = 0;

    float pitch_rate_setpoint   = 0;
    float roll_rate_setpoint    = 0;
    float yaw_rate_setpoint     = 0;

    systime_t time = chVTGetSystemTimeX();

    while ( 1 )
    {
        /* 2 - 6 us */
        // chTMStartMeasurementX( &control_tm );
        time += MS2ST( 5 );

        pitch_rate_setpoint = (pitch_setpoint * CONTROL_2_ANGLE_RATE - euler_angles.pitch) * CONTROL_2_ANGLE_SPEED_RATE;
        pitch_control       = PID_controller_generate_pitch_control( pitch_rate_setpoint - gyro_rates.pitch, gyro_rates.pitch );

        roll_rate_setpoint  = (roll_setpoint * CONTROL_2_ANGLE_RATE - euler_angles.roll) * CONTROL_2_ANGLE_SPEED_RATE;        
        roll_control        = PID_controller_generate_roll_control( roll_rate_setpoint - gyro_rates.roll, gyro_rates.roll );

        yaw_rate_setpoint   = yaw_setpoint * CONTROL_2_ANGLE_SPEED_RATE;        
        yaw_control         = PID_controller_generate_yaw_control( yaw_rate_setpoint - gyro_rates.yaw );


        /* Processing conversion */
        int32_t motorPower  = 0; //MOTOR_INPUT_MAX / 2;
        int32_t power       = 0;
        
        power = motorPower + pitch_control - roll_control - yaw_control;
        motor_powers[MOTOR_1] = clip_value( power, MOTOR_INPUT_MIN, MOTOR_INPUT_MAX );

        power = motorPower + pitch_control + roll_control + yaw_control;
        motor_powers[MOTOR_2] = clip_value( power, MOTOR_INPUT_MIN, MOTOR_INPUT_MAX );

        power = motorPower - pitch_control + roll_control - yaw_control;
        motor_powers[MOTOR_3] = clip_value( power, MOTOR_INPUT_MIN, MOTOR_INPUT_MAX );

        power = motorPower - pitch_control - roll_control + yaw_control;
        motor_powers[MOTOR_4] = clip_value( power, MOTOR_INPUT_MIN, MOTOR_INPUT_MAX );


        motor_control_update_PWM();
        // chTMStopMeasurementX( &control_tm );

        chprintf( debug_stream, "Pitch error: %06d %06d %06d\n", (int)((pitch_rate_setpoint - gyro_rates.pitch)), 
                                                                 (int)(pitch_rate_setpoint), (int)(gyro_rates.pitch) );

        // chprintf( debug_stream, "Best: %d, Worst: %d\n", RTC2US(SYSTEM_FREQUENCY, control_tm.best), RTC2US(SYSTEM_FREQUENCY, control_tm.worst) );
        // chprintf( debug_stream, "Rates SPs: %d, %d, %d\n", 
        //                             (int)(roll_rate_setpoint * INT_ACCURACY_MULTIPLIER), 
        //                             (int)(pitch_rate_setpoint * INT_ACCURACY_MULTIPLIER), 
        //                             (int)(yaw_rate_setpoint * INT_ACCURACY_MULTIPLIER) );

        // chprintf( debug_stream, "Controls: %d, %d, %d\n", roll_control, pitch_control, yaw_control );

        chThdSleepUntil( time );
    }
}



/* 200 Hz */
const  float            SAMPLE_PERIOD_S     = 5.0/1000;

static float                        gyro_sensitivity; 
static gy_521_gyro_accel_data_t     *mpu_data;

static thread_reference_t           sender_ref      = NULL;

static THD_WORKING_AREA(waAHRSThread, 128);
static THD_FUNCTION(AHRSThread, arg)
{
    arg = arg;

    time_measurement_t      filter_tm;
    chTMObjectInit( &filter_tm );

    mpu6050_set_bandwidth( MPU6050_DLPF_BW_42 );
    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_500 );
    mpu6050_set_accel_fullscale( MPU6050_ACCEL_FS_2 );
    mpu6050_set_bypass_mode( true );
    mpu6050_set_interrupt_data_rdy_bit( true );
    mpu6050_set_sample_rate_divider( 4 );

    mpu_data            = mpu6050_get_raw_data();
    gyro_sensitivity    = mpu6050_get_gyro_sensitivity_rate();

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

        // chTMStartMeasurementX( &filter_tm );
        /* 376 - 377 us */
        mpu6050_receive_gyro_accel_raw_data();
        // chTMStopMeasurementX( &filter_tm );

        filter_input.acc_x = mpu_data->x_accel;
        filter_input.acc_y = mpu_data->y_accel;
        filter_input.acc_z = mpu_data->z_accel;
        filter_input.gyr_x = mpu_data->x_gyro * gyro_sensitivity;
        filter_input.gyr_y = mpu_data->y_gyro * gyro_sensitivity;
        filter_input.gyr_z = mpu_data->z_gyro * gyro_sensitivity;
        
        // chTMStartMeasurementX( &filter_tm );
        
        /* 79 - 102 us */
        madgwick_filter_position_execute( &filter_input, &euler_angles );

        /* 1 - 57 us */
        // complementary_filter_position_execute( &filter_input, &euler_angles );

        // chTMStopMeasurementX( &filter_tm );


        lowpass_filter_velocity_execute( &filter_input, &gyro_rates );

        // chprintf( debug_stream, "Best: %d, Worst: %d\n", RTC2US(SYSTEM_FREQUENCY, filter_tm.best), RTC2US(SYSTEM_FREQUENCY, filter_tm.worst) );

        // chprintf( debug_stream, "Angls: %06d %06d %06d\n", 
                                    // (int)(euler_angles.roll * INT_ACCURACY_MULTIPLIER), 
                                    // (int)(euler_angles.pitch * INT_ACCURACY_MULTIPLIER), 
                                    // (int)(euler_angles.yaw * INT_ACCURACY_MULTIPLIER) );

        // chprintf( debug_stream, "Rates: %06d %06d %06d\n", 
        //                             (int)(gyro_rates.roll * INT_ACCURACY_MULTIPLIER), 
        //                             (int)(gyro_rates.pitch * INT_ACCURACY_MULTIPLIER), 
        //                             (int)(gyro_rates.yaw * INT_ACCURACY_MULTIPLIER) );

        // chprintf( debug_stream, "Accel: %06d %06d %06d\n", mpu_data->x_accel, mpu_data->y_accel, mpu_data->z_accel );
        // chprintf( debug_stream, "Gyro : %06d %06d %06d\n", mpu_data->x_gyro, mpu_data->y_gyro, mpu_data->z_gyro );
    

    }
}



/*===========================================================================*/
/* Application code                                                          */
/*===========================================================================*/

/*
 *  MPU6050 - 14 bytes receive time = 407us (400kHz) / 1.56ms (100 kHz)
 */

// #define MPU6050_CALIBRATION

static const mpu6050_offsets_t mpu_offsets = {
    -1750,
    -626,
    1295,
    50,
    -23,
    -1
};


int main(void)
{
    chSysInit();
    halInit();

    /* I2C hardware init */
    palSetPadMode( GPIOB, 8, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN );    // SCL = PB_8
    palSetPadMode( GPIOB, 9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN );    // SDA = PB_9
    i2cStart( mpudrvr, &mpuconf );

    palSetPadMode( GPIOA, 0, PAL_MODE_ALTERNATE(8) );    // TX = PA_0
    palSetPadMode( GPIOA, 1, PAL_MODE_ALTERNATE(8) );    // RX = PA_1

    sdStart( debug_serial, &sdcfg );
    debug_stream = (BaseSequentialStream *)debug_serial;

    /* AHRS EXT init */
    extcfg.channels[5].mode = EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC;
    extcfg.channels[5].cb   = mpu_drdy_cb;

    /* AHRS initialization */
    if ( mpu6050_init( mpudrvr ) != EOK )
    {
        const char *reason = "MPU6050 init failed\n"; 
        chprintf( debug_stream, reason );
        chSysHalt( reason );
    }

    chprintf( debug_stream, "MPU6050 ready\n" );

#ifdef MPU6050_CALIBRATION
    chprintf( debug_stream, "MPU6050 calibration start\n" );
    mpu6050_calibration();
    chSysHalt( "Calibration mode" );
#else
    mpu6050_set_offsets( &mpu_offsets );
#endif

    extStart( &EXTD1, &extcfg );
    chThdCreateStatic( waAHRSThread, sizeof(waAHRSThread), NORMALPRIO, AHRSThread, NULL );

    /* Motor control init */
    motor_control_init();
    motor_powers = motor_control_get_powers_ptr();



    chThdCreateStatic( waControlThread, sizeof(waControlThread), NORMALPRIO, ControlThread, NULL );

    while ( true )
    {
        chThdSleepMilliseconds( 500 );
        // chThdSuspend( &sender_ref );

        if ( !palReadLine( LINE_BUTTON ) )
        {
            // palToggleLine( LINE_LED1 );
            palTogglePad( GPIOA, 5 );

            if ( motor_control_is_armed() )
                motor_control_set_motors_stopped();
            else
                motor_control_set_motors_started(); 
        }

        // palSetPad( GPIOC, 8 );

        // msg_t msg = i2cMasterTransmitTimeout( mpudrvr, 0b1101000, tx_buf, sizeof( tx_buf ), rx_buf, 14, MS2ST(10) );
        
        // palClearPad( GPIOC, 8 );

        // if ( msg == MSG_OK )
            // chprintf( debug_stream, "Ok, 0x%x =)\n", rx_buf[0] );
        // else
            // chprintf( debug_stream, "Not Ok %d 0x%x =)\n", msg, i2cGetErrors( mpudrvr ) );

        // palTogglePad( GPIOA, 5 );

        // chprintf( debug_stream, "Hello =)\n" );

    }
}
