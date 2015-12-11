#include "SDcard.h"
#include "per_proto.h"

uint8_t     SDHC_flag = 0, 
            card_type = 0;

#define CMD_BEGIN   0x40
#define CRC         0x95

uint8_t SD_send_cmd ( SD_command_t command, uint32_t argument )
{
    uint8_t response = 0xFF, retry = 0;
    
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
            default:;
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
    
    // Here we received first correct response and can get other response bytes
    
//    if ( command == READ_OCR )
//    {
//        fdebug( "OCR_R1", response );
//    }
    
    if ( response == 0 && command == READ_OCR )
    {
        uint8_t ocr_resp = 0;
        SDHC_flag = (((ocr_resp = spi_read()) & 0x40) == 0x40) ? 1 : 0;
//        fdebug( "OCR_R[1]", ocr_resp );
        ocr_resp = spi_read();
//        fdebug( "OCR_R[2]", ocr_resp );
        ocr_resp = spi_read();
//        fdebug( "OCR_R[3]", ocr_resp );
        ocr_resp = spi_read();
//        fdebug( "OCR_R[4]", ocr_resp );
    }
    
    spi_cs_set( 1 );
    
    return( response );
}

#define IN_IDLE_STATE    0x01

int init_SDcard ( void )
{    
    uint8_t response = 0xFF, 
            retry = 0, 
            SD_version = 2,
            i = 0;
    
    spi_cs_set( 1 );
    for ( i = 0; i < 10; i++ )                        // 2. send 80 clock cycles so card can init registers
         spi_read();
    spi_cs_set( 0 );
    
    spi_set_speed( FREQ_125K );
    
    while ( (response = SD_send_cmd( GO_IDLE_STATE, 0 )) != IN_IDLE_STATE ) // 0
        if ( retry++ == UINT8_MAX )
            return( TIMEOUT_ERROR_INIT_1 );
    
    retry = 0;
    
    while ( (response = SD_send_cmd( SEND_IF_COND, 0x1AA )) != IN_IDLE_STATE )  // 8
        if ( retry++ == UINT8_MAX )
        {
            SD_version = 1;
            card_type = 1;
            break;
        }
    
    retry = 0;

    do
    {
        response = SD_send_cmd( APP_CMD, 0 );   // 55
//fdebug( "APP_CMD_R", response );
        response = SD_send_cmd( SD_SEND_OP_COND, 1UL << 30 );   // 41 setting HCS bit
//fdebug( "SD_SEND_OP_COND_R", response );
        if ( retry++ == UINT8_MAX )
        {
            return( TIMEOUT_ERROR_INIT_2 );
        }
    } while( response != 0 );

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
            UART_write_string( "Reading command wasn`t accepted\n" );
            return( TIMEOUT_ERROR_READ );
        }
    
    spi_cs_set( 0 );
    
//    timer_start();
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
    
//    uint32_t time_elapsed = timer_stop()/16;
//    idebug("SDR", time_elapsed);
    
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
            UART_write_string( "Writing command wasn`t accepted\n" );
            return( TIMEOUT_ERROR_WRITE );
        }
        
    spi_cs_set( 0 );
    
//    timer_start();

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
        UART_write_string( "SD writing error: 0x%08x\n", response );
        error_process();
        return( TIMEOUT_ERROR_WRITE_NOT_ACCEPTED );
    }

    // Waiting for writing completed
    while ( !(response = spi_read()) );
    
    spi_cs_set( 1 );
    spi_read();
    spi_cs_set( 0 );
    
    while ( !(response = spi_read()) );
    spi_cs_set( 1 );
    
//    uint32_t time_elapsed = timer_stop()/16;
//    idebug("SDW", time_elapsed);
    return( NO_ERROR );
}


