#include "core.h"
#include "serial_protocol.h"

uart_module_t m_module = -1;

void cmdProcessor_init ( uart_module_t module )
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
                    case CMD_DATA_START_CODE:
                        frame.command = DATA_START;
                        break;
                    case CMD_DATA_STOP_CODE:
                        frame.command = DATA_STOP;
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
        } else if ( prefix_byte == PARAMETER_PREFIX ) {
            if ( UART_bytes_available( m_module ) >= PARAMETER_FRAME_SIZE ) 
            {
                uint8_t parameter_byte = UART_get_byte( m_module );
                uint8_t value_byte     = UART_get_byte( m_module );

                switch( parameter_byte )
                {
                    case PARAM_MOTOR_POWER:
                        frame.command = MOTOR_SET_POWER;
                        frame.motorPower = value_byte;
                        break;
                    default:
                        frame.command = UNKNOWN_COMMAND;
                }
                prefix_byte = 0;
            }
        } else
            prefix_byte = 0;
    }
    
    return( &frame );
}

void cmdProcessor_write_cmd ( uart_module_t module, uint8_t prefix, uint8_t code )
{
    uint16_t sendCommand = (uint16_t)prefix << 8 | code;
    UART_write_words( module, &sendCommand, 1 );
}
