#include "file_io.h"
#include "FAT32.h"
#include "per_proto.h"

#define PROCESSING_LIGHT    _LATA5
#define PROCESSING_TRIS     _TRISA5

typedef enum
{
    OPEN_FILE,
    WRITE_BUFFER,
    CLOSE_FILE
}TaskType_t;

typedef struct task_ Task_t;

struct task_
{
    TaskType_t  type;
    char        filename[12];
    uint8_t     buffer_num;
    uint16_t    buffer_sz;
    Task_t      *next_task;
};

#define BUFFERS_AMOUNT  5

static uint8_t      data_buffer[BUFFERS_AMOUNT][512],
                    current_buffer = 0,
                    busy_buffers[BUFFERS_AMOUNT],
                    file_opened = 0,
                    init_flag = 0;

static uint16_t     data_offset = 0;

static Task_t       *tasks_list = NULL,
                    *last_task = NULL;

int init_sd_file_io ( void )
{
    spi_set_speed( FREQ_125K );
    if ( init_FAT32() < 0 )
    {
        debug("FAT32 not found!");
        error_process();
    }
    spi_set_speed( FREQ_16000K );
    PROCESSING_TRIS = 0;
    PROCESSING_LIGHT = 1;
    init_flag = 1;
    return( 0 );
}

inline int convert_filename ( char *filename );

inline static int open ( void )
{
    if ( file_opened )
        return( -1 );
    
    create_new_file( tasks_list->filename );
    file_opened = 1;
    
    return( 0 );
}

inline static int close ( void )
{
    if ( !file_opened )
        return( -1 );
    
    write_data_buffer( data_buffer[tasks_list->buffer_num], tasks_list->buffer_sz );
    busy_buffers[tasks_list->buffer_num] = 0;
    save_current_file();
    file_opened = 0;
    
    return( 0 );
}

int add_task ( TaskType_t task, uint8_t buffer_num, char *filename )
{
    Task_t *new_task = malloc( sizeof( *new_task ) );
    if ( tasks_list == NULL )
        tasks_list = new_task;
    else
        last_task->next_task = new_task;
    
    last_task = new_task;
    new_task->next_task = NULL;
    new_task->buffer_num = 0;
    new_task->buffer_sz = 0;
    new_task->type = task;
    
    switch ( task )
    {
        case OPEN_FILE:
            memcpy( new_task->filename, filename, 12 );
            break;
        case CLOSE_FILE:
        case WRITE_BUFFER:
            new_task->buffer_num = buffer_num;
            new_task->buffer_sz = data_offset;
            break;
    }
    return( 0 );
}

int file_process_tasks ( void )
{
    if ( tasks_list == NULL || !init_flag )
    {
        PROCESSING_LIGHT = 1;
        return( 0 );
    }
    PROCESSING_LIGHT = 0;
    switch ( tasks_list->type )
    {
        case OPEN_FILE:
            open();
            break;
        case CLOSE_FILE:
            close();
            break;
        case WRITE_BUFFER:
            write_data_buffer( data_buffer[tasks_list->buffer_num], tasks_list->buffer_sz );
            busy_buffers[tasks_list->buffer_num] = 0;
            break;
    }
    
    Task_t *tmp_task_ptr = tasks_list;
    
    if ( tasks_list == last_task )
        tasks_list = last_task = NULL;
    else
        tasks_list = tasks_list->next_task;

    free( tmp_task_ptr );
    return( 0 );
}

int file_open ( char *in_filename )
{
    if ( !init_flag )
        return( -1 );
    
    char filename[12];
    // Copy filename in correct array
    memset( filename, 0, sizeof( filename ) );
    
    data_offset = 0;
    
    uint8_t i = 0;    
    for ( i = 0; i < 12; i++ )
    {
        filename[i] = in_filename[i];
        if ( in_filename[i] == '\0' )
            break;
    }
    
    if ( convert_filename( filename ) < 0 )
    {
        debug( "Incorrect filename - file_open()" );
        return( -1 );
    }
    
    add_task( OPEN_FILE, 0, filename );

    return( 0 );
}

uint8_t out_counter = 0;

inline void set_next_buffer( void )
{
    busy_buffers[current_buffer++] = 1;
    current_buffer = current_buffer == BUFFERS_AMOUNT ? 0 : current_buffer;
    while ( busy_buffers[current_buffer] )
    {
        debug( "Buffers overlay" );
        if ( out_counter++ == 255 )
            error_process();
    }
    data_offset = 0;
    memset( data_buffer[current_buffer], 0, sizeof( data_buffer[current_buffer] ) );
}

int file_close ( void )
{
    if ( !init_flag )
        return( -1 );
    
    add_task( CLOSE_FILE, current_buffer, NULL );
    set_next_buffer();
    return( 0 );
}


int file_write( uint8_t *buffer, uint16_t buffer_length )
{
    if ( !init_flag )
        return( -1 );
    
    memcpy( &data_buffer[current_buffer][data_offset], buffer, buffer_length );
    data_offset += buffer_length;
    
    if ( data_offset >= sizeof( data_buffer[current_buffer] ) )
    {
        add_task( WRITE_BUFFER, current_buffer, NULL );
        set_next_buffer();
    }
    
    return( 0 );
}

inline int convert_filename ( char *filename )
{
    uint8_t fileNameFAT[11], 
            j = 0, k = 0;

    if ( filename[j] == '.' || filename[j] == 0 )
        return( -1 ); 
        
    while( filename[j] != '.' )
        if ( ++j > 8 ) 
            return( -1 );

    for ( k = 0; k < j; k++ ) //setting file name
        fileNameFAT[k] = filename[k];

    for ( k = j; k <= 7; k++ ) //filling file name trail with blanks
        fileNameFAT[k] = ' ';

    j++;
    for ( k = 8; k < 11; k++ ) //setting file extention
        fileNameFAT[k] = (filename[j] != 0) ? filename[j++] : ' ';

    for ( j = 0; j < 11; j++ ) //converting small letters to caps
        if ( (fileNameFAT[j] >= 0x61) && (fileNameFAT[j] <= 0x7a) )
            fileNameFAT[j] -= 0x20;

    memcpy( filename, fileNameFAT, sizeof( fileNameFAT ) );

    return( 0 );
}
