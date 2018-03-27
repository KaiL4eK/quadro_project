#include <core.h>

#include <MPU6050.h>

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
    .speed      = 115200,
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

static float                        gyro_sensitivity; 
static gy_521_gyro_accel_data_t     *mpu_data;

static THD_WORKING_AREA(waAccelGyro, 128); // 128 - stack size
static THD_FUNCTION(AccelGyro, arg)
{
    arg = arg;

    mpu_data            = mpu6050_get_raw_data();
    gyro_sensitivity    = mpu6050_get_gyro_sensitivity_rate();

    mpu_drdy_tp         = chThdGetSelfX();

    while ( 1 )
    {
        chEvtWaitAny((eventmask_t)1);

        mpu6050_receive_gyro_accel_raw_data();
    
        // chprintf( debug_stream, "Accel: %06d %06d %06d\n", mpu_data->x_accel, mpu_data->y_accel, mpu_data->z_accel );
        // chprintf( debug_stream, "Gyro : %06d %06d %06d\n", mpu_data->x_gyro, mpu_data->y_gyro, mpu_data->z_gyro );
    

    }
}

/*===========================================================================*/
/* Application code                                                          */
/*===========================================================================*/

/*
 *  MPU6050 - 14 bytes receive time = 407us (400kHz) / 1.56ms (100 kHz)
 *
 */

// #define MPU6050_CALIBRATION

static const mpu6050_offsets_t mpu_offsets = {
    -1772,
    -653,
    1412,
    37,
    -20,
    -9
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
    palSetPadMode( GPIOC, 5, PAL_MODE_OUTPUT_PUSHPULL );

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
#else
    mpu6050_set_offsets( &mpu_offsets );
#endif

    mpu6050_set_bandwidth( MPU6050_DLPF_BW_42 );
    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_500 );
    mpu6050_set_accel_fullscale( MPU6050_ACCEL_FS_2 );
    mpu6050_set_bypass_mode( true );
    mpu6050_set_interrupt_data_rdy_bit( true );
    mpu6050_set_sample_rate_divider( 3 );

    extStart( &EXTD1, &extcfg );
    chThdCreateStatic( waAccelGyro, sizeof(waAccelGyro), NORMALPRIO, AccelGyro, NULL );

    /* PWM for motor control */
    PWMDriver *pwmMotorDriver      = &PWMD3;
    palSetPadMode( GPIOA, 6, PAL_MODE_ALTERNATE(2) );
    palSetPadMode( GPIOC, 7, PAL_MODE_ALTERNATE(2) );
    palSetPadMode( GPIOC, 8, PAL_MODE_ALTERNATE(2) );
    palSetPadMode( GPIOC, 9, PAL_MODE_ALTERNATE(2) );

    PWMConfig pwm3conf = {
        .frequency = 1000000,
        .period    = 4000, /* 4/1000 s = 4 ms = 250 Hz */
        .callback  = NULL,
        .channels  = {
                      {PWM_OUTPUT_ACTIVE_HIGH, NULL},
                      {PWM_OUTPUT_ACTIVE_HIGH, NULL},
                      {PWM_OUTPUT_ACTIVE_HIGH, NULL},
                      {PWM_OUTPUT_ACTIVE_HIGH, NULL}
                      },
        .cr2        = 0,
        .dier       = 0
    };

    pwmStart( pwmMotorDriver, &pwm3conf );

    pwmEnableChannel( pwmMotorDriver, 0, 900 );
    pwmEnableChannel( pwmMotorDriver, 1, 900 );
    pwmEnableChannel( pwmMotorDriver, 2, 900 );
    pwmEnableChannel( pwmMotorDriver, 3, 900 );

    while ( true )
    {
        chThdSleepMilliseconds( 500 );

        // palSetPad( GPIOC, 8 );

        // msg_t msg = i2cMasterTransmitTimeout( mpudrvr, 0b1101000, tx_buf, sizeof( tx_buf ), rx_buf, 14, MS2ST(10) );
        
        // palClearPad( GPIOC, 8 );

        // if ( msg == MSG_OK )
            // chprintf( debug_stream, "Ok, 0x%x =)\n", rx_buf[0] );
        // else
            // chprintf( debug_stream, "Not Ok %d 0x%x =)\n", msg, i2cGetErrors( mpudrvr ) );

        palTogglePad( GPIOA, 5 );

        // chprintf( debug_stream, "Hello =)\n" );

    }
}
