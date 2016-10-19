#ifndef SERIAL_PROTOCOL_H_
#define SERIAL_PROTOCOL_H_

#define COMMAND_FRAME_SIZE          1    // bytes
#define DATA_STAND_FRAME_SIZE       10
#define RESPONSE_FRAME_SIZE         1
#define PARAMETER_FRAME_SIZE        (1*2 + 2*3)

#define COMMAND_PREFIX              '*'
#define CMD_CONNECT_CODE            'c'
#define CMD_DISCONNECT_CODE         'd'
#define CMD_MOTOR_STOP              't'
#define CMD_MOTOR_START             'q'

#define DATA_PREFIX                 '$'

#define RESPONSE_PREFIX             '#'
#define RESP_NOERROR                '0'
#define RESP_NOCONNECT              '1'
#define RESP_ENDDATA                '2'
#define RESP_UNINITIALIZED          '3'

#define PARAMETERS_PREFIX           '~'
/*
#define PARAM_MOTOR_POWER_START     's'
#define PARAM_MOTOR_POWER_END       'e'
#define PARAM_TIME_STEP             't'
#define PARAM_TIME_MEASURE_START    'f'
#define PARAM_TIME_MEASURE_STOP     'g'
*/

#define MEASURE_PERIOD_MS           10.0f

#endif // SERIAL_PROTOCOL_H_
