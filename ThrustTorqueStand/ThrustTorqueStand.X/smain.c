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
 *      nRESET      -   PTP17(17)       -   AN18(RC3)
 *      AIN1(+)     -   YELLOW_TENZO
 *      AIN1(-)     -   ORANGE_TENZO
 *      GND         -   GND
 *      VDD         -   VDD_IN(3.3V)
 *      DIN         -   SDO             -   RG8
 *      DOUT        -   SDI             -   RG7
 *      nDRDY       -   PTP12(18)       -   AN19(RC4)
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

// Thrust coefficient = 10000 -> 13700 from AD7705 = 1.37 kg
// Current coefficient = 66 mV/A -> 2045 = 3.3 V
//                                  0    = 1.65 V
//                                  1636 = 1.65 + 1.32 V    == 20 A
//                                  81.8 ADC digits/A

int current_sensor_init ( uint8_t channel )
{
    if ( channel > 31 )
        return( -1 );
    
    AD1CON1bits.ADON = 0;
    if ( channel > 15 )
        AD1PCFGH &= ~(1 << (channel-16)); // Analog mode pin setup
    else
        AD1PCFGL &= ~(1 << channel); // Analog mode pin setup
    
    AD1CON1bits.ASAM    = 1;        // Auto sample
    AD1CON1bits.SSRC    = 0b111;    // Auto convertion
    AD1CON1bits.AD12B   = 1;       // 0 = 10 bit ADC; 1 = 12 bit ADC
    AD1CON1bits.FORM    = 0b01;     // Signed integer [-2046; 2045]
    
    AD1CON3bits.SAMC    = 0b11111;	     // Sample time 
    AD1CON3bits.ADCS    = 0b11111111;      // Conversion clock select
	AD1CHS0bits.CH0SA   = channel;
    AD1CHS0bits.CH0NA   = 0;
    
    AD1CON1bits.ADON    = 1;
    return( 0 );
}



int16_t current_sensor_read( void )
{	
    static int16_t current_adc_res = 0;
    
    if ( AD1CON1bits.DONE ) {
        AD1CON1bits.DONE = 0;            // reset DONE bit
        current_adc_res = ADC1BUF0;
    }
	return( current_adc_res );       			// read ADC1 data      
}

int voltage_sensor_init ( uint8_t channel )
{
    if ( channel > 31 )
        return( -1 );
    
    AD2CON1bits.ADON = 0;
    if ( channel > 15 )
        AD1PCFGH &= ~(1 << (channel-16)); // Analog mode pin setup
    else
        AD1PCFGL &= ~(1 << channel); // Analog mode pin setup
    
    AD2CON1bits.ASAM = 1;        // Auto sample
    AD2CON1bits.SSRC = 0b111;    // Auto convertion
    AD2CON1bits.AD12B = 1;       // 0 = 10 bit ADC; 1 = 12 bit ADC
    AD2CON1bits.FORM    = 0b00;     // Unsigned integer [0; 4095]
    
    AD2CON3bits.SAMC = 0b11111;	     // Sample time 
    AD2CON3bits.ADCS = 0b11111111;      // Conversion clock select
	AD2CHS0bits.CH0SA = channel;
    AD2CHS0bits.CH0NA = 0;
    
    AD2CON1bits.ADON = 1;
    return( 0 );
}

uint16_t voltage_sensor_read( void )
{	
    static uint16_t voltage_adc_res = 0;
    
    if ( AD2CON1bits.DONE ) {
        AD2CON1bits.DONE = 0;            // reset DONE bit
        voltage_adc_res = ADC2BUF0;
    }
	return( voltage_adc_res );       			// read ADC1 data      
}

/*** MAIN PROGRAMM ***/

void process_uninitialized_error_UART_frame();
void process_UART_frame( void );
void init_control_system_interrupt ( void );
void start_control_system_interrupt ( void );

volatile bool       stop_motors = false;
volatile bool       start_motors = false;

#define UART_DEBUG      UARTm1
#define UART_INTERFACE  UARTm2

int main ( void ) {
    OFF_ALL_ANALOG_INPUTS;
    
    current_sensor_init( 5 );
    
    UART_init( UART_DEBUG, UART_460800, INT_PRIO_MID ); // Debug
    UART_init( UART_INTERFACE, UART_460800, INT_PRIO_HIGHEST );     // Interface
    UART_write_string( UART_DEBUG, "UART initialized\n" );

    cmdProcessor_init( UART_INTERFACE );
    
    tacho_init();
    
    spi_init();
    int res = 0;
    if ( ( res = ad7705_init() ) < 0 ) {
        UART_write_string( UART_DEBUG, "AD7705 initialization failed, %d\n", res );
        while ( 1 )
            process_uninitialized_error_UART_frame();
    }
    spi_set_speed( SPI_PRIM_1, SPI_SEC_2 );
    UART_write_string( UART_DEBUG, "AD7705 initialized and calibrate\n" );
    
    init_control_system_interrupt();
    
    motor_init();
    
    while ( 1 )
    {
        process_UART_frame();
        Nop();
    }
    
    return( 0 );
}

#define INTERRUPT_FREQ_HZ 100L

void init_control_system_interrupt( void )
{
    T4CONbits.TON = 0;
    T4CONbits.TCKPS = TIMER_DIV_8;
    PR4 = FCY / 8 / INTERRUPT_FREQ_HZ;
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

uint16_t    thrust_data;
uint16_t    torque_data;
uint16_t    voltage_data;
uint16_t    current_data;
uint16_t    speed_data;

volatile uint16_t    motor_power_start      = 0;
volatile uint16_t    motor_power_end        = 0;

volatile uint16_t    time_measure_offset    = 0;
volatile uint16_t    time_measure           = 0;
volatile uint16_t    time_step_moment       = 0;

uint16_t    time_period_counter             = 0;
bool        send_data                       = false;
bool        power_changed                    = false;

void start_system ( void )
{
    if ( time_measure_offset == 0 ||
         time_measure        == 0 || 
         time_step_moment    == 0 ) {
         cmdProcessor_response( RESP_ENDDATA );
         return;
    }
    
    tacho_start_cmd();
    time_period_counter = 0;
    send_data           = false;
    power_changed        = false;
    set_motor_power( motor_power_start );
    start_control_system_interrupt();
}

void stop_system ( void )
{
    set_motor_stopped();
    stop_control_system_interrupt();
    tacho_stop_cmd();
    cmdProcessor_response( RESP_ENDDATA );
}

#define ROTOR_DATA_COUNT (sizeof(send_rotor_array)/sizeof(send_rotor_array[0]))

void send_UART_data ( void )
{
    if ( !send_data && time_period_counter >= time_measure_offset ) {
        send_data = true;
        time_period_counter = 0;
    }
    
    if ( send_data ) {
        uint16_t send_rotor_array[] = { thrust_data, torque_data, current_data, speed_data, time_period_counter };
        UART_write_byte( UART_INTERFACE, DATA_PREFIX );
        UART_write_words( UART_INTERFACE, send_rotor_array, ROTOR_DATA_COUNT );
    }
    
    if ( !power_changed && send_data && time_period_counter >= time_step_moment ) {
        power_changed = true;
        set_motor_power( motor_power_end );
        UART_write_string( UART_DEBUG, "Power changed\n" );
    }
    
    if ( send_data && time_period_counter >= time_measure ) {
        stop_system();
    }
}

/**
 * Main Interrupt processing
 */

void __attribute__( (__interrupt__, auto_psv) ) _T4Interrupt()
{
    speed_data = tacho_get_round_speed();   // In rpm
    current_data = current_sensor_read();
    
    if ( ad7705_is_data_ready() )
    {
        thrust_data = ad7705_read_data();
//        UART_write_string( UART_DEBUG, "%06d, %06d, %06d\n", thrust_data, current_data, speed_data );
    } else {
        UART_write_string( UART_DEBUG, "No data\n" );       // Not happened
    }
    
    send_UART_data();
    time_period_counter++;
    
    _T4IF = 0;
}

void process_uninitialized_error_UART_frame()
{
    UART_frame_t *frame = cmdProcessor_rcvFrame();
    if ( frame->command != NO_COMMAND && frame->command != UNKNOWN_COMMAND )
        cmdProcessor_response( RESP_UNINITIALIZED );
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
            cmdProcessor_response( RESP_NOERROR );
            stop_system();
            UART_write_string( UART_DEBUG, "Connect\n" );
            break;
        case DISCONNECT:
            stop_system();
            UART_write_string( UART_DEBUG, "Disconnect\n" );
            break;
        case MEASURE_START:
            start_system();
            UART_write_string( UART_DEBUG, "MStart\n" );
            break;
        case MEASURE_STOP:
            stop_system();
            UART_write_string( UART_DEBUG, "MStop\n" );
            break;
        case MEASURE_SET_PARAMS:

            motor_power_start   = PERCENTS_2_INPUT( frame->motorPowerStart );
            motor_power_end     = PERCENTS_2_INPUT( frame->motorPowerEnd );
            time_measure_offset = frame->timeMeasureOffsetMs;
            time_measure        = frame->timeMeasureTimeMs;
            time_step_moment    = frame->timeStepMomentMs;
            
            UART_write_string( UART_DEBUG, "Params: %d, %d, %d, %d, %d\n", 
                                            motor_power_start,
                                            motor_power_end,
                                            time_measure_offset,
                                            time_step_moment,                        
                                            time_measure );
            
            UART_write_string( UART_DEBUG, "MSetPower\n" );
            cmdProcessor_response( RESP_NOERROR );
            break;
    }
}