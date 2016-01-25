#include "input_control.h"
#include "per_proto.h"

static Control_t            control_raw;
static Calibrated_control_t clbr_control_raw = { { 16839, 30125, 23482 },   //Roll
                                                 { 16756, 30104, 23430 },   //Pitch
                                                 { 16832, 30149, 23490 },   //Throttle
                                                 { 16740, 30073, 23406 },   //Yaw
                                                 { 16787, 30145, 23466 }
                                                };
static Control_values_t     dir_values;
static uint8_t              // calibration_flag = 0,
                            input_control_online_flag = 0,
                            init_flag = 0;

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
#define WD_TIMER_RESET {TMR3 = 0; input_control_online_flag = 1; LOSS_LIGHT = 1;}

void ic_find_control()
{
    LOSS_LIGHT = 1;
    memset( &control_raw, 0, sizeof(control_raw) );
    while( !input_control_online_flag )
    {
        LOSS_LIGHT = 0;
    }
    LOSS_LIGHT = 1;
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
void __attribute__( (__interrupt__, auto_psv) ) _T3Interrupt()
{
    // Processing losing signal from transmitter
    LOSS_LIGHT = 0;
    input_control_online_flag = 0;
    _T3IF = 0;
}

void init_input_control()
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
    init_flag = 1;
}

/********************************/
/*        CALLIBRATION          */
/********************************/
#define CLBR_TIME   10
#define CLBR_LIGHT  _RA5
static uint8_t  elapsed_seconds = 0;

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

// Change this after going to 80 MHz!!!
void ic_make_calibration()
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
    while( input_control_online_flag != 1 );
    delay_ms(500);
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
    }
    turn_off_timer1_time_measurement();
    clbr_control_raw.channel_1.mid = (clbr_control_raw.channel_1.max + clbr_control_raw.channel_1.min)/2;
    clbr_control_raw.channel_2.mid = (clbr_control_raw.channel_2.max + clbr_control_raw.channel_2.min)/2;
    clbr_control_raw.channel_3.mid = (clbr_control_raw.channel_3.max + clbr_control_raw.channel_3.min)/2;
    clbr_control_raw.channel_4.mid = (clbr_control_raw.channel_4.max + clbr_control_raw.channel_4.min)/2;
    clbr_control_raw.channel_5.mid = (clbr_control_raw.channel_5.max + clbr_control_raw.channel_5.min)/2;
    CLBR_LIGHT = 1;
//    calibration_flag = 1;
}

void send_UART_calibration_data( void )
{
    UART_write_string( "\n\rMin:%05ld, %05ld, %05ld, %05ld, %05ld\n\rMax:%05ld, %05ld, %05ld, %05ld, %05ld\n\rMid:%05ld, %05ld, %05ld, %05ld, %05ld\n", 
                        clbr_control_raw.channel_1.min, clbr_control_raw.channel_2.min, clbr_control_raw.channel_3.min, clbr_control_raw.channel_4.min, clbr_control_raw.channel_5.min,
                        clbr_control_raw.channel_1.max, clbr_control_raw.channel_2.max, clbr_control_raw.channel_3.max, clbr_control_raw.channel_4.max, clbr_control_raw.channel_5.max,
                        clbr_control_raw.channel_1.mid, clbr_control_raw.channel_2.mid, clbr_control_raw.channel_3.mid, clbr_control_raw.channel_4.mid, clbr_control_raw.channel_5.mid
                     );
}

void __attribute__( (__interrupt__, auto_psv) ) _T1Interrupt()
{
    elapsed_seconds++;
    _T1IF = 0;
}

/********************************/
/*  CHANNELS PARAMETERS OUTPUT  */
/********************************/
int8_t get_direction_values( Control_values_t *out_dir_vals )
{
    int16_t res = 0;
    Control_t tmp_count_cntrl_raw;
    
    memcpy( &tmp_count_cntrl_raw, &control_raw, sizeof( control_raw ) );
    if ( !input_control_online_flag || !init_flag )
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
    
    memcpy( out_dir_vals, &dir_values, sizeof( dir_values ) );
    
    return( 0 );
}

void send_UART_control_values( void )
{
    UART_write_string( "In: %04d, %04d, %04d, %04d, %01d\n", 
            dir_values.throttle, dir_values.rudder, dir_values.roll, dir_values.pitch, dir_values.two_pos_switch );
}

void send_UART_control_raw_data( void )
{
    UART_write_string( "In: %04ld, %04ld, %04ld, %04ld, %04ld\n", 
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

void __attribute__( (__interrupt__, auto_psv) ) _IC1Interrupt()
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

void __attribute__( (__interrupt__, auto_psv) ) _IC2Interrupt()
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

void __attribute__( (__interrupt__, auto_psv) ) _IC3Interrupt()
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

void __attribute__( (__interrupt__, auto_psv) ) _IC4Interrupt()
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

void __attribute__( (__interrupt__, auto_psv) ) _IC5Interrupt()
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
