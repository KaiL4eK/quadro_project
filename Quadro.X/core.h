#ifndef Q_CORE_H_
#define	Q_CORE_H_

#include "per_proto.h"
#include "error_.h"

#define CONTROL_BYTE    0xff

#define clip_value( power, min, max ) (power > max ? max : power < min ? min : power)

#define FREQ_CONTROL_SYSTEM   400L

typedef struct
{
    int32_t     roll,           // Multiplyed by 100
                pitch,
                yaw;
    int16_t     motor1_power,
                motor2_power,
                motor3_power,
                motor4_power;
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

void cmdProcessor_init ( UART_moduleNum_t module );
UART_frame_t *cmdProcessor_rcvFrame ( void );
void cmdProcessor_write_cmd ( UART_moduleNum_t module, uint8_t prefix, uint8_t code );

#endif	/* Q_CORE_H_ */

