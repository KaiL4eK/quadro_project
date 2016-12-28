#include "core.h"
#include "serial_protocol.h"

UART_moduleNum_t m_module_from = -1;
UART_moduleNum_t m_module_to   = -1;

void command_translator_init ( UART_moduleNum_t module_from, UART_moduleNum_t module_to )
{
    m_module_from = module_from;
    m_module_to   = module_to;
}

void command_translator ( void )
{    
    static uint8_t cmd_prefix_byte = 0;
    
    if ( UART_bytes_available( m_module_from ) > 0 )
    {
        if ( !cmd_prefix_byte )
            cmd_prefix_byte = UART_get_byte( m_module_from );
        
        if ( cmd_prefix_byte == COMMAND_PREFIX ) {
            if ( UART_bytes_available( m_module_from ) >= COMMAND_FRAME_SIZE )
            {
                uint8_t command_byte = UART_get_byte( m_module_from );

                UART_write_byte( m_module_to, cmd_prefix_byte );
                UART_write_byte( m_module_to, command_byte );
                
                cmd_prefix_byte = 0;
            }
        } else if ( cmd_prefix_byte == PARAMETER_PREFIX ) {
            if ( UART_bytes_available( m_module_from ) >= PARAMETER_FRAME_SIZE )
            {
                uint8_t parameter_byte = UART_get_byte( m_module_from );
                uint8_t value_byte     = UART_get_byte( m_module_from );
                
                UART_write_byte( m_module_to, cmd_prefix_byte );
                UART_write_byte( m_module_to, parameter_byte );
                UART_write_byte( m_module_to, value_byte );
                
                cmd_prefix_byte = 0;
            }
        } else
            cmd_prefix_byte = 0;
    }
}

//------------------------------------------------------------------------------------------



void data_translator ( int32_t encoder_roll, int32_t encoder_pitch )
{
    static uint8_t     data_resp_prefix_byte = 0;
    static uint16_t    full_buffer[DATA_FULL_FRAME_SIZE/2];
    
    if ( UART_bytes_available( m_module_to ) > 0 )
    {
        if ( !data_resp_prefix_byte )
            data_resp_prefix_byte = UART_get_byte( m_module_to );
        
        if ( data_resp_prefix_byte == DATA_PREFIX ) {
            if ( UART_bytes_available( m_module_to ) >= DATA_QUADRO_FRAME_SIZE ) 
            {
                uint8_t buffer[DATA_QUADRO_FRAME_SIZE];

                UART_get_bytes( m_module_to, buffer, DATA_QUADRO_FRAME_SIZE );

//                full_buffer[0] = (uint16_t)buffer[0] << 8 | buffer[1];
//                full_buffer[1] = (uint16_t)buffer[2] << 8 | buffer[3];
//                full_buffer[2] = (uint16_t)buffer[4] << 8 | buffer[5];
//                full_buffer[3] = (uint16_t)buffer[6] << 8 | buffer[7];
//                full_buffer[4] = (uint16_t)buffer[8] << 8 | buffer[9];

                memcpy( full_buffer, buffer, DATA_QUADRO_FRAME_SIZE );
                
                // encoder: 180 degree = 1000 points
#define DEGREE_PER_100_POINTS 18
                
                full_buffer[5] = (encoder_roll  * DEGREE_PER_100_POINTS);  // angle * 100
                full_buffer[6] = (encoder_pitch * DEGREE_PER_100_POINTS);

                UART_write_byte( m_module_from, data_resp_prefix_byte );
                UART_write_words( m_module_from, full_buffer, DATA_FULL_FRAME_SIZE/2 );

                data_resp_prefix_byte = 0;
            }
        } else if ( data_resp_prefix_byte == RESPONSE_PREFIX ) {
            if ( UART_bytes_available( m_module_to ) >= RESPONSE_FRAME_SIZE ) {
                uint8_t resp_byte = UART_get_byte( m_module_to );

                UART_write_byte( m_module_from, data_resp_prefix_byte );
                UART_write_byte( m_module_from, resp_byte );

                data_resp_prefix_byte = 0;
            }
        } else {
            data_resp_prefix_byte = 0;
        }
    }
}
