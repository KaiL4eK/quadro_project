#ifndef MOTOR_CONTROL_H_
#define	MOTOR_CONTROL_H_

#include "core.h"

void motors_init();
void set_motor_power( uint8_t nMotor, int32_t power );

void set_motors_started( uint8_t motor_nums );
void set_motors_stopped();

#define MOTOR_1 ( 1 << 0 )
#define MOTOR_2 ( 1 << 1 )
#define MOTOR_3 ( 1 << 2 )
#define MOTOR_4 ( 1 << 3 )
#define MOTORS_ALL ( MOTOR_1 | MOTOR_2 | MOTOR_3 | MOTOR_4 )

//#define USEC_2_PWM(x)   ((x)*4 - 1)     // ((FCY/1000000L)*(x)/PWM_PRESCALE - 1)

#define ESC_MAX_POWER   7599L // USEC_2_PWM(1900)
#define ESC_MIN_POWER   4799L // USEC_2_PWM(1200)
#define ESC_STOP_POWER  3599L // USEC_2_PWM(900)

#define INPUT_POWER_MAX 2800L // ESC_MAX_POWER - ESC_MIN_POWER
#define INPUT_POWER_MIN 0L

#endif	/* MOTOR_CONTROL_H_ */

