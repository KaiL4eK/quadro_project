/*
 * File:   pwm.c
 * Author: Alexey Devyatkin
 *
 */


#include "core.h"

uint32_t pwm_period_tmp_cnts = 0,
         cntrlRange_min = 0,
         cntrlRange_max = 0;

void pwm_init ( uint16_t freq )
{
// Zero timer counter
    TMR2 = 0;
//RP29R<5:0>: RP29 Output Pin Mapping bits
//18 OC1 Output Compare 1
    RPOR14bits.RP29R = 18;  // PTP6
//Duty cycle
    OC1R = 0;
//TCKPS<1:0>: Timerx Input Clock Prescale Select bits
//11 = 1:256
//10 = 1:64
//01 = 1:8
//00 = 1:1
    T2CONbits.TCKPS = 0b01;
//Period
    OC1RS = PR2 = cntrlRange_max = pwm_period_tmp_cnts = FCY/8/freq;
//SYNCSEL<4:0>: Trigger/Synchronization Source Selection bits
//11111 = This OC module (1)
//11110 = Input Capture 9 (2)
//11101 = Input Capture 6 (2)
//11100 = CTMU (2)
//11011 = A/D (2)
//11010 = Comparator 3 (2)
//11001 = Comparator 2 (2)
//11000 = Comparator 1 (2)
//10111 = Input Capture 4 (2)
//10110 = Input Capture 3 (2)
//10101 = Input Capture 2 (2)
//10100 = Input Capture 1 (2)
//10011 = Input Capture 8 (2)
//10010 = Input Capture 7 (2)
//1000x = reserved
//01111 = Timer5
//01110 = Timer4
//01101 = Timer3
//01100 = Timer2
//01011 = Timer1
//01010 = Input Capture 5 (2)
//01001 = Output Compare 9 (1)
//01000 = Output Compare 8 (1)
//00111 = Output Compare 7 (1)
//00110 = Output Compare 6 (1)
//00101 = Output Compare 5 (1)
//00100 = Output Compare 4 (1)
//00011 = Output Compare 3 (1)
//00010 = Output Compare 2 (1)
//00001 = Output Compare 1 (1)
//00000 = Not synchronized to any other module
    OC1CON2bits.SYNCSEL = 0b01100;
//OCTRIG: OCx Trigger/Sync Select bit
//1 = Trigger OCx from source designated by SYNCSELx bits
//0 = Synchronize OCx with source designated by SYNCSELx bits
    OC1CON2bits.OCTRIG = 0;
//OCTSEL<2:0>: Output Compare x Timer Select bits
//111 = Peripheral Clock (F CY )
//110 = Reserved
//101 = Reserved
//100 = Timer1
//011 = Timer5
//010 = Timer4
//001 = Timer3
//000 = Timer2
    OC1CON1bits.OCTSEL = 0;
//OCM<2:0>: Output Compare x Mode Select bits (1)
//111 = Center-Aligned PWM mode on OCx (2)
//110 = Edge-Aligned PWM mode on OCx (2)
//101 = Double Compare Continuous Pulse mode: initialize OCx pin low, toggle OCx state continuously
//on alternate matches of OCxR and OCxRS
//100 = Double Compare Single-Shot mode: initialize OCx pin low, toggle OCx state on matches of OCxR
//and OCxRS for one cycle
//011 = Single Compare Continuous Pulse mode: compare events continuously toggle OCx pin
//010 = Single Compare Single-Shot mode: initialize OCx pin high, compare event forces OCx pin low
//001 = Single Compare Single-Shot mode: initialize OCx pin low, compare event forces OCx pin high
//000 = Output compare channel is disabled
    OC1CON1bits.OCM = 0b110;
//1 = Starts 16-bit Timerx
//0 = Stops 16-bit Timerx
    T2CONbits.TON = 1;
}

void pwm_set_dutyCycle_percent ( uint8_t percentage )
{
    if ( percentage > 100 )
        percentage = 100;
    
    OC1R = ((cntrlRange_max - cntrlRange_min) * percentage)/100;
}

void pwm_set_cntrlRange ( int16_t min_prc, int16_t max_prc )
{
    cntrlRange_max = pwm_period_tmp_cnts * min_prc / 100;
    cntrlRange_min = pwm_period_tmp_cnts * max_prc / 100;
}
