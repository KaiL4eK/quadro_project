#ifndef MOTOR_CONTROL_H_
#define	MOTOR_CONTROL_H_

#include "core.h"

typedef enum {
    MOTOR_1 = 0,
    MOTOR_2,
    MOTOR_3,
    MOTOR_4
} motor_num_t;

void motor_control_init( void );
void motor_control_set_motor_power( motor_num_t nMotor, int16_t power );
void motor_control_set_motor_powers( int16_t powers[4] );

void motor_control_set_motor_started( motor_num_t nMotor );
void motor_control_set_motor_stopped( motor_num_t nMotor );
void motor_control_set_motors_started( void );
void motor_control_set_motors_stopped( void );

#define INPUT_POWER_MAX 2800L // ESC_MAX_POWER - ESC_MIN_POWER
#define INPUT_POWER_MIN 0L

#endif	/* MOTOR_CONTROL_H_ */

