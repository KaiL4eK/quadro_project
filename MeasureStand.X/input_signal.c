#include "core.h"

uint16_t    *tableSignal = NULL;
uint16_t    curPoint = 0,
            numPoints = 0,
            curSignalZeroLevel = 0;

#define M_PI    acos(-1.0)

int init_sinus ( uint16_t halfAmpl, uint16_t period_time_ms, uint16_t offset )
{
    if ( tableSignal != NULL )
    {
        free( tableSignal );
        tableSignal = NULL;
    }
        
    numPoints = 400L*period_time_ms/1000L;
    if ( numPoints < 4 || offset < halfAmpl )
    {
        return( -1 );
    }
 
    curSignalZeroLevel = offset;
    
    tableSignal = calloc( numPoints, sizeof( *tableSignal ) );
    if ( tableSignal == NULL )
    {
        return( -1 );
    }
    
    for ( int i = 0; i < numPoints; i++ )
    {
        tableSignal[i] = halfAmpl * sinf( 2*M_PI*(i*1.0/numPoints) ) + offset;
    }
    
    return( 0 );
}

int init_square ( uint16_t lowLevel, uint16_t highLevel, uint16_t halfTime )
{
    if ( tableSignal != NULL )
    {
        free( tableSignal );
        tableSignal = NULL;
    }
    
    numPoints = 400L*(2L*halfTime)/1000L;
    if ( numPoints < 4 || lowLevel > highLevel )
    {
        return( -1 );
    }
    
    curSignalZeroLevel = lowLevel;
    
    tableSignal = calloc( numPoints, sizeof( *tableSignal ) );
    if ( tableSignal == NULL )
    {
        return( -1 );
    }
    
    for ( int i = 0; i < numPoints/2; i++ )
    {
        tableSignal[i] = lowLevel;
        tableSignal[i + numPoints/2] = highLevel;
    }
    
    return( 0 );
}

uint16_t get_next_signal_value ( void )
{
    if ( ++curPoint == numPoints )
    {
        curPoint = 0;
    }
    if ( tableSignal != NULL )
    {
        return( tableSignal[curPoint] );
    }
    return( 0 );
    
}

uint16_t get_signal_zero_lvl ( void )
{
    curPoint = 0;
    return( curSignalZeroLevel );
}
