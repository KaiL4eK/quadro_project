/*
 * File:   emain.c
 * Author: alexey
 *
 * Created on June 4, 2016, 12:37 PM
 */

#include "core.h"
#include "pragmas.h"

#define ENCODERS_ENABLED

uint8_t     sendFlag = 0;
uint32_t    encoderRoll = 0,
            encoderPitch = 0;

int main ( void ) 
{
    OFF_ALL_ANALOG_INPUTS;
    // UARTm1 - USB
    // UARTm2 - dsPIC on Quadro
    UART_init( UARTm1, UART_460800, true );
    UART_init( UARTm2, UART_38400, false );
    UART_write_string( UARTm1, "UART initialized\n" );
    
    cmdProcessor_init();
    UART_write_string( UARTm1, "Start!\n" );
#ifdef ENCODERS_ENABLED 
    _TRISD9 = _TRISD8 = _TRISD10 = _TRISD11 = 1;
    
    IC1CONbits.ICM = IC_CE_MODE_DISABLED;
    IC1CONbits.ICTMR = IC_TIMER_2;
    IC1CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC1CONbits.ICM = IC_CE_MODE_EDGE;
    _IC1IP = INT_PRIO_MID;
    _IC1IF = 0;     // Zero interrupt flag
    _IC1IE = 1;     // Enable interrupt
/*
    IC2CONbits.ICM = IC_CE_MODE_DISABLED;
    IC2CONbits.ICTMR = IC_TIMER_2;
    IC2CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC2CONbits.ICM = IC_CE_MODE_EDGE;
    _IC2IP = INT_PRIO_MID;
    _IC2IF = 0;     // Zero interrupt flag
    _IC2IE = 1;     // Enable interrupt
*/
    IC3CONbits.ICM = IC_CE_MODE_DISABLED;
    IC3CONbits.ICTMR = IC_TIMER_2;
    IC3CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC3CONbits.ICM = IC_CE_MODE_EDGE;
    _IC3IP = INT_PRIO_MID;
    _IC3IF = 0;     // Zero interrupt flag
    _IC3IE = 1;     // Enable interrupt
/*
    IC4CONbits.ICM = IC_CE_MODE_DISABLED;
    IC4CONbits.ICTMR = IC_TIMER_2;
    IC4CONbits.ICI = IC_INT_MODE_1ST_CE;
    IC4CONbits.ICM = IC_CE_MODE_EDGE;
    _IC4IP = INT_PRIO_MID;
    _IC4IF = 0;     // Zero interrupt flag
    _IC4IE = 1;     // Enable interrupt
*/
#endif
    
    bool        sendData = false;
    
    while ( 1 ) 
    {
        UART_frame_t *frame = cmdProcessor_rcvFrame();
        switch ( frame->command )
        {
            case CONNECT:
                cmdProcessor_write_cmd_resp( UARTm2, 
                        COMMAND_PREFIX, CMD_CONNECT_CODE );
                
                if ( cmdProcessor_U2_waitResponse() < 0 )
                    cmdProcessor_write_cmd_resp( UARTm1, 
                            RESPONSE_PREFIX, RESP_NOCONNECT );
                else
                    cmdProcessor_write_cmd_resp( UARTm1, 
                            RESPONSE_PREFIX, RESP_NOERROR );
                break;
            case DATA_START:
                sendData = true;
                cmdProcessor_cleanBuffer( UARTm2 );
                cmdProcessor_write_cmd_resp( UARTm2, 
                        COMMAND_PREFIX, CMD_DATA_START_CODE );
                
//                cmdProcessor_write_cmd_resp( UARTm1, 
//                        RESPONSE_PREFIX, RESP_NOERROR );
                break;
            case DATA_STOP:
                sendData = false;
                cmdProcessor_write_cmd_resp( UARTm2, 
                        COMMAND_PREFIX, CMD_DATA_STOP_CODE );
                
//                cmdProcessor_write_cmd_resp( UARTm1, 
//                        RESPONSE_PREFIX, RESP_ENDDATA );
                break;
            case MOTOR_START:
                cmdProcessor_write_cmd_resp( UARTm2, 
                        PARAMETER_PREFIX, PARAM_MOTOR_START );
                UART_write_byte( UARTm2, frame->motorPower );
                
                break;
            case MOTOR_STOP:
                cmdProcessor_write_cmd_resp( UARTm2, 
                        PARAMETER_PREFIX, PARAM_MOTOR_STOP );
                UART_write_byte( UARTm2, 0 );

                break;
            case UNKNOWN_COMMAND:
                cmdProcessor_write_cmd_resp( UARTm1, 
                        RESPONSE_PREFIX, RESP_NOCONNECT );
                break;
            default:
                break;
        }
        
        if ( sendData )
        {
            uint16_t    quadroData[7];
            int         res             = 0;
            while ( (res = cmdProcessor_U2_rcvData( quadroData )) < 0 ) { Nop(); }
            if ( res == 1 ) {
                cmdProcessor_write_cmd_resp( UARTm1, RESPONSE_PREFIX, RESP_ENDDATA );
                sendData = false;
            }
            // encoder: 180 degree = 1000 points
#define DEGREE_PER_100_POINTS 18
            
            quadroData[5] = (encoderRoll * DEGREE_PER_100_POINTS);  // angle * 100
            quadroData[6] = (encoderPitch * DEGREE_PER_100_POINTS);
            UART_write_byte( UARTm1, DATA_PREFIX );
            UART_write_words( UARTm1, quadroData, 7 );
        }
//        UART_write_string( UARTm1, "%ld %ld\n", encoderRoll, encoderPitch );
//        delay_ms( 100 );
    }
    
    return ( 0 );
}
#ifdef ENCODERS_ENABLED 
void __attribute__( (__interrupt__, no_auto_psv) ) _IC1Interrupt()
{
    if ( _RD8 )
        encoderRoll += (_RD9<<1)-1;
    else
        encoderRoll += 1-(_RD9<<1);
    
    uint16_t trash = IC1BUF;
    _IC1IF = 0;
}
/*
void __attribute__( (__interrupt__, no_auto_psv) ) _IC2Interrupt()
{
    if ( _RD9 )
        encoderRoll += 1-(_RD8<<1);
    else
        encoderRoll += (_RD8<<1)-1;
    
    uint16_t trash = IC2BUF;
    _IC2IF = 0;
}
*/
void __attribute__( (__interrupt__, no_auto_psv) ) _IC3Interrupt()
{
    if ( _RD10 )
        encoderPitch += (_RD11<<1)-1;
    else
        encoderPitch += 1-(_RD11<<1);
    
    uint16_t trash = IC3BUF;
    _IC3IF = 0;
}
/*
void __attribute__( (__interrupt__, no_auto_psv) ) _IC4Interrupt()
{
    if ( _RD11 )
        encoderPitch += 1-(_RD10<<1);
    else
        encoderPitch += (_RD10<<1)-1;
    
    uint16_t trash = IC4BUF;
    _IC4IF = 0;
}
*/
#endif