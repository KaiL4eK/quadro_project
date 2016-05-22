#ifndef CORE_H_
#define	CORE_H_

#include "per_proto.h"

void tacho_init ( void );
int16_t tacho_get_round_speed ( void );
void tacho_start_cmd ( void );
void tacho_stop_cmd ( void );

int init_sinus ( uint16_t halfAmpl, uint16_t period_time_ms, uint16_t offset );
int init_square ( uint16_t lowLevel, uint16_t highLevel, uint16_t halfTime );
uint16_t get_next_signal_value ( void );
uint16_t get_signal_zero_lvl ( void );

#endif	/* CORE_H_ */

