#ifndef MOTOR_CONTROL_H_
#define	MOTOR_CONTROL_H_

#include "core.h"

void motors_init();
void set_motor1_power( int16_t power );
void set_motor2_power( int16_t power );
void set_motor3_power( int16_t power );
void set_motor4_power( int16_t power );

#define MOTOR_1 ( 1 << 0 )
#define MOTOR_2 ( 1 << 1 )
#define MOTOR_3 ( 1 << 2 )
#define MOTOR_4 ( 1 << 3 )
#define MOTORS_ALL ( MOTOR_1 | MOTOR_2 | MOTOR_3 | MOTOR_4 )


#define INPUT_POWER_MAX 10000L
#define INPUT_POWER_MIN 0L

void set_motors_started( uint8_t motor_nums );
void set_motors_stopped();

#endif	/* MOTOR_CONTROL_H_ */

