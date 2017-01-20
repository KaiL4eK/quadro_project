#include "SDcard.h"

uint8_t                     SDHC_flag = 0;

static UART_moduleNum_t     uart_debug     =   UARTmUndef;

#define CMD_BEGIN   0x40
#define CRC         0x95

uint8_t SD_send_cmd ( SD_command_t command, uint32_t argument )
{
    uint8_t     response    = 0xFF; 
    uint8_t     retry       = 0;
    
    if ( SDHC_flag == 0 )	
    {
        switch ( command )
        {
            case READ_SINGLE_BLOCK:
            case READ_MULTIPLE_BLOCKS:
            case WRITE_SINGLE_BLOCK:
            case WRITE_MULTIPLE_BLOCKS:
            case ERASE_BLOCK_START_ADDR:
            case ERASE_BLOCK_END_ADDR:
                argument <<= 9; // Multiplying 512
                break;
            default:
                ;
        } 
    }
    
    spi_cs_set( 0 );
    
    spi_write( (command & 0x3f) | CMD_BEGIN );
    spi_write( argument >> 24 );
    spi_write( argument >> 16 );
    spi_write( argument >> 8  );
    spi_write( argument       );
    spi_write( ( command == SEND_IF_COND ? 0x87 : CRC ) );
    
    while ( (response = spi_read()) & 0x80/*== 0xFF*/ )
    {
        if ( retry++ == UINT8_MAX )
            break;
    }
    
    if ( response == 0 && command == READ_OCR )
    {
        uint8_t ocr_resp = 0;
        SDHC_flag = (((ocr_resp = spi_read()) & 0x40) == 0x40) ? 1 : 0;
        ocr_resp = spi_read();
        ocr_resp = spi_read();
        ocr_resp = spi_read();
    }
    
    if ( command == SEND_IF_COND )
    {
        uint64_t r7_resp;
        r7_resp = (response << 8) | spi_read();
        r7_resp = (r7_resp << 8) | spi_read();
        r7_resp = (r7_resp << 8) | spi_read();
        r7_resp = (r7_resp << 8) | spi_read();
        r7_resp = (r7_resp << 8) | spi_read();
        r7_resp = (r7_resp << 8) | spi_read();
    }
    
    spi_cs_set( 1 );
    
    return( response );
}

typedef struct {
    unsigned idle_state:1;
    unsigned erase_reset:1;
    unsigned illegal_command:1;
    unsigned command_crc_error:1;
    unsigned erase_sequence_error:1;
    unsigned address_error:1;
    unsigned parameter_error:1;
    unsigned zero:1;
} r1_response_t;

#define IN_IDLE_STATE    0x01

int SD_initialize ( UART_moduleNum_t uart )
{    
//    r1_response_t   response;
    uint8_t         response    = 0xFF;
    uint8_t         SD_version  = 2,
                    i           = 0,
                    card_type   = 0;

    uint8_t         retry = 0;
    
    uart_debug = uart;
    
    spi_set_speed( SPI_SPEED_LOW );
    
    spi_cs_set( 1 );
    for ( i = 0; i < 10; i++ )                        // 2. send 80 clock cycles so card can init registers
         spi_read();
    spi_cs_set( 0 );
    
    while ( (response = SD_send_cmd( GO_IDLE_STATE, 0 )) != IN_IDLE_STATE ) // 0
    {
//        UART_write_string( uart_debug, "GO_IDLE_STATE: 0x%x\n", response );
        if ( retry++ == UINT8_MAX )
            return( TIMEOUT_ERROR_INIT_1 );
    }
//    UART_write_string( uart_debug, "GO_IDLE_STATE: 0x%x\n", response );
    
    retry = 0;
    
    while ( (response = SD_send_cmd( SEND_IF_COND, 0x1AA )) != IN_IDLE_STATE )  // 8
    {
//        UART_write_string( uart_debug, "SEND_IF_COND: 0x%x\n", response );
        if ( retry++ == UINT8_MAX )
        {
            UART_write_string( uart_debug, "SD card version 1\n" );
            SD_version = 1;
            card_type = 1;
            break;
        }
    }
//    UART_write_string( uart_debug, "SEND_IF_COND: 0x%x\n", response );
    
    retry = 0;

    while ( 1 ) 
    {
        SD_send_cmd( APP_CMD, 0 );
        if ( SD_send_cmd( SD_SEND_OP_COND, 1UL << 30 ) != 0 ) {
            if ( retry++ == UINT8_MAX ) {
                UART_write_string( uart_debug, "Failed to init SD card ACMD41 %d\n", response );
                return( TIMEOUT_ERROR_INIT_3 );
            }
            continue;
        } else 
            break;
        
    }

    retry = 0;
    SDHC_flag = 0;
    
    if( SD_version == 2 )
    {
        while ( (response = SD_send_cmd( READ_OCR, 0 )) != 0 )
            if ( retry++ == UINT8_MAX )
            {
                card_type = 0;
                break;
            }

        card_type = (SDHC_flag == 1) ? 2 : 3;
    }
    
    return( NO_ERROR );
}

/*
uint8_t SD_erase ( uint32_t start_block, uint32_t total_block )
{
    uint8_t response = 0;
    
    if ( (response = SD_send_cmd( ERASE_BLOCK_START_ADDR, start_block )) != 0 )
        return( response );
    
    if ( (response = SD_send_cmd( ERASE_BLOCK_END_ADDR, (start_block + total_block - 1) )) != 0 )
        return( response );

    if ( (response = SD_send_cmd( ERASE_SELECTED_BLOCKS, 0 )) != 0 )
        return( response );
    
    return( NO_ERROR );
}
*/

int SD_read_sector ( uint32_t start_block, uint8_t *buffer )
{
    uint8_t response = 0, 
            retry = 0;
    uint16_t i = 0;

    while ( (response = SD_send_cmd( READ_SINGLE_BLOCK, start_block )) != 0 )
        if ( retry++ == UINT8_MAX )
        {
            UART_write_string( uart_debug, "Reading command wasn`t accepted\n" );
            return( TIMEOUT_ERROR_READ );
        }
    
    spi_cs_set( 0 );
    
    retry = 0;
    while ( (response = spi_read()) != 0xfe )
        if ( retry++ == UINT8_MAX )
        {
            spi_cs_set( 1 );
            return( TIMEOUT_ERROR_READ );
        }

    for ( i = 0; i < 512; i++ )
    {
        buffer[i] = spi_read();
    }
    // CRC
    spi_read();
    spi_read();
    
    spi_read();
    
    spi_cs_set( 1 );
    return( NO_ERROR );    
}

int SD_write_sector ( uint32_t start_block, uint8_t *buffer )
{
    uint8_t response = 0,
            retry = 0;
    uint16_t i = 0;
    
    while ( (response = SD_send_cmd( WRITE_SINGLE_BLOCK, start_block )) != 0 )
        if ( retry++ == UINT8_MAX )
        {
            UART_write_string( uart_debug, "Writing command wasn`t accepted\n" );
            return( TIMEOUT_ERROR_WRITE );
        }
        
    spi_cs_set( 0 );
    
    spi_write( 0xfe );
    
    for ( i = 0; i < 512; i++ )
    {
        spi_write( buffer[i] );
    }
    // CRC
    spi_write( 0x00 );
    spi_write( 0x00 );
    
    if ( ((response = spi_read()) & 0x1f) != WRITING_ACCEPTED )
    {
        spi_read();
        spi_cs_set( 1 );
        UART_write_string( uart_debug, "SD writing error: 0x%08x\n", response );
        error_process( "SD card write error" );
        return( TIMEOUT_ERROR_WRITE_NOT_ACCEPTED );
    }

    // Waiting for writing completed
    while ( !(response = spi_read()) );
    
    spi_cs_set( 1 );
    spi_read();
    spi_cs_set( 0 );
    
    while ( !(response = spi_read()) );
    spi_cs_set( 1 );
    
    return( NO_ERROR );
}


