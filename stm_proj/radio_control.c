#include <core.h>


static GPTDriver *measure_tmr = &GPTD7;
static const GPTConfig tmr_cfg = {
    /* 0.0655s = 65.5ms */
    .frequency  = 1000000,
    .callback   = NULL,
    .cr2        = 0,
    .dier       = 0
};

#define MEASURE_TIMER_MAX_COUNTER   UINT16_MAX

typedef struct {
    int32_t     raw_width;
    int32_t     front_cntr_value;
    ioline_t    line;

} rc_channel_t;

static rc_channel_t rc_channels[4];


static void rc_pulse_cb( EXTDriver *extp, expchannel_t channel );

void radio_control_init ( void )
{
    /* Zero variables */
    memset( rc_channels, 0, sizeof( rc_channels ) );

    /* TODO - Replace modes! */
    EXTChannelConfig ch_cfg;
    ch_cfg.cb = rc_pulse_cb;

    ch_cfg.mode = EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC;
    extSetChannelMode( &EXTD1, 5, &ch_cfg );

    ch_cfg.mode = EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC;
    extSetChannelMode( &EXTD1, 5, &ch_cfg );

    ch_cfg.mode = EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC;
    extSetChannelMode( &EXTD1, 5, &ch_cfg );

    ch_cfg.mode = EXT_CH_MODE_BOTH_EDGES | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOC;
    extSetChannelMode( &EXTD1, 5, &ch_cfg );

    gptStart( measure_tmr, &tmr_cfg );
    /* UINT16_MAX - 16bit timer */
    gptStartContinuous( measure_tmr, MEASURE_TIMER_MAX_COUNTER );
}

static void rc_pulse_cb( EXTDriver *extp, expchannel_t channel )
{
    gptcnt_t cntr = gptGetCounterX( measure_tmr );

    extp = extp; // Warning avoid

    /* TODO - Choose valid channel based on <channel> arg */
    rc_channel_t   *active_ch = &rc_channels[0];

    if ( palReadLine( active_ch->line ) )
    {
        active_ch->front_cntr_value = cntr;
    }
    else
    {
        active_ch->raw_width = cntr - active_ch->front_cntr_value;

        if ( active_ch->raw_width < 0 )
            active_ch->raw_width += MEASURE_TIMER_MAX_COUNTER;
    }
}
