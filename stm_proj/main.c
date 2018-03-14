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

    palSetPadMode( GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL );

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

    while ( true )
    {
        chThdSleepMilliseconds( 500 );

        gy_521_gyro_accel_data_t *mpu_data = mpu6050_get_raw_data();

        mpu6050_receive_gyro_accel_raw_data();

        chprintf( debug_stream, "Accel: %06d %06d %06d\n", mpu_data->x_accel, mpu_data->y_accel, mpu_data->z_accel );

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
