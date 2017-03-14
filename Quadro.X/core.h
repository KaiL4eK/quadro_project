#ifndef Q_CORE_H_
#define	Q_CORE_H_

#include <per_proto.h>

#include "MPU6050.h"
#include "motor_control.h"

#define CONTROL_BYTE    0xff

#define FREQ_CONTROL_SYSTEM   400L


/********** DEFINES **********/
typedef int16_t euler_angle_degree_int_t;
typedef int16_t motor_power_t;
typedef int16_t error_value_t;


typedef struct {
    float   acc_x,
            acc_y,
            acc_z,
            gyr_x,
            gyr_y,
            gyr_z;
}imu_filter_input_t;

typedef struct
{
    int16_t                     pitch_rate,
                                roll_rate,
                                yaw_rate;
    
    euler_angle_degree_int_t    roll,
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

/********** error.c **********/

#define ERR_LIGHT           _LATA4
#define ERR_LIGHT_NO_ERR    1
#define ERR_LIGHT_ERR       0
#define INIT_ERR_L  { _TRISA4 = 0; ERR_LIGHT = ERR_LIGHT_NO_ERR; }

void error_process_init ( uart_module_t uart_module );
void error_process ( const char *err_str );

/********** PID CONTROLLER **********/
// _rev - rate is reversed (divided by)
typedef struct {
    uint16_t        prop_rev,
                    diff,
                    integr_rev;
} PID_rates_t;

typedef struct {
    float           prop,
                    diff,
                    integr;
} PID_rates_float_t;

typedef struct {
    float p, i, d;
} PID_parts_t;

void control_system_timer_init( void );
void process_UART_frame( void );

void complementary_filter_set_angle_rate( float rate_a );
void complementary_filter_set_rotation_speed_rate( float rate_a );

void UART_debug_interface( uart_module_t uart );
void send_serial_data_full ( uart_module_t uart, uart_module_t debug, quadrotor_state_t *q_state );


void PID_controller_reset_integral_sums ( void );
int16_t PID_controller_generate_pitch_control( float error, float angle_speed );
int16_t PID_controller_generate_roll_control( float error, float angle_speed );
int16_t PID_controller_generate_yaw_control( float error );

/********** COMMAND PROCESSOR **********/

void cmdProcessor_init ( uart_module_t module );
UART_frame_t *cmdProcessor_rcvFrame ( void );
void cmdProcessor_write_cmd ( uart_module_t module, uint8_t prefix, uint8_t code );

int battery_charge_initialize ( void );
void battery_charge_read_value ( void );
uint16_t battery_charge_get_voltage_x10 ( void );
void battery_charge_set_filter_value ( float value );

#endif	/* Q_CORE_H_ */

