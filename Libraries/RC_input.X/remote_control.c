#include "remote_control.h"

/* Input capture defines */

#define IC_TIMER_2                  1
#define IC_TIMER_3                  0

#define IC_CE_MODE_DISABLED         0b000
#define IC_CE_MODE_EDGE             0b001
#define IC_CE_MODE_FALLING_EDGE     0b010
#define IC_CE_MODE_RISING_EDGE      0b011
#define IC_CE_MODE_4TH_RISE_EDGE    0b100
#define IC_CE_MODE_16TH_RISE_EDGE   0b101

#define IC_INT_MODE_1ST_CE          0b00
#define IC_INT_MODE_2ND_CE          0b01
#define IC_INT_MODE_3RD_CE          0b10
#define IC_INT_MODE_4TH_CE          0b11

typedef struct
{
    int32_t    min, 
               max, 
               mid;
}Channel_t;

typedef struct
{
    Channel_t   channel_1,
                channel_2,
                channel_3,
                channel_4,
                channel_5;
}Calibrated_control_t;

/********** RC FUNCTIONS **********/

static RC_input_raw_t            control_raw;
static Calibrated_control_t clbr_control_raw = { { 16754, 29989, 23371 },   //Roll
                                                 { 16832, 30036, 23434 },   //Pitch
                                                 { 16832, 30068, 23450 },   //Throttle
                                                 { 16752, 29991, 23371 },   //Yaw
                                                 { 16736, 30020, 23786 }
                                                };
static RC_input_values_t     dir_values;
static bool                 remote_control_online   = false,
                            intialized              = false;

/* Prototypes */
static void init_channel_1();
static void init_channel_2();
static void init_channel_3();
static void init_channel_4();
static void init_channel_5();

/********************************/
/*      MAIN INITIALIZATION     */
/********************************/
#define LOSS_LIGHT      _LATA3
#define TRIS_LOSS_L     _TRISA3
#define LOSS_LIGHT_LOST     0
#define LOSS_LIGHT_FOUND    1
#define WD_TIMER_RESET {TMR3 = 0; remote_control_online = true; LOSS_LIGHT = LOSS_LIGHT_FOUND;}

bool remote_control_find_controller()
{
    LOSS_LIGHT = LOSS_LIGHT_FOUND;
    memset( &control_raw, 0, sizeof(control_raw) );
    
    return remote_control_online;
}

static void 
init_watch_dog_timer()
{
    T3CONbits.TON = 0;
    T3CONbits.TCKPS = TIMER_DIV_256;
    TMR3 = 0;
    _T3IF = 0;
    _T3IE = 1;
    PR3 = FCY/256/4;    // 32 MHz = 0.25 second
    T3CONbits.TON = 1;
}

/* Whatch dog timer overflow */
void __attribute__( (__interrupt__, no_auto_psv) ) _T3Interrupt()
{
    // Processing losing signal from transmitter
    LOSS_LIGHT = LOSS_LIGHT_LOST;
    remote_control_online = false;
    _T3IF = 0;
}

RC_input_values_t *remote_control_init( void )
{
    T2CONbits.TON = 0;
    T2CONbits.TCKPS = TIMER_DIV_1;
    TMR2 = 0;
    PR2 = UINT16_MAX;
    TRIS_LOSS_L = 0;
    init_channel_1();
    init_channel_2();
    init_channel_3();
    init_channel_4();
    init_channel_5();
    T2CONbits.TON = 1;
    init_watch_dog_timer();
    intialized = true;
    
    return( &dir_values );
}

RC_input_raw_t *remote_control_get_raw_prt ( void )
{
    return &control_raw;
}

/********************************/
/*        CALLIBRATION          */
/********************************/
#define CLBR_TIME   30
#define CLBR_LIGHT  _RA5
volatile static uint8_t  elapsed_seconds = 0;

static void 
turn_on_timer1_time_measurement()
{
    elapsed_seconds = 0;
    T1CONbits.TCKPS = TIMER_DIV_256;
    TMR1 = 0;
    _T1IP = 1;
    _T1IF = 0;
    _T1IE = 1;
    PR1 = FCY/256; //32 MHz = 62500 it is 1 second! Limit is 65535!
    T1CONbits.TON = 1;
}

static void 
turn_off_timer1_time_measurement()
{
    T1CONbits.TON = 0;
    _T1IE = 0;
    TMR1 = 0;
}

void remote_control_make_calibration( uart_module_t module )
{
    _TRISA5 = 0;
    _LATA5 = 0;
    CLBR_LIGHT = 1;
    /* Set initial values */
    memset( &clbr_control_raw, 0, sizeof(clbr_control_raw) );
    clbr_control_raw.channel_1.min = UINT16_MAX;
    clbr_control_raw.channel_2.min = UINT16_MAX;
    clbr_control_raw.channel_3.min = UINT16_MAX;
    clbr_control_raw.channel_4.min = UINT16_MAX;
    clbr_control_raw.channel_5.min = UINT16_MAX;
    turn_on_timer1_time_measurement();
    while( !remote_control_online );
    
    delay_ms( 500 );
    
    while( elapsed_seconds < CLBR_TIME )
    {
        CLBR_LIGHT = 0;
        clbr_control_raw.channel_1.max = max( clbr_control_raw.channel_1.max, control_raw.channel_1 );
        clbr_control_raw.channel_1.min = min( clbr_control_raw.channel_1.min, control_raw.channel_1 );
        clbr_control_raw.channel_2.max = max( clbr_control_raw.channel_2.max, control_raw.channel_2 );
        clbr_control_raw.channel_2.min = min( clbr_control_raw.channel_2.min, control_raw.channel_2 );
        clbr_control_raw.channel_3.max = max( clbr_control_raw.channel_3.max, control_raw.channel_3 );
        clbr_control_raw.channel_3.min = min( clbr_control_raw.channel_3.min, control_raw.channel_3 );
        clbr_control_raw.channel_4.max = max( clbr_control_raw.channel_4.max, control_raw.channel_4 );
        clbr_control_raw.channel_4.min = min( clbr_control_raw.channel_4.min, control_raw.channel_4 );
        clbr_control_raw.channel_5.max = max( clbr_control_raw.channel_5.max, control_raw.channel_5 );
        clbr_control_raw.channel_5.min = min( clbr_control_raw.channel_5.min, control_raw.channel_5 );
        delay_ms( 500 );
        remote_control_send_UART_control_raw_data( module );
//        UART_write_string( module, "Calibration\n" );
    }
    turn_off_timer1_time_measurement();
    clbr_control_raw.channel_1.mid = (clbr_control_raw.channel_1.max + clbr_control_raw.channel_1.min)/2;
    clbr_control_raw.channel_2.mid = (clbr_control_raw.channel_2.max + clbr_control_raw.channel_2.min)/2;
    clbr_control_raw.channel_3.mid = (clbr_control_raw.channel_3.max + clbr_control_raw.channel_3.min)/2;
    clbr_control_raw.channel_4.mid = (clbr_control_raw.channel_4.max + clbr_control_raw.channel_4.min)/2;
    clbr_control_raw.channel_5.mid = (clbr_control_raw.channel_5.max + clbr_control_raw.channel_5.min)/2;
    CLBR_LIGHT = 1;
    
    while ( 1 )
    {
        UART_write_string( module, "\nCh1: %05ld, %05ld, %05ld\n", clbr_control_raw.channel_1.min, clbr_control_raw.channel_1.max, clbr_control_raw.channel_1.mid );
        UART_write_string( module, "Ch2: %05ld, %05ld, %05ld\n", clbr_control_raw.channel_2.min, clbr_control_raw.channel_2.max, clbr_control_raw.channel_2.mid );
        UART_write_string( module, "Ch3: %05ld, %05ld, %05ld\n", clbr_control_raw.channel_3.min, clbr_control_raw.channel_3.max, clbr_control_raw.channel_3.mid );
        UART_write_string( module, "Ch4: %05ld, %05ld, %05ld\n", clbr_control_raw.channel_4.min, clbr_control_raw.channel_4.max, clbr_control_raw.channel_4.mid );
        UART_write_string( module, "Ch5: %05ld, %05ld, %05ld\n", clbr_control_raw.channel_5.min, clbr_control_raw.channel_5.max, clbr_control_raw.channel_5.mid );

        delay_ms( 3000 );
    }
//    calibration_flag = 1;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _T1Interrupt()
{
    elapsed_seconds++;
    _T1IF = 0;
}

/********************************/
/*  CHANNELS PARAMETERS OUTPUT  */
/********************************/
int remote_control_update_control_values( void )
{
    int16_t res = 0;
    RC_input_raw_t tmp_count_cntrl_raw;
    
    memcpy( &tmp_count_cntrl_raw, &control_raw, sizeof( control_raw ) );
    if ( !remote_control_online || !intialized )
    {
        tmp_count_cntrl_raw.channel_1 = clbr_control_raw.channel_1.mid;
        tmp_count_cntrl_raw.channel_2 = clbr_control_raw.channel_2.mid;
        tmp_count_cntrl_raw.channel_4 = clbr_control_raw.channel_4.mid;
        tmp_count_cntrl_raw.channel_3 = clbr_control_raw.channel_3.min;
        tmp_count_cntrl_raw.channel_5 = clbr_control_raw.channel_5.min;
    }
    
    int32_t throttle_delta = tmp_count_cntrl_raw.channel_3 - clbr_control_raw.channel_3.min;
    throttle_delta = throttle_delta <= 0 ? 1 : throttle_delta;
    res = throttle_delta*THROTTLE_MAX/(clbr_control_raw.channel_3.max - clbr_control_raw.channel_3.min);
    dir_values.throttle = res < THROTTLE_MIN ? THROTTLE_MIN : THROTTLE_MAX < res ? THROTTLE_MAX : res;

    int32_t rudder_delta = tmp_count_cntrl_raw.channel_4 - clbr_control_raw.channel_4.mid;
    res = rudder_delta*ANGLES_MAX/(clbr_control_raw.channel_4.mid - clbr_control_raw.channel_4.min);
    dir_values.rudder = res < ANGLES_MIN ? ANGLES_MIN : ANGLES_MAX < res ? ANGLES_MAX : res;
    
    int32_t roll_delta = tmp_count_cntrl_raw.channel_1 - clbr_control_raw.channel_1.mid;
    res = roll_delta*ANGLES_MAX/(clbr_control_raw.channel_1.mid - clbr_control_raw.channel_1.min);
    dir_values.roll =  res < ANGLES_MIN ? ANGLES_MIN : ANGLES_MAX < res ? ANGLES_MAX : res;

    int32_t pitch_delta = tmp_count_cntrl_raw.channel_2 - clbr_control_raw.channel_2.mid;
    res = pitch_delta*ANGLES_MAX/(clbr_control_raw.channel_2.mid - clbr_control_raw.channel_2.min);
    dir_values.pitch = res < ANGLES_MIN ? ANGLES_MIN : ANGLES_MAX < res ? ANGLES_MAX : res;
    
    dir_values.two_pos_switch = tmp_count_cntrl_raw.channel_5 > clbr_control_raw.channel_5.mid ? TWO_POS_SWITCH_ON : TWO_POS_SWITCH_OFF;
    
    return( 0 );
}

void remote_control_send_UART_control_values( uart_module_t module )
{
    UART_write_string( module, "In: %04d, %04d, %04d, %04d, %01d\n", 
            dir_values.throttle, dir_values.rudder, dir_values.roll, dir_values.pitch, dir_values.two_pos_switch );
}

void remote_control_send_UART_control_raw_data( uart_module_t module )
{
    UART_write_string( module, "In: %04ld, %04ld, %04ld, %04ld, %04ld\n", 
            control_raw.channel_3, control_raw.channel_4, control_raw.channel_1, control_raw.channel_2, control_raw.channel_5 );
}
/********************************/
/*      CHANNEL 1 - RD8         */
/********************************/

static void 
init_channel_1()
{
    _TRISD8 = 1;
    IC1CONbits.ICM = IC_CE_MODE_DISABLED;
    IC1CONbits.ICTMR = IC_TIMER_2;
    IC1CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC1CONbits.ICM = IC_CE_MODE_EDGE;
    _IC1IP = 7;     //Priority 7
    _IC1IF = 0;     // Zero interrupt flag
    _IC1IE = 1;     // Enable interrupt
}

static void 
clean_IC1_buffer()
{
    uint16_t trash = 0;
    while( IC1CONbits.ICBNE )
        trash = IC1BUF;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _IC1Interrupt()
{
    uint16_t    pulse_fall = 0,
                pulse_rise = 0;

    if ( !_RD8 ) //Interrupt after falling edge
    {
        pulse_rise = IC1BUF;
        pulse_fall = IC1BUF;
        clean_IC1_buffer();
        if ( pulse_fall > pulse_rise )
            control_raw.channel_1 = (pulse_fall - pulse_rise);
        else
            control_raw.channel_1 = (pulse_fall + (PR2 - pulse_rise));
    }
    WD_TIMER_RESET;
    _IC1IF = 0;
}


/********************************/
/*      CHANNEL 2 - RD9         */
/********************************/

static void 
init_channel_2()
{
    _TRISD9 = 1;
    IC2CONbits.ICM = IC_CE_MODE_DISABLED;
    IC2CONbits.ICTMR = IC_TIMER_2;
    IC2CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC2CONbits.ICM = IC_CE_MODE_EDGE;
    _IC2IP = 7;     //Priority 7
    _IC2IF = 0;     // Zero interrupt flag
    _IC2IE = 1;     // Enable interrupt
}

static void 
clean_IC2_buffer()
{
    uint16_t trash = 0;
    while( IC2CONbits.ICBNE )
        trash = IC2BUF;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _IC2Interrupt()
{
    uint16_t    pulse_fall = 0,
                pulse_rise = 0;

    if ( !_RD9 ) //Interrupt after falling edge
    {
        pulse_rise = IC2BUF;
        pulse_fall = IC2BUF;
        clean_IC2_buffer();
        if ( pulse_fall > pulse_rise )
            control_raw.channel_2 = (pulse_fall - pulse_rise);
        else
            control_raw.channel_2 = (pulse_fall + (PR2 - pulse_rise));
    }
    WD_TIMER_RESET;
    _IC2IF = 0;
}


/********************************/
/*      CHANNEL 3 - RD10        */
/********************************/

static void 
init_channel_3()
{
    _TRISD10 = 1;
    IC3CONbits.ICM = IC_CE_MODE_DISABLED;
    IC3CONbits.ICTMR = IC_TIMER_2;
    IC3CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC3CONbits.ICM = IC_CE_MODE_EDGE;
    _IC3IP = 7;     //Priority 7
    _IC3IF = 0;     // Zero interrupt flag
    _IC3IE = 1;     // Enable interrupt
}

static void 
clean_IC3_buffer()
{
    uint16_t trash = 0;
    while( IC3CONbits.ICBNE )
        trash = IC3BUF;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _IC3Interrupt()
{
    uint16_t    pulse_fall = 0,
                pulse_rise = 0;

    if ( !_RD10 ) //Interrupt after falling edge
    {
        pulse_rise = IC3BUF;
        pulse_fall = IC3BUF;
        clean_IC3_buffer();
        if ( pulse_fall > pulse_rise )
            control_raw.channel_3 = (pulse_fall - pulse_rise);
        else
            control_raw.channel_3 = (pulse_fall + (PR2 - pulse_rise));
    }
    WD_TIMER_RESET;
    _IC3IF = 0;
}


/********************************/
/*      CHANNEL 4 - RD11        */
/********************************/

static void 
init_channel_4()
{
    _TRISD11 = 1;
    IC4CONbits.ICM = IC_CE_MODE_DISABLED;
    IC4CONbits.ICTMR = IC_TIMER_2;
    IC4CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC4CONbits.ICM = IC_CE_MODE_EDGE;
    _IC4IP = 7;     //Priority 7
    _IC4IF = 0;     // Zero interrupt flag
    _IC4IE = 1;     // Enable interrupt
}

static void 
clean_IC4_buffer()
{
    uint16_t trash = 0;
    while( IC4CONbits.ICBNE )
        trash = IC4BUF;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _IC4Interrupt()
{
    uint16_t    pulse_fall = 0,
                pulse_rise = 0;

    if ( !_RD11 ) //Interrupt after falling edge
    {
        pulse_rise = IC4BUF;
        pulse_fall = IC4BUF;
        clean_IC4_buffer();
        if ( pulse_fall > pulse_rise )
            control_raw.channel_4 = (pulse_fall - pulse_rise);
        else
            control_raw.channel_4 = (pulse_fall + (PR2 - pulse_rise));
    }
//    WD_TIMER_RESET;
    _IC4IF = 0;
}

/********************************/
/*      CHANNEL 5 - RD12        */
/********************************/

static void 
init_channel_5()
{
    _TRISD12 = 1;
    IC5CONbits.ICM = IC_CE_MODE_DISABLED;
    IC5CONbits.ICTMR = IC_TIMER_2;
    IC5CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC5CONbits.ICM = IC_CE_MODE_EDGE;
    _IC5IP = 7;     // Priority 7
    _IC5IF = 0;     // Zero interrupt flag
    _IC5IE = 1;     // Enable interrupt
}

static void 
clean_IC5_buffer()
{
    uint16_t trash = 0;
    while( IC5CONbits.ICBNE )
        trash = IC5BUF;
}

void __attribute__( (__interrupt__, no_auto_psv) ) _IC5Interrupt()
{
    uint16_t    pulse_fall = 0,
                pulse_rise = 0;

    if ( !_RD12 ) //Interrupt after falling edge
    {
        pulse_rise = IC5BUF;
        pulse_fall = IC5BUF;
        clean_IC5_buffer();
        if ( pulse_fall > pulse_rise )
            control_raw.channel_5 = (pulse_fall - pulse_rise);
        else
            control_raw.channel_5 = (pulse_fall + (PR2 - pulse_rise));
    }
//    WD_TIMER_RESET;
    _IC5IF = 0;
}
