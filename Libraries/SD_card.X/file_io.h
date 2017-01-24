#ifndef FILE_IO_H_
#define	FILE_IO_H_

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>
#include "common.h"

int file_open ( char *name_format );
int file_write( uint8_t *buffer, uint16_t buffer_length );
int file_close ( void );
int file_io_initialize ( uint8_t uart_module );
int file_process_tasks ( void );
uint8_t file_get_buffers_loaded_count ( void );

#endif	/* FILE_IO_H_ */

