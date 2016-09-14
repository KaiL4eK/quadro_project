/*
 * File:   timing.c
 * Author: Alex Devyatkin
 *
 */

#include "core.h"
#include <libpic30.h>

void pll_32MHz_init ( void )
{

}

void delay_ms ( uint16_t time_ms )
{
    __delay_ms( time_ms );
    return;
#if 0
    TMR1 = 0;                   // Сброс таймера
    T1CONbits.TCKPS = 0b01;    // Настройка на делителе 8
    PR1 = FCY/8/1000;          // Расчет на работу таймера до переполенния в течении 1 мс
                                // FCY / Div * 0.001 sec
    _T1IE = 1;
    T1CONbits.TON = 1;       // Включаем таймер

    uint16_t msCounter = 0;     // Каждые 1 мс таймер увеличивается на 1
    while ( msCounter < time_ms )   // Счет до получения требуемых микросекунд
    {
        while ( !_T1IF ) {  }  // Ожидаем переполнения
        _T1IF = 0;             // очищаем флаг
        msCounter++;            // Отсчитали 1 мс - инкрементируем таймер
    }
    
    T1CONbits.TON = 0;       // Отключаем таймер
#endif
}

void delay_us ( uint16_t time_us )
{
    __delay_us( time_us );
    return;
#if 0
    TMR1 = 0;                   // Сброс таймера
    T1CONbits.TCKPS = 0b00;    // Настройка на делителе 1
    PR1 = FCY/1000000;         // Расчет на работу таймера до переполенния в течении 1 мкс
                                // FCY / Div * 0.000001 sec
    _T1IE = 1;
    T1CONbits.TON = 1;       // Включаем таймер
    
    uint16_t usCounter = 0;     // Каждые 1 мкс таймер увеличивается на 1
    while ( usCounter < time_us )   // Счет до получения требуемых микросекунд
    {
        while ( !_T1IF ) { }   // Ожидаем переполнения
        _T1IF = 0;             // очищаем флаг
        usCounter++;            // Отсчитали 1 мкс - инкрементируем таймер
    }
    
    T1CONbits.TON = 0;       // Отключаем таймер
#endif
}
