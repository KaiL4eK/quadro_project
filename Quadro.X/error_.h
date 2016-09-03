#ifndef ERROR_H_
#define	ERROR_H_

#include <stdlib.h>
#include <xc.h>

/*** error.c ***/

#define ERR_LIGHT           _LATA4
#define ERR_LIGHT_NO_ERR    1
#define ERR_LIGHT_ERR       0
#define INIT_ERR_L  { _TRISA4 = 0; ERR_LIGHT = ERR_LIGHT_NO_ERR; }

void error_process ( const char *err_str );

#endif	/* ERROR_H_ */

