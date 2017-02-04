#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include "file_io.h"
#include "FAT32.h"

static int convert_filename ( char *filename );

typedef enum
{
    OPEN_FILE,
    WRITE_BUFFER,
    CLOSE_FILE
}TaskType_t;

typedef struct task_
{    
    TaskType_t      type;
    char            filename[12];
    uint8_t         i_buffer;
    uint16_t        buffer_sz;
    struct task_    *next_task;
} Task_t;

#define BUFFERS_AMOUNT  5

static uint8_t          uart_debug                          = -1;
static uint8_t          data_buffer[BUFFERS_AMOUNT][512],
                        busy_buffers[BUFFERS_AMOUNT],
                        i_active_buffer                     = 0,
                        file_opened                         = 0;
static bool             initialized                         = 0;

static uint16_t         data_offset                         = 0;

static Task_t           *tasks_list                         = NULL,
                        *last_task                          = NULL;

static const uint16_t   data_buffer_sz                      = sizeof( data_buffer[0] );

uint8_t file_get_buffers_loaded_count ( void )
{
    int i = 0;
    uint8_t count = 0;
    for ( i = 0; i < BUFFERS_AMOUNT; i++ )
        if ( busy_buffers[i] )
            count++;
    
    return count;
}

/*** FAT32 Dependent functions ***/

int file_io_initialize ( uint8_t uart_module )
{
    if ( fat32_initialize( uart_module ) != 0 ) {
        return 1;
    }
    
    uart_debug = uart_module;
    
    UART_write_string( uart_debug, "File I/O initialized\n" );
    
    initialized = 1;
    return 0;
}

static int open ( void )
{
    if ( file_opened )
        return( -1 );
    
    fat32_create_new_file( tasks_list->filename );
    file_opened = 1;
    
    return( 0 );
}

static int close ( void )
{
    if ( !file_opened )
        return( -1 );
    
    fat32_write_data_buffer( data_buffer[tasks_list->i_buffer], tasks_list->buffer_sz );
    busy_buffers[tasks_list->i_buffer] = 0;
    
    fat32_file_close();
    file_opened = 0;
    
    return( 0 );
}

/*** Tasks ***/

static int add_task ( TaskType_t task, uint8_t i_buffer, char *filename )
{
    Task_t *new_task = calloc( 1, sizeof( *new_task ) );
    if ( tasks_list == NULL )
        tasks_list = new_task;
    else
        last_task->next_task = new_task;
    
    last_task = new_task;
    new_task->next_task = NULL;
    new_task->type = task;
    
    switch ( task )
    {
        case OPEN_FILE:
            memcpy( new_task->filename, filename, 12 );
            break;
            
        case CLOSE_FILE:
            new_task->buffer_sz = data_offset;
        case WRITE_BUFFER:
            new_task->i_buffer  = i_buffer;
            break;
    }
    return( 0 );
}

// Asynchronous function
int file_process_tasks ( void )
{
    if ( tasks_list == NULL || !initialized )
        return( 0 );

    switch ( tasks_list->type )
    {
        case OPEN_FILE:
            open();
            break;
        case CLOSE_FILE:
            close();
            break;
        case WRITE_BUFFER:
            fat32_write_data_buffer( data_buffer[tasks_list->i_buffer], data_buffer_sz );
            busy_buffers[tasks_list->i_buffer] = 0;
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

static void set_next_buffer( void )
{
    static uint8_t out_counter = 0;
    
    busy_buffers[i_active_buffer++] = 1;
    i_active_buffer = (i_active_buffer == BUFFERS_AMOUNT) ? 0 : i_active_buffer;
    while ( busy_buffers[i_active_buffer] )
    {
        if ( out_counter++ == 255 ) 
        {
            file_close();
//            while ( 1 ) { Nop(); }
        }
    }
    data_offset = 0;
    memset( data_buffer[i_active_buffer], 0, sizeof( data_buffer[i_active_buffer] ) );
}

/*** User functions ***/

int file_open ( char *name_format )
{
    if ( !initialized )
        return( -1 );
    
    char filename[12];
    sprintf( filename, name_format, fat32_get_file_index() );
    
    data_offset = 0;
    
    if ( convert_filename( filename ) < 0 )
    {
        UART_write_string( uart_debug, "Incorrect filename - file_open()\n" );
        return( -1 );
    }
    
    add_task( OPEN_FILE, 0, filename );

    return( 0 );
}

int file_close ( void )
{
    if ( !initialized )
        return( -1 );
    
    add_task( CLOSE_FILE, i_active_buffer, NULL );
    set_next_buffer();
    return( 0 );
}


int file_write( uint8_t *buffer, uint16_t buffer_length )
{
    if ( !initialized || !file_opened )
        return( -1 );
    
    memcpy( &data_buffer[i_active_buffer][data_offset], buffer, buffer_length );
    data_offset += buffer_length;
    
    if ( data_offset >= sizeof( data_buffer[i_active_buffer] ) )
    {
        add_task( WRITE_BUFFER, i_active_buffer, NULL );
        set_next_buffer();
    }
    
    return( 0 );
}

static int convert_filename ( char *filename )
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
