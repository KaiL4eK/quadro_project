#include "error.h"
#include "per_proto.h"

#define SET_ERR_L   { ERR_LIGHT = 0; }

void error_process ( void )
{
    SET_ERR_L;
    debug("Err");
    while ( 1 );
}
