#include <ch.h>
#include <hal.h>

#include <chprintf.h>

static SerialDriver *btDriver    = &SD5;
static SerialDriver *uartDriver  = &SD7;

static const SerialConfig uartCfg = {
  .speed = 460800,
  .cr1 = 0,
  .cr2 = USART_CR2_LINEN,
  .cr3 = 0
};


static const SerialConfig btCfg = {
  .speed = 115200,
  .cr1 = 0,
  .cr2 = USART_CR2_LINEN,
  .cr3 = 0
};

static THD_WORKING_AREA(waBluetoothSerial, 128);
static THD_FUNCTION(BluetoothSerial, arg) 
{
    arg = arg;

    uint8_t received_byte = 0;

    while (true)
    {
        msg_t msg = sdGet( btDriver );

        received_byte = msg;

        sdPut( uartDriver, received_byte );

        palToggleLine( LINE_LED1 );
        chThdSleepMilliseconds( 300 );
    }
}


int main(void)
{
    chSysInit();
    halInit();

    /* Setup Bluetooth serial */
    sdStart( btDriver, &btCfg );
    palSetPadMode( GPIOE, 8, PAL_MODE_ALTERNATE(8) );    // TX
    palSetPadMode( GPIOE, 7, PAL_MODE_ALTERNATE(8) );    // RX
 
    /* Setup wired serial */
    sdStart( uartDriver, &uartCfg );
    palSetPadMode( GPIOC, 12, PAL_MODE_ALTERNATE(8) );    // TX
    palSetPadMode( GPIOD, 2, PAL_MODE_ALTERNATE(8) );    // RX

    chThdCreateStatic(waBluetoothSerial, sizeof(waBluetoothSerial), NORMALPRIO, BluetoothSerial, NULL);

    while (true)
    {
        chThdSleepSeconds(1);
    }
}
