#ifndef FILE_IO_H_
#define	FILE_IO_H_

#include <stdint.h>
#include <stdlib.h>
#include <xc.h>

int file_open ( char *in_filename );
int file_write( char *buffer, uint16_t buffer_length );
int file_close ( void );

int file_process_tasks ( void );

#endif	/* FILE_IO_H_ */

