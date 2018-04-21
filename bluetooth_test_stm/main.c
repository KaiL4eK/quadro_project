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


/********** COMMON CONFIGURATION **********/

#define FRAME_START         0x7E
#define FRAME_END           FRAME_START

typedef enum {
    COMMAND_EMPTY           = 0x00,
    COMMAND_CONNECT,

} frame_command_t;


typedef struct {
    /* [ rp[P, I, D], y[P, I, D] ] */
    struct {
            float pidRates[3];
    } controlSysteRates[2];

} connect_response_t;

/********** COMMON CONFIGURATION END **********/


typedef enum {
    WAIT_4_START,
    WAIT_4_COMMAND

} receive_mode_t;

void processConnectCommand( void )
{
    connect_response_t response = { .controlSysteRates = { 
                                        { .pidRates = { 10.2, 5.3, 0.01 } }, 
                                        { .pidRates = { 11.1, 6.5, 0.003 } } 
                                                         } 
                                  };


    sdWrite( btDriver, (uint8_t *)&response, sizeof(response) );
}

static THD_WORKING_AREA(waBluetoothSerial, 128);
static THD_FUNCTION(BluetoothSerial, arg) 
{
    arg = arg;

    receive_mode_t  receive_mode = WAIT_4_START;
    frame_command_t received_cmd;
    uint8_t received_byte = 0;

    while (true)
    {
        msg_t msg = sdGet( btDriver );

        received_byte = msg;

        chprintf( (BaseSequentialStream *)uartDriver, "Got: 0x%x / %c\n", received_byte, received_byte );
        // sdPut( uartDriver, received_byte );

        switch ( receive_mode )
        {
            case WAIT_4_START:

                if ( received_byte == FRAME_START )
                {
                    receive_mode = WAIT_4_COMMAND;
                    chprintf( (BaseSequentialStream *)uartDriver, "Wait 4 cmd\n" );
                }  

                break;

            case WAIT_4_COMMAND:

                received_cmd = received_byte;

                switch ( received_cmd )
                {
                    case COMMAND_CONNECT:
                        chprintf( (BaseSequentialStream *)uartDriver, "Got connect cmd\n" );
                        processConnectCommand();
                        break;

                    default:
                        receive_mode = WAIT_4_START;
                }

                break;

            default:
                ;
        }

        palToggleLine( LINE_LED1 );
        // chThdSleepMilliseconds( 300 );
    }
}


static THD_WORKING_AREA(waUARTSerial, 128);
static THD_FUNCTION(UARTSerial, arg) 
{
    arg = arg;

    uint8_t received_byte = 0;

    while (true)
    {
        msg_t msg = sdGet( uartDriver );

        received_byte = msg;

        sdPut( btDriver, received_byte );

        palToggleLine( LINE_LED2 );
        // chThdSleepMilliseconds( 300 );
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
    chThdCreateStatic(waUARTSerial, sizeof(waUARTSerial), NORMALPRIO, UARTSerial, NULL);

    while (true)
    {
        chThdSleepSeconds(1);
    }
}
