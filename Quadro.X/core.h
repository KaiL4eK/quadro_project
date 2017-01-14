#ifndef Q_CORE_H_
#define	Q_CORE_H_

#include "per_proto.h"
#include "error_.h"

#include "motor_control.h"

#define CONTROL_BYTE    0xff

#define FREQ_CONTROL_SYSTEM   400L

/********** DEFINES **********/
typedef int16_t euler_angle_degree_int_t;
typedef int16_t motor_power_t;
typedef int16_t error_value_t;

typedef struct
{
    euler_angle_degree_int_t    roll,           // Multiplyed by 100
                                pitch,
                                yaw;

    motor_power_t               motor_power[4];
}quadrotor_state_t;

typedef enum {
    
    NO_COMMAND,
    UNKNOWN_COMMAND,
    CONNECT,
    DISCONNECT,
    DATA_START,
    DATA_STOP,
    MOTOR_START,
    MOTOR_STOP,
    MOTOR_SET_POWER
    
}UART_commands_e;

typedef struct {
 
    UART_commands_e command;
    uint8_t         motorPower;
    // Not ready
}UART_frame_t;

/********** PID CONTROLLER **********/
// _rev - rate is reversed (divided by)
typedef struct {
    uint16_t        prop_rev,
                    diff,
                    integr_rev;
} PID_rates_t;

typedef struct {
    float           prop_rev,
                    diff,
                    integr_rev;
} PID_rates_float_t;

typedef struct {
    int16_t p, i, d;
} PID_parts_t;

PID_parts_t *PID_controller_get_pitch_parts( void );

void PID_controller_reset_integral_sums ( void );
int16_t PID_controller_generate_pitch_control( error_value_t error );
int16_t PID_controller_generate_roll_control( error_value_t error );

/********** COMMAND PROCESSOR **********/

void cmdProcessor_init ( UART_moduleNum_t module );
UART_frame_t *cmdProcessor_rcvFrame ( void );
void cmdProcessor_write_cmd ( UART_moduleNum_t module, uint8_t prefix, uint8_t code );

#endif	/* Q_CORE_H_ */

