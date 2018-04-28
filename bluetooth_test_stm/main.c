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

#define COMMAND_CONNECT         (1 << 0)
#define COMMAND_DATA_START      (1 << 1)
#define COMMAND_DATA_STOP       (1 << 2)
#define COMMAND_DATA_PACK       (1 << 3)
#define COMMAND_PING            (1 << 4)
#define COMMAND_SET_MAIN_RATES  (1 << 5)
#define COMMAND_SET_YAW_RATES   (1 << 6)

typedef uint8_t frame_command_t;

typedef struct {
    float       rates[3];
} pid_rates_t;

typedef struct {
    /* [ rp[P, I, D], y[P, I, D] ] */
    pid_rates_t PIDRates[2];

} connect_response_t;

typedef struct {
    float       roll;
} data_package_t;

typedef struct {
    bool        succeed;
} response_ack_t;

uint8_t calcChksum( uint8_t *data, uint8_t len )
{
    uint16_t data_byte;
    uint16_t crc = 0;

    while ( len-- )
    {
        data_byte = *data++;

        crc = (crc << 2) + crc + data_byte;
        crc = (crc << 2) + crc + data_byte;
        crc = crc ^ (crc >> 8);
    }

    return (crc & 0xFF);
}

typedef enum {
    WAIT_4_START,
    WAIT_4_COMMAND,
    WAIT_4_DATA,
    WAIT_4_CKSUM

} receive_mode_t;

/********** COMMON CONFIGURATION END **********/



bool            is_data_transfering     = false;
virtual_timer_t watchdog_vt;

void watchdogTimerHandler ( void *arg )
{
    arg = arg;

    palSetLine( LINE_LED1 );

    chprintf( (BaseSequentialStream *)uartDriver, "Got timeout\n" );
}

void startDataTransfer ( void )
{
    is_data_transfering = true;
    
    /* Timeout 3 seconds */
    chVTSet( &watchdog_vt, S2ST( 3 ), watchdogTimerHandler, NULL );

    palClearLine( LINE_LED1 );
    palSetLine( LINE_LED2 );
}

void resetDataTransfer ( void )
{
    startDataTransfer();
}

void stopDataTransfer ( void )
{
    is_data_transfering = false;

    chVTReset( &watchdog_vt );

    palClearLine( LINE_LED2 );
}

pid_rates_t main_rates  = { .rates = { 10.2, 5.3, 0.01 } };
pid_rates_t yaw_rates   = { .rates = { 11.1, 6.5, 0.003 } };


void responseConnectCommand ( frame_command_t cmd )
{
    connect_response_t response = { .PIDRates = { main_rates, yaw_rates } };

    sdPut( btDriver, FRAME_START );
    sdPut( btDriver, cmd );

    sdWrite( btDriver, (uint8_t *)&response, sizeof(response) );

    uint8_t cksum = calcChksum( (void *)&response, sizeof(response) );

    sdPut( btDriver, cksum );
}


void responseAck ( frame_command_t cmd, bool set )
{
    response_ack_t response = { .succeed = set };

    sdPut( btDriver, FRAME_START );
    sdPut( btDriver, cmd );

    sdWrite( btDriver, (uint8_t *)&response, sizeof(response) );
}

void sendDataPackage ( frame_command_t cmd, data_package_t *data )
{
    sdPut( btDriver, FRAME_START );
    sdPut( btDriver, cmd );

    sdWrite( btDriver, (uint8_t *)data, sizeof(*data) );
}

static THD_WORKING_AREA(waBluetoothSerial, 512);
static THD_FUNCTION(BluetoothSerial, arg) 
{
    arg = arg;

    receive_mode_t  receive_mode        = WAIT_4_START;
    frame_command_t received_cmd;

    uint16_t        receive_data_length = 0;    
    uint8_t         received_byte       = 0;

    uint8_t         data_buffer[255];
    uint8_t         data_idx            = 0;

    uint8_t         cksum               = 0;

    chVTObjectInit( &watchdog_vt );

    /* Do we need to check if device was connected? */

    while (true)
    {
        msg_t msg = sdGet( btDriver );

        received_byte = msg;

        // chprintf( (BaseSequentialStream *)uartDriver, "Got: 0x%x / %c\n", received_byte, received_byte );
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
                        responseConnectCommand( COMMAND_CONNECT );
                        receive_mode = WAIT_4_START;
                        break;

                    case COMMAND_PING:
                        chprintf( (BaseSequentialStream *)uartDriver, "Got ping cmd\n" );
                        responseAck( COMMAND_PING, true );
                        receive_mode = WAIT_4_START;
                        
                        resetDataTransfer();
                        break;

                    case COMMAND_DATA_START:
                        chprintf( (BaseSequentialStream *)uartDriver, "Got data start cmd\n" );
                        responseAck( COMMAND_DATA_START, true );
                        receive_mode = WAIT_4_START;
                        
                        startDataTransfer();
                        break;


                    case COMMAND_DATA_STOP:
                        chprintf( (BaseSequentialStream *)uartDriver, "Got data stop cmd\n" );
                        responseAck( COMMAND_DATA_STOP, true );
                        receive_mode = WAIT_4_START;
                        
                        stopDataTransfer();
                        break;

                    case COMMAND_SET_MAIN_RATES:
                        chprintf( (BaseSequentialStream *)uartDriver, "Got set main rates cmd\n" );
                        receive_data_length = sizeof( pid_rates_t );
                        receive_mode = WAIT_4_DATA;
                        data_idx = 0;

                        break;

                    case COMMAND_SET_YAW_RATES:
                        chprintf( (BaseSequentialStream *)uartDriver, "Got set yaw rates cmd\n" );
                        receive_data_length = sizeof( pid_rates_t );
                        receive_mode = WAIT_4_DATA;
                        data_idx = 0;

                        break;

                    default:
                        chprintf( (BaseSequentialStream *)uartDriver, "Unknown cmd\n" );
                        receive_mode = WAIT_4_START;
                }

                break;

            case WAIT_4_DATA:

                data_buffer[data_idx++] = received_byte;

                if ( data_idx >= receive_data_length )
                    receive_mode = WAIT_4_CKSUM;

                break;

            case WAIT_4_CKSUM:

                cksum = received_byte;

                uint8_t calc_cksum = calcChksum( data_buffer, receive_data_length );

                if ( calc_cksum == cksum )
                {
                    responseAck ( received_cmd, true );
                }
                else
                {
                    responseAck ( received_cmd, false );
                    chprintf( (BaseSequentialStream *)uartDriver, "Invalid cksum\n" );
                }

                receive_mode = WAIT_4_START;

                if ( received_cmd == COMMAND_SET_MAIN_RATES )
                {
                    pid_rates_t *rates = (pid_rates_t *)data_buffer;

                    main_rates = *rates;
                    chprintf( (BaseSequentialStream *)uartDriver, "Main rates: %d, %d, %d\n", (int)(rates->rates[0] * 1000), 
                                                                                              (int)(rates->rates[1] * 1000), 
                                                                                              (int)(rates->rates[2] * 1000) );
                }
                else if ( received_cmd == COMMAND_SET_YAW_RATES )
                {
                    pid_rates_t *rates = (pid_rates_t *)data_buffer;

                    yaw_rates = *rates;
                    chprintf( (BaseSequentialStream *)uartDriver, "Yaw rates: %d, %d, %d\n", (int)(rates->rates[0] * 1000), 
                                                                                             (int)(rates->rates[1] * 1000), 
                                                                                             (int)(rates->rates[2] * 1000) );
                }

                break;

            default:
                ;
        }

        
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

        // chThdSleepMilliseconds( 300 );
    }
}

static THD_WORKING_AREA(waBlinker, 16);
static THD_FUNCTION(Blinker, arg) 
{
    arg = arg;

    systime_t time = chVTGetSystemTimeX();

    while ( true )
    {
        time += MS2ST( 500 );
        palToggleLine( LINE_LED3 );
        chThdSleepUntil( time );
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
    chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO, Blinker, NULL);

    while (true)
    {
        chThdSleepSeconds(1);
    }
}
