#include "timing.h"
#include "per_proto.h"

void setup_PLL_oscillator( void )
{
    PLLFBDbits.PLLDIV = 38; // M=40 Fvco = 160Mhz
    CLKDIVbits.PLLPOST = 0; // N1=2 Fpost = 80Mhz
    CLKDIVbits.PLLPRE = 6;  // N2=8 Fpre = 4Mhz

    __builtin_write_OSCCONH(0x03);  // Initiate Clock Switch to Primary 
               // Oscillator with PLL (NOSC=0b011) 
    __builtin_write_OSCCONL(0x01);  // Start clock switching 
    while (OSCCONbits.COSC != 0b011) ;
    while (OSCCONbits.LOCK != 1) ;
}

/********************************/
/*              DELAYS          */
/********************************/

void delay_ms( uint16_t mseconds )
{
    if ( mseconds * TIMER_MS_TICK > UINT32_MAX )
    {
        return;
    }
    T6CONbits.TON = 0;  // Disable timer
    T6CONbits.T32 = 1;  // 32-bit timer
    T6CONbits.TCKPS = TIMER_DIV_1; // Prescale bits 1:1
    TMR7HLD = 0;
    TMR6 = 0;
    T6CONbits.TON = 1;  // Enable timer
    while( (TMR6 | ((uint32_t)TMR7HLD) << 16) < (mseconds*TIMER_MS_TICK) );
    TMR7HLD = 0;
    TMR6 = 0;
    T6CONbits.TON = 0;
}

void delay_us( uint16_t useconds )
{
    if ( useconds * TIMER_US_TICK > UINT32_MAX )
    {
        return;
    }
    T6CONbits.TON = 0;  // Disable timer
    T6CONbits.T32 = 1;  // 32-bit timer
    T6CONbits.TCKPS = TIMER_DIV_1; // Prescale bits 1:1
    TMR7HLD = 0;
    TMR6 = 0;
    T6CONbits.TON = 1;  // Enable timer
    while( (TMR6 | ((uint32_t)TMR7HLD) << 16) < (useconds*TIMER_US_TICK) );
    TMR7HLD = 0;
    TMR6 = 0;
    T6CONbits.TON = 0;
}

void timer_start()
{
    T8CONbits.TON = 0;
    T8CONbits.T32 = 1;
    T8CONbits.TCKPS = TIMER_DIV_1;
    TMR9HLD = 0;
    TMR8 = 0;
    T8CONbits.TON = 1;
}

uint32_t timer_restart()
{
    uint32_t res = TMR8 | ((uint32_t)TMR9HLD) << 16;
    timer_start();
    return( res );
}

uint32_t timer_stop()
{
    uint32_t res = TMR8 | ((uint32_t)TMR9HLD) << 16;
    T8CONbits.TON = 0;
    return( res );
}
