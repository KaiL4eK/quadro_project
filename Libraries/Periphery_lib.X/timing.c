#include "per_proto.h"
#include <libpic30.h>

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

#define TIMER_MS_TICK (FCY/1000)
#define TIMER_US_TICK (FCY/1000000)

void delay_ms( uint16_t mseconds )
{
    __delay_ms( mseconds );
    
//    T6CONbits.TON = 0;  // Disable timer
//    T6CONbits.T32 = 1;  // 32-bit timer
//    T6CONbits.TCKPS = TIMER_DIV_1; // Prescale bits 1:1
//    TMR7HLD = 0;
//    TMR6 = 0;
//    T6CONbits.TON = 1;  // Enable timer
//    while( (TMR6 | ((uint32_t)TMR7HLD) << 16) < (mseconds*TIMER_MS_TICK) );
//    TMR7HLD = 0;
//    TMR6 = 0;
//    T6CONbits.TON = 0;
}

void delay_us( uint16_t useconds )
{
    __delay_us( useconds );
    
//    T6CONbits.TON = 0;  // Disable timer
//    T6CONbits.T32 = 1;  // 32-bit timer
//    T6CONbits.TCKPS = TIMER_DIV_1; // Prescale bits 1:1
//    TMR7HLD = 0;
//    TMR6 = 0;
//    T6CONbits.TON = 1;  // Enable timer
//    while( (TMR6 | ((uint32_t)TMR7HLD) << 16) < (useconds*TIMER_US_TICK) );
//    TMR7HLD = 0;
//    TMR6 = 0;
//    T6CONbits.TON = 0;
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

TimerTicks32_t timer_restart()
{
    uint32_t res = TMR8 | ((uint32_t)TMR9HLD) << 16;
    timer_start();
    return( res );
}

TimerTicks32_t timer_stop()
{
    uint32_t res = TMR8 | ((uint32_t)TMR9HLD) << 16;
    T8CONbits.TON = 0;
    return( res );
}

uint32_t convert_ticks_to_ms ( TimerTicks32_t timer_ticks, uint8_t timer_divider )
{
    return( timer_ticks*timer_divider/TIMER_MS_TICK );
}

uint32_t convert_ticks_to_us ( TimerTicks32_t timer_ticks, uint8_t timer_divider )
{
    return( timer_ticks*timer_divider/TIMER_US_TICK );
}
