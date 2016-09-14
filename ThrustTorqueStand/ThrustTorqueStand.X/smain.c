#include "core.h"
#include "motor_control.h"
#include "ad7705.h"
#include <serial_protocol.h>

#include "pragmas.h"

/*
 * Connection:
 *      <AD7705>        <Direct connect>    <PIC24>
 *      SCLK        -   SCK             -   RG6
 *      MCLK IN     -   QUARTZ
 *      MCLK OUT    -   QUARTZ
 *      CS          -   GND(Always chosen)
 *      nRESET      -   PTP17(17)       -   RE4
 *      AIN1(+)     -   YELLOW_TENZO
 *      AIN1(-)     -   ORANGE_TENZO
 *      GND         -   GND
 *      VDD         -   VDD_IN(3.3V)
 *      DIN         -   SDO             -   RG8
 *      DOUT        -   SDI             -   RG7
 *      nDRDY       -   PTP12(18)       -   RD6
 *      REF IN(-)   -   GND
 *      REF IN(+)   -   POT_OUT
 * 
 *      <Potentiometer> <Connect>
 *      POT_SUP(+)  -   VDD(3.3V)
 *      POT_SUP(-)  -   GND
 * 
 *      <Tenzo>         <Connect>
 *      RED_TENZO   -   USB(+)(red)(+5V)
 *      BROWN_TENZO -   GND
 * 
 *      <USB>           <Connect>           <PIC24>
 *      Green       -   PTP15(15)       -   RC14
 *      White       -   PTP5(5)         -   RB14
 *      Black       -   GND
 */

int current_sensor_init ( uint8_t channel )
{
    if ( channel > 31 )
        return( -1 );
    
    AD1CON1bits.ADON = 0;
    if ( channel > 15 )
        AD1PCFGH &= ~(1 << (channel-16)); // Analog mode pin setup
    else
        AD1PCFGL &= ~(1 << channel); // Analog mode pin setup
    
    AD1CON1bits.ASAM = 1;        // Auto sample
    AD1CON1bits.SSRC = 0b111;    // Auto convertion
    AD1CON1bits.AD12B = 0;       // 0 = 10 bit ADC; 1 = 12 bit ADC
    
    AD1CON2bits.CHPS = 0x10;	
    AD1CON3bits.SAMC = 0b11111;	     // Sample time 
    AD1CON3bits.ADCS = 0b11111111;      // Conversion clock select
	AD1CHS0bits.CH0SA = channel;
    AD1CHS0bits.CH0NA = 0;
    AD1CON1bits.ADON = 1;
    return( 0 );
}

int16_t ADC_res = 0;

int16_t current_sensor_read( void )
{	
    if ( AD1CON1bits.DONE ) {
        AD1CON1bits.DONE = 0;            // reset DONE bit
        ADC_res = ADC1BUF0;
    }
	return( ADC_res );       			// read ADC1 data      
}


void init_control_system_interrupt ( void );

struct stand_data_pack_ {
    uint16_t    thrust;
    uint16_t    torque;
    uint16_t    voltage;
    uint16_t    current;
    uint16_t    speed;
} stand_data;

int16_t tenzo_data = 0,
        current_data = 0;

bool        stop_motors = false;
bool        start_motors = false;
bool        dataSend = false;
uint32_t    motorPower = 0;
uint16_t    timeMoments = 0;

int main ( void ) {
    OFF_ALL_ANALOG_INPUTS;
    
    current_sensor_init( 5 );

    UART_init( UARTm1, UART_460800, INT_PRIO_MID ); // Debug
    UART_init( UARTm2, UART_460800, INT_PRIO_HIGHEST );     // Interface
    UART_write_string( UARTm1, "UART initialized\n" );
//    motors_init();
//    tacho_init();
//    init_control_system_interrupt();
    
#ifdef AD7705
    spi_init();
    int res = 0;
    if ( ( res = ad7705_init() ) < 0 )
    {
        UART_write_string( "AD7705 initialization failed, %d\n", res );
        while ( 1 );
    }
    spi_set_speed( SPI_PRIM_1, SPI_SEC_2 );
    UART_write_string( "AD7705 initialized and calibrate\n" );
#endif
//    if ( init_sin_table( 1000, 1000, 1000 ) != 0 )
//    if ( init_square( 0, 8000, 2000 ) != 0 )
//    {
//        UART_write_string( "Init signal table failed\n" );
//        while(1);
//    }
            
    while ( 1 )
    {
        delay_ms( 100 );

//        current_data = ADC_read();
//        process_UART_input_command2( UART_get_last_received_command() );
        
        UART_write_string( UARTm1, "Current: %d\n", current_sensor_read() );
        
#ifdef AD7705
        if ( ad7705_is_data_ready() )
        {
            uint16_t data_res = ad7705_read_data();
            UART_write_string( "Data: %d\n", data_res );
        }
#endif
    }
    
    return( 0 );
}

#define INTERRUPT_FREQ 200L

void init_control_system_interrupt( void )
{
    T4CONbits.TON = 0;
    T4CONbits.TCKPS = TIMER_DIV_1;
    PR4 = (FCY/INTERRUPT_FREQ) & 0xffff;
    _T4IP = INT_PRIO_MID_HIGH;
    _T4IE = 1;
    _T4IF = 0;
}

void start_control_system_interrupt ( void )
{
    TMR4 = 0;
    T4CONbits.TON = 1;
}

void stop_control_system_interrupt ( void )
{
    T4CONbits.TON = 0;
}

uint16_t    throttle_power = 0;
int         input_signal_flag = 0;

void start_system ( void )
{
    start_control_system_interrupt();
    tacho_start_cmd();
    set_motor_started();
}

void stop_system ( void )
{
    set_motor_stopped();
    stop_control_system_interrupt();
    input_signal_flag = 0;
    tacho_stop_cmd();
}

#define ROTOR_DATA_COUNT (sizeof(send_rotor_array)/sizeof(send_rotor_array[0]))

void send_UART_motor_data ( uint16_t speed, int16_t thrust, uint16_t inputSignal, uint16_t current )
{
    uint16_t send_rotor_array[] = { speed, thrust < 0 ? 0 : thrust, inputSignal/*, current */};
    UART_write_words( UARTm1, send_rotor_array, ROTOR_DATA_COUNT );
}

void __attribute__( (__interrupt__, auto_psv) ) _T4Interrupt()
{
    uint16_t tmp_round_speed = tacho_get_round_speed();
    
    send_UART_motor_data( tmp_round_speed, tenzo_data, throttle_power, current_data );
    
//    if ( input_signal_flag )
//        throttle_power = get_next_signal_value();
//    else
//        throttle_power = get_signal_zero_lvl();
    
    set_motor_power( throttle_power );
    
    _T4IF = 0;
}

void process_UART_frame( void )
{
    UART_frame_t *frame = cmdProcessor_rcvFrame();
    switch ( frame->command )
    {
        case NO_COMMAND:
        case UNKNOWN_COMMAND:
            break;
        case CONNECT:
            cmdProcessor_write_cmd( UARTm2, RESPONSE_PREFIX, RESP_NOERROR );
            dataSend = false;
            stop_motors = true;
            UART_write_string( UARTm1, "Connect\n" );
            break;
        case DISCONNECT:
            dataSend = false;
            stop_motors = true;
            UART_write_string( UARTm1, "Disconnect\n" );
            break;
        case DATA_START:
            timeMoments = 0;
            dataSend = true;
            UART_write_string( UARTm1, "DStart\n" );
            break;
        case DATA_STOP:
            dataSend = false;
            UART_write_string( UARTm1, "DStop\n" );
            break;
        case MOTOR_START:
            start_motors = true;
            UART_write_string( UARTm1, "MStart\n" );
            break;
        case MOTOR_STOP:
            stop_motors = true;
            UART_write_string( UARTm1, "MStop\n" );
            break;
        case MOTOR_SET_POWER:
            motorPower = frame->motorPower * INPUT_POWER_MAX / 100L;
            UART_write_string( UARTm1, "MSetPower\n" );
            break;
    }
}
