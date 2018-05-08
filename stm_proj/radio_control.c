#include <core.h>

#define RADIO_MODULE_PREFIX     "[Radio] "

static GPTDriver *measure_tmr = &GPTD7;
static const GPTConfig tmr_cfg = {
    /* 65.5 ms */
    /* Measured pulse is in range [1; 1.8]ms */
    .frequency  = 1000000,
    .callback   = NULL,
    .cr2        = 0,
    .dier       = 0
};


/* UINT16_MAX - 16bit timer */
#define MEASURE_TIMER_MAX_COUNTER   UINT16_MAX

typedef struct {
    int32_t     raw_width;
    int32_t     front_cntr_value;
    ioline_t    line;

    int32_t     max_value;
    int32_t     min_value;
    int32_t     value_range;

    float       raw_value_2_control_rate;

} lld_rc_channel_t;

/* Extern structure */
rc_control_values_t     control_input;

#define MAX_USED_CH_PIN_IDX     15
#define CHANNEL_NOT_FILLED      -1  // or 0xFF unsigned

static uint8_t  pin2channel[MAX_USED_CH_PIN_IDX+1];

static lld_rc_channel_t rc_channels[MAX_CHANNELS_USED] = { 

        { .line = PAL_LINE(GPIOB, 2),  .min_value = 1043, .max_value = 1874 },
        { .line = PAL_LINE(GPIOB, 1),  .min_value = 1046, .max_value = 1873 },
        { .line = PAL_LINE(GPIOB, 15), .min_value = 1043, .max_value = 1876 },
        { .line = PAL_LINE(GPIOB, 14), .min_value = 1040, .max_value = 1871 },
        { .line = PAL_LINE(GPIOB, 13), .min_value = 1044, .max_value = 1876 }
    };


static void rc_pulse_cb( EXTDriver *extp, expchannel_t channel );

static bool             rc_connected = false;
static virtual_timer_t  connect_vt;

static void rc_connect_timeout ( void *p ) {
    rc_connected = false;

    memset( &control_input, 0, sizeof( control_input ) );
}

#ifdef RC_TIME_MEASUREMENT_DEBUG
time_measurement_t      rc_tm;
#endif

void radio_control_init ( void )
{
#ifdef RC_TIME_MEASUREMENT_DEBUG
    chTMObjectInit( &rc_tm );
    dprintf_mod( RADIO_MODULE_PREFIX, "Time measure init: %d / %d\n", rc_tm.worst, rc_tm.best );
#endif

    gptStart( measure_tmr, &tmr_cfg );
    gptStartContinuous( measure_tmr, MEASURE_TIMER_MAX_COUNTER );

    /* Init pin to channel conversion table */
    memset( pin2channel, CHANNEL_NOT_FILLED, sizeof( pin2channel ) );
    
    EXTChannelConfig ch_cfg;
    ch_cfg.cb = rc_pulse_cb;
    ch_cfg.mode = EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOB;

    /* Assign conversion from pins (pads) to channels */
    /* + init EXT driver */
    int ch;
    for ( ch = 0; ch < MAX_CHANNELS_USED; ch++ )
    {
        uint32_t pad_idx = PAL_PAD( rc_channels[ch].line );

        if ( pad_idx <= MAX_USED_CH_PIN_IDX )
        {
            /* Conversion table */
            pin2channel[pad_idx]  = ch;

            /* EXT init */
            extSetChannelMode( &EXTD1, pad_idx, &ch_cfg );

            dprintf_mod( RADIO_MODULE_PREFIX, "Initialized channel: %d\n", pad_idx );
        }

        /* Set other parameters */
        lld_rc_channel_t    *active_ch = &rc_channels[ch];

        active_ch->value_range              = active_ch->max_value - active_ch->min_value;
        float control_values_range          = MAX_CONTROL_VALUE - MIN_CONTROL_VALUE;

        active_ch->raw_value_2_control_rate = control_values_range / active_ch->value_range;
    }
}

int radio_control_calibration ( void )
{
    /* Wait for first pulses */
    chThdSleepMilliseconds( 100 );

    /* Prepare for calibartion */
    int ch;
    for ( ch = 0; ch < MAX_CHANNELS_USED; ch++ )
    {
        rc_channels[ch].max_value = 0;
        rc_channels[ch].min_value = INT32_MAX;
    }

    /* 50 Hz ~ 20 ms */
    int i;
    for ( i = 0; i < 1000; i++ )
    {
        if ( !rc_connected )
            return -1;

        for ( ch = 0; ch < MAX_CHANNELS_USED; ch++ )
        {   
            lld_rc_channel_t    *active_ch = &rc_channels[ch];

            if ( active_ch->raw_width > active_ch->max_value )
            {
                active_ch->max_value = active_ch->raw_width;
            }

            if ( active_ch->raw_width < active_ch->min_value )
            {
                active_ch->min_value = active_ch->raw_width;
            }
        }

        chThdSleepMilliseconds( 20 );
    }

    dprintf_mod_str( RADIO_MODULE_PREFIX, "Calibration results:\n" );
    for ( ch = 0; ch < MAX_CHANNELS_USED; ch++ )
    {
        dprintf_mod( RADIO_MODULE_PREFIX, "   ch%d: .min_value = %d, .max_value = %d\n", ch, 
                                                rc_channels[ch].min_value, rc_channels[ch].max_value );
    }
}

bool radio_control_is_connected ( void )
{
    return rc_connected;
}

static void rc_pulse_cb( EXTDriver *extp, expchannel_t channel )
{
    gptcnt_t cntr = gptGetCounterX( measure_tmr );

    extp = extp; // Warning avoid

    uint8_t         ch_idx      = channel <= MAX_USED_CH_PIN_IDX ? pin2channel[channel] : CHANNEL_NOT_FILLED;

    if ( ch_idx >= MAX_CHANNELS_USED )
        return;

    /* Avoid unnecessary triggers */
    if ( ch_idx >= 3 && !rc_connected )
        return;

    lld_rc_channel_t    *active_ch  = &rc_channels[ch_idx];

    if ( palReadLine( active_ch->line ) == PAL_HIGH )
    {
        active_ch->front_cntr_value = cntr;
    }
    else
    {
        active_ch->raw_width = cntr - active_ch->front_cntr_value;

        if ( active_ch->raw_width < 0 )
            active_ch->raw_width += MEASURE_TIMER_MAX_COUNTER;

        // Now for debug
        control_input.channels[ch_idx] = (active_ch->raw_width - active_ch->min_value) * active_ch->raw_value_2_control_rate;

        control_input.channels[ch_idx] = clip_value( control_input.channels[ch_idx], MIN_CONTROL_VALUE, MAX_CONTROL_VALUE );
    }

    /* Reset watchdog virtual timer */
    if ( ch_idx < 3 )
    {
        rc_connected = true;

        /* Timer measurement */
#ifdef RC_TIME_MEASUREMENT_DEBUG
        chTMStartMeasurementX( &rc_tm );
#endif

        {
            /* 2-8 us */
            chSysLockFromISR();
            /* Each 60 ms timeout ~ 3 pulses skipped */
            chVTSetI( &connect_vt, MS2ST(20 * 3), rc_connect_timeout, NULL );
            chSysUnlockFromISR();
        }

#ifdef RC_TIME_MEASUREMENT_DEBUG
        chTMStopMeasurementX( &rc_tm );
#endif
    }
}
