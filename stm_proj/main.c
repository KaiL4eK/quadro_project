#include <ch.h>
#include <hal.h>

#include <chprintf.h>

static I2CDriver            *mpudrvr    = &I2CD1;
static const I2CConfig      mpuconf     = {
    .timingr  = STM32_TIMINGR_PRESC(0U) |
                STM32_TIMINGR_SCLDEL(11U) | STM32_TIMINGR_SDADEL(0U) |
                // STM32_TIMINGR_SCLH(61U)  | STM32_TIMINGR_SCLL(61U),
                STM32_TIMINGR_SCLH(48U)  | STM32_TIMINGR_SCLL(74U),
    .cr1      = 0,
    .cr2      = 0
};


static SerialDriver         *debug_serial = &SD7;
static BaseSequentialStream *debug_stream = NULL;
static const SerialConfig   sdcfg = {
    .speed  = 115200,
    .cr1        = 0,
    .cr2        = USART_CR2_LINEN,
    .cr3        = 0
};

int main(void)
{
    chSysInit();
    halInit();

    /* I2C init */
    palSetPadMode( GPIOB, 8, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN );    // SCL = PB_8
    palSetPadMode( GPIOB, 9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN );    // SDA = PB_9
    i2cStart( mpudrvr, &mpuconf );

    /* Serial init */
    palSetPadMode( GPIOE, 8, PAL_MODE_ALTERNATE(8) );    // TX = PE_8
    palSetPadMode( GPIOE, 7, PAL_MODE_ALTERNATE(8) );    // RX = PE_7
    sdStart( debug_serial, &sdcfg );
    debug_stream = (BaseSequentialStream *)debug_serial;

    uint8_t tx_buf[] = { 0x75 };
    uint8_t rx_buf[2];

    while ( true )
    {
        chThdSleepMilliseconds( 500 );

        msg_t msg = i2cMasterTransmitTimeout( mpudrvr, 0b1101000, tx_buf, sizeof( tx_buf ), rx_buf, 1, MS2ST(50) );
        
        if ( msg == MSG_OK )
            chprintf( debug_stream, "Ok, 0x%x =)\n", rx_buf[0] );
        else
            chprintf( debug_stream, "Not Ok %d 0x%x =)\n", msg, i2cGetErrors( mpudrvr ) );

    }
}
