#ifndef INPUT_SIGNAL_H_
#define	INPUT_SIGNAL_H_

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>

int init_sinus ( uint16_t halfAmpl, uint16_t period_time_ms, uint16_t offset );
uint16_t get_next_signal_value ( void );
uint16_t get_signal_zero_lvl ( void );



#endif	/* INPUT_SIGNAL_H_ */

