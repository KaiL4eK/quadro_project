#define CORE_DEBUG_ENABLED
#include <core.h>

static SerialDriver         *btDriver       = &SD3;
static const SerialConfig btCfg = {
    .speed = 115200,
    .cr1 = 0,
    .cr2 = USART_CR2_LINEN,
    .cr3 = 0
};

#define TEST_DISABLED

#define BT_CONTROL_PREFIX   "[BT control]: "

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
    float           rates[3];
} pid_rates_t;

typedef struct {
    /* [ rp[P, I, D], y[P, I, D] ] */
    pid_rates_t     PIDRates[2];
    uint16_t        plot_period_ms;

} connect_response_t;

typedef struct {
    bool            succeed;
} response_ack_t;

#define DATA_FLOAT_MULTIPLIER   100

typedef struct {
    /* Service fields */
    uint32_t        packId;

    /* Data fields */
    int16_t         roll;
    int16_t         pitch;
    int16_t         yaw;

    int16_t         ref_roll;
    int16_t         ref_pitch;
    int16_t         ref_yaw;
} data_package_t;

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

void stopDataTransfer ( void );


bool            is_data_transfering     = false;
virtual_timer_t watchdog_vt;
uint32_t        package_id              = 0;

void watchdogTimerHandler ( void *arg )
{
    arg = arg;

    dprintf_mod( BT_CONTROL_PREFIX, "Got timeout\n" );

    is_data_transfering = false;
}

void resetDataTransfer ( void )
{
    /* Reset timer timeout */
    chVTSet( &watchdog_vt, S2ST( 10 ), watchdogTimerHandler, NULL );
}

void startDataTransfer ( void )
{
    is_data_transfering = true;
    package_id          = 0;

    resetDataTransfer();
}

time_measurement_t      send_tm;

void stopDataTransfer ( void )
{
    is_data_transfering = false;

    chVTReset( &watchdog_vt );
#if 0
    dprintf( "Best: %d, Worst: %d\n", 
                RTC2US(SYSTEM_FREQUENCY, send_tm.best), 
                RTC2US(SYSTEM_FREQUENCY, send_tm.worst) );
#endif
}


uint32_t    send_period_ms  = CONTROL_SYSTEM_PERIOD_MS;

BSEMAPHORE_DECL(sen_sem, false);

void responseConnectCommand ( frame_command_t cmd )
{
    PID_rates_t main_rates  = PID_controller_get_rates( PID_ROLL );
    PID_rates_t yaw_rates   = PID_controller_get_rates( PID_YAW );

    connect_response_t response = { .plot_period_ms = send_period_ms };

    response.PIDRates[0].rates[0] = main_rates.prop;
    response.PIDRates[0].rates[1] = main_rates.diff;
    response.PIDRates[0].rates[2] = main_rates.integr;

    response.PIDRates[1].rates[0] = yaw_rates.prop;
    response.PIDRates[1].rates[1] = yaw_rates.diff;
    response.PIDRates[1].rates[2] = yaw_rates.integr;

    uint8_t cksum = calcChksum( (void *)&response, sizeof(response) );

    chBSemWait( &sen_sem );

    sdPut( btDriver, FRAME_START );
    sdPut( btDriver, cmd );
    sdWrite( btDriver, (uint8_t *)&response, sizeof(response) );
    sdPut( btDriver, cksum );

    chBSemSignal( &sen_sem );
}


void responseAck ( frame_command_t cmd, bool set )
{
    response_ack_t response = { .succeed = set };

    chBSemWait( &sen_sem );

    sdPut( btDriver, FRAME_START );
    sdPut( btDriver, cmd );
    sdWrite( btDriver, (uint8_t *)&response, sizeof(response) );

    chBSemSignal( &sen_sem );
}

void sendDataPackage ( data_package_t *data )
{
    /* First set final data, then calc cksum */
    data->packId = package_id++;

    uint8_t cksum = calcChksum( (void *)data, sizeof(*data) );
    
    chBSemWait( &sen_sem );

    sdPut( btDriver, FRAME_START );
    sdPut( btDriver, COMMAND_DATA_PACK );
    sdWrite( btDriver, (uint8_t *)data, sizeof(*data) );
    sdPut( btDriver, cksum );

    chBSemSignal( &sen_sem );
}

static THD_WORKING_AREA(waBluetoothSerial, 1024);
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

        // dprintf( "Got: 0x%x / %c\n", received_byte, received_byte );
        // sdPut( uartDriver, received_byte );

        switch ( receive_mode )
        {
            case WAIT_4_START:

                if ( received_byte == FRAME_START )
                {
                    receive_mode = WAIT_4_COMMAND;
                    dprintf( "Wait 4 cmd\n" );
                }  

                break;

            case WAIT_4_COMMAND:

                received_cmd = received_byte;

                switch ( received_cmd )
                {
                    case COMMAND_CONNECT:
                        dprintf( "Got connect cmd\n" );
                        responseConnectCommand( COMMAND_CONNECT );
                        receive_mode = WAIT_4_START;
                        break;

                    case COMMAND_PING:
                        dprintf( "Got ping cmd\n" );
                        responseAck( COMMAND_PING, true );
                        receive_mode = WAIT_4_START;
                        
                        resetDataTransfer();
                        break;

                    case COMMAND_DATA_START:
                        dprintf( "Got data start cmd\n" );
                        responseAck( COMMAND_DATA_START, true );
                        receive_mode = WAIT_4_START;
                        
                        startDataTransfer();
                        break;


                    case COMMAND_DATA_STOP:
                        dprintf( "Got data stop cmd\n" );
                        responseAck( COMMAND_DATA_STOP, true );
                        receive_mode = WAIT_4_START;
                        
                        stopDataTransfer();
                        break;

                    case COMMAND_SET_MAIN_RATES:
                        dprintf( "Got set main rates cmd\n" );
                        receive_data_length = sizeof( pid_rates_t );
                        receive_mode = WAIT_4_DATA;
                        data_idx = 0;

                        break;

                    case COMMAND_SET_YAW_RATES:
                        dprintf( "Got set yaw rates cmd\n" );
                        receive_data_length = sizeof( pid_rates_t );
                        receive_mode = WAIT_4_DATA;
                        data_idx = 0;

                        break;

                    default:
                        dprintf( "Unknown cmd\n" );
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
                    dprintf( "Invalid cksum\n" );
                }

                receive_mode = WAIT_4_START;

                /* Resolving received data */
                pid_rates_t *rates = (pid_rates_t *)data_buffer;
                PID_rates_t res_rates;

                res_rates.prop     = rates->rates[0];
                res_rates.integr   = rates->rates[1];
                res_rates.diff     = rates->rates[2];

                if ( received_cmd == COMMAND_SET_MAIN_RATES )
                {
                    PID_controller_set_rates( &res_rates, PID_ROLL );
                    PID_controller_set_rates( &res_rates, PID_PITCH );
                }
                else if ( received_cmd == COMMAND_SET_YAW_RATES )
                {
                    PID_controller_set_rates( &res_rates, PID_YAW );
                }

#if 1
                if ( received_cmd == COMMAND_SET_MAIN_RATES ||
                     received_cmd == COMMAND_SET_YAW_RATES )
                {
                    PID_rates_t roll_rates = PID_controller_get_rates( PID_ROLL );
                    PID_rates_t pitch_rates = PID_controller_get_rates( PID_PITCH );
                    PID_rates_t yaw_rates = PID_controller_get_rates( PID_YAW );

                    dprintf( "PID: R: %d / %d / %d | P: %d / %d / %d | Y: %d / %d / %d\n",
                                (int)(roll_rates.prop       * INT_ACCURACY_MULTIPLIER),
                                (int)(roll_rates.integr     * INT_ACCURACY_MULTIPLIER),
                                (int)(roll_rates.diff       * INT_ACCURACY_MULTIPLIER),
                                (int)(pitch_rates.prop      * INT_ACCURACY_MULTIPLIER),
                                (int)(pitch_rates.integr    * INT_ACCURACY_MULTIPLIER),
                                (int)(pitch_rates.diff      * INT_ACCURACY_MULTIPLIER),
                                (int)(yaw_rates.prop        * INT_ACCURACY_MULTIPLIER),
                                (int)(yaw_rates.integr      * INT_ACCURACY_MULTIPLIER),
                                (int)(yaw_rates.diff        * INT_ACCURACY_MULTIPLIER) );
                }
#endif

                break;

            default:
                ;
        }

        
        // chThdSleepMilliseconds( 300 );
    }
}

static THD_WORKING_AREA(waSender, 128);
static THD_FUNCTION(Sender, arg) 
{
    arg = arg;

    chTMObjectInit( &send_tm );

    systime_t time = chVTGetSystemTimeX();

    // static float value = 0.0; 

    while ( 1 )
    {
        time += MS2ST( send_period_ms );
        
        
        if ( is_data_transfering )
        {
            data_package_t data;

            // value += 0.02;

            // if ( value > 7 )
            //     value = 0.0;

            data.roll   = euler_angles.roll   * DATA_FLOAT_MULTIPLIER;
            data.pitch  = euler_angles.pitch  * DATA_FLOAT_MULTIPLIER;
            data.yaw    = euler_angles.yaw    * DATA_FLOAT_MULTIPLIER;

            // dprintf( "Data send: %d / %d / %d\n", data.roll, data.pitch, data.yaw );

            // chTMStartMeasurementX( &send_tm );

            /* 95 - 111 us */
            sendDataPackage( &data );

            // chTMStopMeasurementX( &send_tm );
        }

        chThdSleepUntil( time );
    }
}

#ifndef TEST_DISABLED

static THD_WORKING_AREA(waUARTSerial, 16);
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

#endif

int bt_control_init( tprio_t prio )
{
    /* Setup Bluetooth serial */
    sdStart( btDriver, &btCfg );
    palSetPadMode( GPIOC, 10, PAL_MODE_ALTERNATE(7) );    // TX
    palSetPadMode( GPIOC, 11, PAL_MODE_ALTERNATE(7) );    // RX`

    chThdCreateStatic(waBluetoothSerial, sizeof(waBluetoothSerial), prio, BluetoothSerial, NULL);
    chThdCreateStatic(waSender, sizeof(waSender), prio , Sender, NULL);

#ifndef TEST_DISABLED
    chThdCreateStatic(waUARTSerial, sizeof(waUARTSerial), prio, UARTSerial, NULL);
#endif

    dprintf_mod( BT_CONTROL_PREFIX, "Initialized\n" );

    return EOK;
}
