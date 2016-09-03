#include "FAT32.h"
#include "SDcard.h"
#include "per_proto.h"
#include "error_.h"

//************* external variables *************
Sector_t    first_data_sector,
            first_sector = 0,
            first_FAT_sector = 0,
            reserved_sector_count,
            sector_per_cluster;

Cluster_t   root_cluster,
            total_clusters;

uint16_t    bytes_per_sector;

//global flag to keep track of free cluster count updating in FSinfo sector
//uint8_t     free_cluster_count_updated;

uint8_t     FAT_sector_buffer[512];
Sector_t    FAT_sector_number = 0;

uint8_t     dir_sector_buffer[512];
Cluster_t   dir_cluster = 0;
Sector_t    dir_sector_first = 0, 
            dir_sector_number = 0;
uint16_t    dir_sector_offset = 0;
struct dir_entry_Structure *new_dir = NULL;

Cluster_t   data_cluster = 0;
Sector_t    data_first_sector = 0,
            data_sector = 0;

Cluster_t   next_free_cluster = 0;
/**********************************************************/

Cluster_t find_next_free_cluster ( Cluster_t startCluster );
static int find_empty_dir_entry ( void );

/**********************************************************/
int init_FAT32 ( void )
{
    struct BS_Structure *bpb; //mapping the buffer onto the structure
    struct MBRinfo_Structure *mbr;
    struct partitionInfo_Structure *partition;
    Sector_t    dataSectors;
    uint8_t     buffer[512];
    
    {
        int err_state = 0;
        if ( (err_state = init_SDcard()) < 0 )
        {
            UART_write_string( UARTm1, "SD card initialization failed\n" );
            UART_write_string( UARTm1, (err_state == TIMEOUT_ERROR_INIT_1 ? "SD card not found\n" : 
                                        err_state == TIMEOUT_ERROR_INIT_2 ? "Timeout 2\n" : "Unknown error\n") );
            error_process( __FUNCTION__ );
        }
    }
    
    SD_read_sector( first_sector, buffer );

    bpb = (struct BS_Structure *)buffer;
    
    if ( bpb->jumpBoot[0] != 0xE9 && bpb->jumpBoot[0] != 0xEB )   //check if it is boot sector
    {
        mbr = (struct MBRinfo_Structure *)buffer;       //if it is not boot sector, it must be MBR
        
        if ( mbr->signature != 0xaa55 ) 
            return( -1 );       //if it is not even MBR then it's not FAT32

        partition = (struct partitionInfo_Structure *)(&mbr->partitionData[0]);//first partition
        
        first_sector = partition->firstSector; //the unused sectors, hidden to the FAT
        
        SD_read_sector( first_sector, buffer );//read the bpb sector

        if(bpb->jumpBoot[0]!=0xE9 && bpb->jumpBoot[0]!=0xEB) 
            return( -1 ); 
    }

    bytes_per_sector = bpb->bytesPerSector;
    sector_per_cluster = bpb->sectorPerCluster;
    reserved_sector_count = bpb->reservedSectorCount;
    root_cluster = bpb->rootCluster;
    first_FAT_sector = first_sector + reserved_sector_count;
    first_data_sector = first_FAT_sector + (bpb->numberofFATs * bpb->FATsize_F32);
    
    dataSectors = bpb->totalSectors_F32
                  - reserved_sector_count
                  - (bpb->numberofFATs * bpb->FATsize_F32);
    total_clusters = dataSectors / sector_per_cluster;
#ifdef QDEBUG
UART_write_string( "/-----------------------------------------------------/\n" );
UART_write_string( "\tBoot info:\n" );
UART_write_string( "Total sectors: %ld\n", bpb->totalSectors_F32 );
UART_write_string( "Hidden sectors: %ld\n", bpb->hiddenSectors );
UART_write_string( "First sector of partition: %ld\n", first_sector );
UART_write_string( "Reserved sectors amount: %ld\n", reserved_sector_count );
UART_write_string( "First data sector: %ld\n", first_data_sector );
UART_write_string( "Bytes per sector: %ld\n", bytes_per_sector );
UART_write_string( "Sector per cluster: %ld\n", sector_per_cluster );
UART_write_string( "Root cluster: %ld\n", root_cluster );
UART_write_string( "Total clusters: %ld\n", total_clusters ); 
UART_write_string( "Data sector amount: %ld\n", dataSectors );
UART_write_string( "Number of FAT: %ld\n", bpb->numberofFATs ); 
UART_write_string( "FAT size: %ld\n", bpb->FATsize_F32 );
UART_write_string( "FSInfo sector: %ld\n", bpb->FSinfo );
UART_write_string( "/-----------------------------------------------------/\n" );
#endif /* QDEBUG */
    next_free_cluster = find_next_free_cluster( root_cluster );
    find_empty_dir_entry();
#ifdef QDEBUG
UART_write_string( "/-----------------------------------------------------/\n" );
UART_write_string( "\tWork info:\n" );
UART_write_string( "FAT sector: %ld\n", FAT_sector_number );
UART_write_string( "Dir cluster: %ld\n", dir_cluster );
UART_write_string( "Dir sector: %ld\n", dir_sector_number );
UART_write_string( "Dir sector offset: %ld\n", dir_sector_offset );
UART_write_string( "Next free cluster: %ld\n", next_free_cluster );
UART_write_string( "/-----------------------------------------------------/\n" );
#endif /* QDEBUG */
//    if ( getSetFreeCluster( TOTAL_FREE, GET, 0 ) > total_clusters )  //check if FSinfo free clusters count is valid
//        free_cluster_count_updated = 0;
//    else
//        free_cluster_count_updated = 1;

    return( 0 );
}
#ifdef QDEBUG
void show_content_of_FAT( uint16_t entries_amount )
{
    uint8_t     i = 0;
    uint32_t    *value = 0;

    for ( i = 0; i < 128; i++ )
    {
        if ( entries_amount-- == 0 )
            return;
        value = (uint32_t *) &FAT_sector_buffer[i*4];
        UART_write_string( "FAT: 0x08%x\n", *value );
    }  
    UART_write_string("/-------------------------/\n");
}
#endif
inline void read_FAT_sector_with_cluster ( Cluster_t current_cluster )
{
    FAT_sector_number = current_cluster / (bytes_per_sector/4);
    
    SD_read_sector( first_FAT_sector + FAT_sector_number, FAT_sector_buffer );
}

Cluster_t get_next_cluster_FAT ( Cluster_t current_cluster )
{
    uint16_t    FAT_sector_offset;
    //get the offset address in that sector number
    FAT_sector_offset = (uint16_t) (current_cluster * 4LL % bytes_per_sector);

    read_FAT_sector_with_cluster( current_cluster );
    //get the cluster address from the buffer
    return( (*(Cluster_t *)&FAT_sector_buffer[FAT_sector_offset]) & 0x0fffffff );
}

int set_next_cluster_FAT_cache ( Cluster_t current_cluster, Cluster_t cluster_entry )
{
    uint16_t    FAT_sector_offset;
    //get the offset address in that sector number
    FAT_sector_offset = (uint16_t) (current_cluster * 4LL % bytes_per_sector);

    if ( current_cluster > ((FAT_sector_number + 1) * 128 - 1) )
    {
        SD_write_sector( first_FAT_sector + FAT_sector_number, FAT_sector_buffer );
        FAT_sector_number++;
        memset( FAT_sector_buffer, 0, sizeof( FAT_sector_buffer ) );
    }
    
    *(Cluster_t *)(&FAT_sector_buffer[FAT_sector_offset]) = cluster_entry;   //for setting new value in cluster entry in FAT
    
    return( 0 );
}

int set_next_cluster_FAT ( Cluster_t current_cluster, Cluster_t cluster_entry )
{
    uint16_t    FAT_sector_offset;
    
    //get the offset address in that sector number
    FAT_sector_offset = (uint16_t) (current_cluster * 4 % bytes_per_sector);
    
    read_FAT_sector_with_cluster( current_cluster );

    *(Cluster_t *)(&FAT_sector_buffer[FAT_sector_offset]) = cluster_entry;   //for setting new value in cluster entry in FAT

    SD_write_sector( first_FAT_sector + FAT_sector_number, FAT_sector_buffer );
    
    return( 0 );
}

Cluster_t find_next_free_cluster ( Cluster_t startCluster )
{
    uint32_t    *value; 
    Cluster_t   cluster;
    uint8_t     i;

    startCluster -= (startCluster % (bytes_per_sector/4));   //to start with the first file in a FAT sector
    for ( cluster = startCluster; cluster < total_clusters; cluster += 128 )
    {
        read_FAT_sector_with_cluster( cluster );
        
        for ( i = 0; i < 128; i++ )
        {
            value = (uint32_t *) &FAT_sector_buffer[i*4];
            if ( ((*value) & 0x0fffffff) == 0 )
                return( cluster+i );
        }
    }
    return( 0 );
}

inline Sector_t get_first_sector_of_cluster ( Cluster_t cluster )
{
    return( ((cluster - 2) * sector_per_cluster) + first_data_sector );
}

static int find_empty_dir_entry ( void )
{
    struct dir_entry_Structure *dir;
    
    dir_cluster = root_cluster;
    while ( 1 )
    {
        dir_sector_first = get_first_sector_of_cluster( dir_cluster );
        for ( dir_sector_number = 0; dir_sector_number < sector_per_cluster; dir_sector_number++ )
        {
            SD_read_sector( dir_sector_first + dir_sector_number, dir_sector_buffer );
            for ( dir_sector_offset = 0; dir_sector_offset < bytes_per_sector; dir_sector_offset += DIR_ENTRY_SIZE )
            {
                dir = (struct dir_entry_Structure *) &dir_sector_buffer[dir_sector_offset];
                
                if ( ((dir->name[0] == DELETED) || (dir->name[0] == EMPTY)) && !(dir->attrib & ATTR_VOLUME_ID) )
                {
                    return( 0 );
                }
            }
        }

        if ( (dir_cluster = get_next_cluster_FAT( dir_cluster )) > 0x0ffffff6 )
        {
            UART_write_string( UARTm1, "Found end of root dir cluster\n" );
            return( -1 );
        }
        if ( dir_cluster == 0 ) 
        {
            UART_write_string( UARTm1, "Error in getting cluster\n" );  
            return( -1 );
        }
    }
    return( -1 );
}

// Called as task
int create_new_file ( char *filename )
{
    new_dir = (struct dir_entry_Structure *) &dir_sector_buffer[dir_sector_offset];
    
    dir_sector_offset += DIR_ENTRY_SIZE;
    
    data_cluster = next_free_cluster++;
    set_next_cluster_FAT_cache( data_cluster, EOC );   //last cluster of the file, marked EOF
    data_first_sector = get_first_sector_of_cluster( data_cluster );
    data_sector = 0;

    memcpy( new_dir->name, filename, FAT_FILENAME_LENGTH );
    
    new_dir->firstClusterHI = (data_cluster >> 16) & 0xffff;
    new_dir->firstClusterLO = data_cluster & 0xffff;
    new_dir->fileSize = 0;
    new_dir->attrib = ATTR_ARCHIVE;
    
    return( NO_ERROR_FAT );
}
// Called as task
int write_data_buffer ( uint8_t *buffer, uint16_t size )
{
    SD_write_sector( data_first_sector + data_sector, buffer );
    new_dir->fileSize += size;
    
    if ( ++data_sector == sector_per_cluster )
    {
        set_next_cluster_FAT_cache( data_cluster, next_free_cluster );
        data_cluster = next_free_cluster++;
        set_next_cluster_FAT_cache( data_cluster, EOC );
        data_first_sector = get_first_sector_of_cluster( data_cluster );
        data_sector = 0;
    }
    
    return( 0 );
}
// Called as task
static int write_FAT_buffer ( void )
{
//    debug( "FAT writing" );
    SD_write_sector( first_FAT_sector + FAT_sector_number, FAT_sector_buffer );
    return( 0 );
}
// Called as task
static int write_dir_buffer ( void )
{
//    debug( "Dir writing" );
    SD_write_sector( dir_sector_first + dir_sector_number, dir_sector_buffer );
    return( 0 );
}
// Called as task
int save_current_file ( void )
{
//    debug( "Saving file" );
    write_dir_buffer();
    write_FAT_buffer();
    if ( dir_sector_offset == bytes_per_sector )
    {
        if ( ++dir_sector_number == sector_per_cluster )
        {
            // Don`t change order of operations!!
            // Cause all readings are cached and programm needs last reading
            set_next_cluster_FAT( dir_cluster, next_free_cluster );
            dir_cluster = next_free_cluster++;
            set_next_cluster_FAT( dir_cluster, EOC );
            dir_sector_first = get_first_sector_of_cluster( dir_cluster );
            dir_sector_number = 0;
        }
        dir_sector_offset = 0;
        memset( dir_sector_buffer, 0, sizeof( dir_sector_buffer ) );
    }
    return( 0 );
}
/* 
int get_file_dir_entry_from_root ( char *filename, FileDescriptor_t *out_fd )
{
    Cluster_t   cluster = root_cluster;
    Sector_t    sector,
                first_cluster_sector;
    uint16_t    i = 0, j = 0;
    uint8_t     buffer[512];
    struct dir_entry_Structure *dir;
    
    while ( 1 )
    {
        first_cluster_sector = get_first_sector_of_cluster( cluster );
        for ( sector = 0; sector < sector_per_cluster; sector++ )
        {
            idebug( "Searching file dir entry", (first_cluster_sector + sector) );
            read_sector( first_cluster_sector + sector, buffer );
            for ( i = 0; i < bytes_per_sector; i += DIR_ENTRY_SIZE )
            {
                dir = (struct dir_entry_Structure *) &buffer[i];
                
                fdebug( "Found", dir->name[0] );
                
                if ( dir->name[0] == EMPTY ) //indicates end of the file list of the directory
                {
                    debug("No directory entries else");
                    return( NO_FILE_ERROR );   
                }
                if ( (dir->name[0] != DELETED) && (dir->attrib != ATTR_LONG_NAME) )
                {
                    for ( j = 0; j < 11; j++ )
                        if ( dir->name[j] != filename[j] ) 
                            break;

                    if ( j == 11 )  // Filename found
                    {
                        Cluster_t   cluster_count = 0;
                        memcpy( &out_fd->dir_entry, dir, DIR_ENTRY_SIZE );
                        out_fd->dir_entry_sector = first_cluster_sector + sector;
                        out_fd->dir_entry_sector_offset = i;
                        
                        cluster = ((uint32_t)dir->firstClusterHI) << 16 | dir->firstClusterLO;
                        
                        while ( 1 )
                        {
                            Cluster_t next_cluster = get_next_cluster_FAT( cluster );
                            if ( next_cluster == EOF ) 
                                break;
                            cluster = next_cluster;
                            cluster_count++;
                        }

                        out_fd->last_data_cluster = cluster;
                        //last sector number of the last cluster of the file
                        out_fd->last_data_sector_in_cluster = 
                            (dir->fileSize - (cluster_count * sector_per_cluster * bytes_per_sector)) / bytes_per_sector;
                        
                        return( NO_ERROR_FAT );
                    }
                }
            }
        }

        if ( (cluster = get_next_cluster_FAT( cluster )) > 0x0ffffff6 )
        {
            return( NO_FILE_ERROR );
        }
        if ( cluster == 0 ) 
        {
            debug( "Error in getting cluster" );  
            return( CLUSTER_ERROR );
        }
    }
    return( NO_ERROR_FAT );    // can`t be here!
}

int write_to_file ( char *in_buffer, FileDescriptor_t *cur_file )
{
    uint32_t    writed_count = 0;
    Cluster_t   cluster = cur_file->last_data_cluster;
    Sector_t    frst_cluster_sector = get_first_sector_of_cluster( cluster );
    uint8_t     buffer[512];
    uint16_t    sector_offset = cur_file->dir_entry.fileSize % bytes_per_sector;
    
    read_sector( cur_file->last_data_sector_in_cluster + frst_cluster_sector, buffer );
    
    while ( in_buffer[writed_count] != '\0' )
    {
        buffer[sector_offset++] = in_buffer[writed_count];
        
        if ( sector_offset == bytes_per_sector )
        {
            debug( "New sector" );
            SD_write_sector( cur_file->last_data_sector_in_cluster + frst_cluster_sector, buffer );
            sector_offset = 0;
            cur_file->last_data_sector_in_cluster++;
            if ( cur_file->last_data_sector_in_cluster == sector_per_cluster )
            {
                debug( "New cluster" );
                cur_file->last_data_sector_in_cluster = 0;
                Cluster_t free_cluster = find_next_free_cluster( cluster );
                set_next_cluster_FAT( cluster, free_cluster );
                cluster = free_cluster;
                set_next_cluster_FAT( cluster, EOF );
                cur_file->last_data_cluster = cluster;
                frst_cluster_sector = get_first_sector_of_cluster( cluster );
            }
            read_sector( cur_file->last_data_sector_in_cluster + frst_cluster_sector );
        }
        
        writed_count++;
        cur_file->dir_entry.fileSize++;
    }

    SD_write_sector( cur_file->last_data_sector_in_cluster + frst_cluster_sector, buffer );
    
    //Set size of file
    read_sector( cur_file->dir_entry_sector );
    
    struct dir_entry_Structure *dir = (struct dir_entry_Structure *) &buffer[cur_file->dir_entry_sector_offset];
    dir->fileSize = cur_file->dir_entry.fileSize;
    
    SD_write_sector( cur_file->dir_entry_sector, buffer );
    
    // Don`t forget write back filesize
    return( writed_count );
}

int write_sector_to_file ( char in_buffer[512], FileDescriptor_t *cur_file )
{
    uint32_t    writed_count = 0;
    Cluster_t   cluster = cur_file->last_data_cluster;
    Sector_t    frst_cluster_sector = get_first_sector_of_cluster( cluster );
    uint8_t     buffer[512];
    
    read_sector( cur_file->last_data_sector_in_cluster + frst_cluster_sector );
    
    memcpy( buffer, in_buffer, 512 );

    SD_write_sector( cur_file->last_data_sector_in_cluster + frst_cluster_sector );
    
    cur_file->last_data_sector_in_cluster++;
    
    if ( cur_file->last_data_sector_in_cluster == sector_per_cluster )
    {
        debug( "New cluster" );
        cur_file->last_data_sector_in_cluster = 0;
        Cluster_t free_cluster = find_next_free_cluster( cluster );
        set_next_cluster_FAT( cluster, free_cluster );
        cluster = free_cluster;
        set_next_cluster_FAT( cluster, EOF );
        cur_file->last_data_cluster = cluster;
        frst_cluster_sector = get_first_sector_of_cluster( cluster );
    }
            
    cur_file->dir_entry.fileSize += 512;
    writed_count += 512;
    //Set size of file
    read_sector( cur_file->dir_entry_sector );
    
    struct dir_entry_Structure *dir = (struct dir_entry_Structure *) &buffer[cur_file->dir_entry_sector_offset];
    dir->fileSize = cur_file->dir_entry.fileSize;
    
    SD_write_sector( cur_file->dir_entry_sector );
    
    // Don`t forget write back filesize
    return( writed_count );
}
*/

/*
Cluster_t getSetFreeCluster( uint8_t totOrNext, uint8_t get_set, Cluster_t FSEntry )
{
    uint8_t buffer[512];
    struct FSInfo_Structure *FS = (struct FSInfo_Structure *)buffer;
    
    read_sector( first_sector + 1 );
  
    if ( (FS->leadSignature != 0x41615252) || (FS->structureSignature != 0x61417272) || (FS->trailSignature !=0xaa550000) )
    {
        debug( "Incorrect FSInfo" );
        return( 0xffffffff );
    }

    if ( get_set == GET )
    {
        if ( totOrNext == TOTAL_FREE )
            return( FS->freeClusterCount );
        else // when totOrNext = NEXT_FREE
            return( FS->nextFreeCluster );
    }
    else
    {
        if ( totOrNext == TOTAL_FREE )
            FS->freeClusterCount = FSEntry;
        else // when totOrNext = NEXT_FREE
            FS->nextFreeCluster = FSEntry;

        SD_write_sector( first_sector + 1 );	//update FSinfo
    }
    
    return( 0xffffffff );
}

void freeMemoryUpdate ( uint8_t flag, uint32_t size_bytes )
{
    Cluster_t freeClusters;
    //convert file size into number of clusters occupied
    if((size_bytes % bytes_per_sector) == 0)
        size_bytes = size_bytes / bytes_per_sector;
    else 
        size_bytes = (size_bytes / bytes_per_sector) +1;
    
    if((size_bytes % sector_per_cluster) == 0) 
        size_bytes = size_bytes / sector_per_cluster;
    else 
        size_bytes = (size_bytes / sector_per_cluster) +1;

    if ( free_cluster_count_updated )
    {
        freeClusters = getSetFreeCluster( TOTAL_FREE, GET, 0 );
        if ( flag == ADD )
            freeClusters = freeClusters + size_bytes;
        else  //when flag = REMOVE
            freeClusters = freeClusters - size_bytes;
        getSetFreeCluster( TOTAL_FREE, SET, freeClusters );
    }
}
*/

inline void displayMemory ( uint8_t flag, uint32_t memory )
{
    uint8_t memoryString[] = "              Bytes\n", //20 character long string for memory display
            i = 0;
    
    for ( i = 12; i > 0; i-- ) //converting freeMemory into ASCII string
    {
        if ( i == 5 || i == 9 ) 
        {
            memoryString[i-1] = ',';  
            i--;
        }
        memoryString[i-1] = (memory % 10) | 0x30;
        memory /= 10;
        if ( memory == 0 ) 
            break;
    }
    if(flag == HIGH)
        memoryString[13] = 'K';
    UART_write_string( UARTm1, (char *)memoryString );
}
#ifdef QDEBUG
int get_file_list_root( void )
{
    Cluster_t   cluster = root_cluster;
    Sector_t    sector,
                first_cluster_sector;
    uint16_t    i;
    uint8_t     j,
                buffer[512];
    struct dir_entry_Structure *dir;
    UART_write_string( UARTm1, "\nFile List:\n" );
    while ( 1 )
    {
        first_cluster_sector = get_first_sector_of_cluster( cluster );
        UART_write_string( UARTm1, "Reading sector to find entry: %ld\n", first_cluster_sector );
        for ( sector = 0; sector < sector_per_cluster; sector++ )
        {
            SD_read_sector( first_cluster_sector + sector, buffer );            
            for ( i = 0; i < bytes_per_sector; i += DIR_ENTRY_SIZE )
            {
                dir = (struct dir_entry_Structure *) &buffer[i];

                if ( dir->name[0] == EMPTY ) //indicates end of the file list of the directory
                {
                    UART_write_string( UARTm1, "No directory entries else\n" );
                    return( NULL );   
                }
                if ( (dir->name[0] != DELETED) && (dir->attrib != ATTR_LONG_NAME) )
                {
                    UART_write_string( UARTm1, "   " );
                    for ( j = 0; j < 11; j++ )
                    {
                        if ( j == 8 ) 
                            UART_write_byte( UARTm1, ' ' );
                        UART_write_byte( dir->name[j] );
                    }
                    UART_write_string( UARTm1, "   " );
                    if ( (dir->attrib != ATTR_DIRECTORY) && (dir->attrib != ATTR_VOLUME_ID) )
                    {
                        UART_write_string( UARTm1, "FILE   \n" );
                        displayMemory( LOW, dir->fileSize );
                    }
                    else
                    {
                        UART_write_string( (dir->attrib == ATTR_DIRECTORY) ? "DIR" : "ROOT" );
                    }
                    Cluster_t current_dE_cluster = (uint32_t)dir->firstClusterHI << 16 | dir->firstClusterLO;
                    UART_write_string( UARTm1, "Cluster: 0x%08x\n", current_dE_cluster );
                    UART_write_string( UARTm1, "Attributes: 0x%08x\n", dir->attrib );
                }
                else
                {
                    if ( dir->attrib == ATTR_LONG_NAME )
                    {
                        UART_write_string( UARTm1, "Bad attribute - long name\n" );
                    }
                    else
                    {
                        UART_write_string( UARTm1, "Dir entry is deleted\n" );
                    }
                }
            }
        }

        cluster = get_next_cluster_FAT( cluster );

        if ( cluster > 0x0ffffff6 )
        {
            return( NULL );
        }
        if ( cluster == 0 ) 
        {
            UART_write_string( UARTm1, "Error in getting cluster\n" );  
            return( NULL );
        }
    }
    return( NULL );
}
#endif
