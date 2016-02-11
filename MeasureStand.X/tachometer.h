#ifndef INPUT_CONTROL_H_
#define	INPUT_CONTROL_H_

#include "core.h"

void tacho_init ( void );
int16_t tacho_get_round_speed ( void );
void tacho_start_cmd ( void );
void tacho_stop_cmd ( void );

#endif	/* INPUT_CONTROL_H_ */

