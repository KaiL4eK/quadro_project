#ifndef MOTOR_CONTROL_H_
#define	MOTOR_CONTROL_H_

#include "core.h"

void motor_init();
void set_motor_started( void );
void set_motor_stopped( void );
void set_motor_power( int32_t power );

#define INPUT_POWER_MAX 2800L // ESC_MAX_POWER - ESC_MIN_POWER
#define INPUT_POWER_MIN 0L

#endif	/* MOTOR_CONTROL_H_ */

