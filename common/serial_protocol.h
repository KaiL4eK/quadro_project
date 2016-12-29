#ifndef SERIAL_PROTOCOL_H_
#define SERIAL_PROTOCOL_H_

#define COMMAND_FRAME_SIZE      1    // bytes
#define DATA_FULL_FRAME_SIZE	14
#define DATA_QUADRO_FRAME_SIZE  10
#define RESPONSE_FRAME_SIZE     1
#define PARAMETER_FRAME_SIZE    2

#define COMMAND_PREFIX          '*'
#define CMD_CONNECT_CODE        'c'
#define CMD_DISCONNECT_CODE     'd'
#define CMD_DATA_START_CODE     's'
#define CMD_DATA_STOP_CODE      'p'
#define CMD_MOTOR_STOP          't'
#define CMD_MOTOR_START         'q'

#define DATA_PREFIX             '$'

#define RESPONSE_PREFIX         '#'
#define RESP_NOERROR            '0'
#define RESP_NOCONNECT          '1'
#define RESP_ENDDATA            '2'

#define PARAMETER_PREFIX        '~'
#define PARAM_MOTOR_POWER       'o'

#endif // SERIAL_PROTOCOL_H_
