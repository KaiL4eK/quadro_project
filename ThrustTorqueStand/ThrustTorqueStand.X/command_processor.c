#include "core.h"

UART_moduleNum_t m_module = -1;

void cmdProcessor_init ( UART_moduleNum_t module )
{
    m_module = module;
}

UART_frame_t    frame;
uint8_t         prefix_byte = 0;

UART_frame_t *cmdProcessor_rcvFrame ( void )
{
    frame.command = NO_COMMAND;
    
    if ( UART_bytes_available( m_module ) > 0 )
    {
        if ( !prefix_byte ) {
            prefix_byte = UART_get_byte( m_module );
        }
        
        if ( prefix_byte == COMMAND_PREFIX ) {
            if ( UART_bytes_available( m_module ) >= COMMAND_FRAME_SIZE ) 
            {
                uint8_t command_byte = UART_get_byte( m_module );
                switch( command_byte )
                {
                    case CMD_CONNECT_CODE:
                        frame.command = CONNECT;
                        break;
                    case CMD_DISCONNECT_CODE:
                        frame.command = DISCONNECT;
                        break;
                    case CMD_MOTOR_STOP:
                        frame.command = MOTOR_STOP;
                        break;
                    case CMD_MOTOR_START:
                        frame.command = MOTOR_START;
                        break;
                    default:
                        frame.command = UNKNOWN_COMMAND;
                }
                prefix_byte = 0;
            }
        } else if ( prefix_byte == PARAMETERS_PREFIX ) {
            if ( UART_bytes_available( m_module ) >= PARAMETER_FRAME_SIZE ) 
            {
                uint8_t buffer[PARAMETER_FRAME_SIZE];
                
                UART_get_bytes( m_module, buffer, PARAMETER_FRAME_SIZE );
// Redo for universal QT-MPLAB interface
                frame.command               = MEASURE_SET_PARAMS;
                frame.motorPowerStart       = buffer[0];
                frame.motorPowerEnd         = buffer[1];
                frame.timeMeasureStartMs    = (uint16_t)buffer[2] << 8 | buffer[3];
                frame.timeMeasureDeltaMs    = (uint16_t)buffer[4] << 8 | buffer[5];
                frame.timeStepMomentMs      = (uint16_t)buffer[6] << 8 | buffer[7];
                
                prefix_byte = 0;
            }
        } else
            prefix_byte = 0;
    }
    
    return( &frame );
}

void cmdProcessor_response ( uint8_t code )
{
    uint16_t sendCommand = (uint16_t)RESPONSE_PREFIX << 8 | code;
    UART_write_words( m_module, &sendCommand, 1 );
}
