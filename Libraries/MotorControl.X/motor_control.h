#ifndef MOTOR_CONTROL_H_
#define	MOTOR_CONTROL_H_

#include <per_proto.h>

/*
 * Modules utilized:
 *      PWM_GEN         1   RE0
 *      PWM_GEN         2   RE2
 *      PWM_GEN         3   RE4
 *      PWM_GEN         4   RE6
 */

#ifdef DSPIC_ENABLE_PLL
    #error "PLL not supported yet"
#endif

typedef int16_t motor_power_t;

typedef enum {
    MOTOR_1 = 0,
    MOTOR_2,
    MOTOR_3,
    MOTOR_4
} motor_num_t;

bool motor_control_is_armed( void );
void motor_control_init( void );
void motor_control_set_motor_power( motor_num_t nMotor, int16_t power );
void motor_control_set_motor_powers( int16_t powers[4] );

void motor_control_set_motor_started( motor_num_t nMotor );
void motor_control_set_motor_stopped( motor_num_t nMotor );
void motor_control_set_motors_started( void );
void motor_control_set_motors_stopped( void );

#define INPUT_POWER_MAX 3200L // ESC_MAX_POWER - ESC_MIN_POWER
#define INPUT_POWER_MIN 0L

#ifndef clip_value
    #define clip_value( val, min, max ) ((val) > max ? max : (val) < min ? min : (val))
#endif

#endif	/* MOTOR_CONTROL_H_ */

