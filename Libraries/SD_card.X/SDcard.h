#ifndef SD_CARD_H_
#define	SD_CARD_H_

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>
#include "common.h"

typedef enum
{
    GO_IDLE_STATE           = 0,
    SEND_OP_COND            = 1,
    SEND_IF_COND			= 8,
    SEND_CSD                = 9,
    STOP_TRANSMISSION       = 12,
    SEND_STATUS             = 13,
    SET_BLOCK_LEN           = 16,
    READ_SINGLE_BLOCK       = 17,
    READ_MULTIPLE_BLOCKS    = 18,
    WRITE_SINGLE_BLOCK      = 24,
    WRITE_MULTIPLE_BLOCKS   = 25,
    ERASE_BLOCK_START_ADDR  = 32,
    ERASE_BLOCK_END_ADDR    = 33,
    ERASE_SELECTED_BLOCKS   = 38,
    SD_SEND_OP_COND			= 41,  //ACMD
    APP_CMD					= 55,
    READ_OCR				= 58,
    CRC_ON_OFF              = 59
}SD_command_t;

#define CRC_ON                  1
#define CRC_OFF                 0

#define TIMEOUT_ERROR_INIT_1                ( -1 )
#define TIMEOUT_ERROR_INIT_2                ( -2 )
#define TIMEOUT_ERROR_INIT_3                ( -3 )
#define TIMEOUT_ERROR_READ                  ( -4 )
#define TIMEOUT_ERROR_WRITE                 ( -5 )
#define TIMEOUT_ERROR_WRITE_NOT_ACCEPTED    ( -6 )
#define NO_ERROR                            ( 0 )

#define WRITING_ACCEPTED    0x05

int SD_initialize ( uint8_t uart_module );
int SD_read_sector ( uint32_t start_block, uint8_t *buffer );
int SD_write_sector ( uint32_t start_block, uint8_t *buffer );

#endif	/* SD_CARD_H_ */

