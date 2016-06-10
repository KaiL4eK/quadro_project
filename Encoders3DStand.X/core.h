#ifndef Q_CORE_H_
#define	Q_CORE_H_

#include "per_proto.h"

typedef enum {
    
    NO_COMMAND,
    UNKNOWN_COMMAND,
    CONNECT,
    DATA_START,
    DATA_STOP
    
}UART_commands_e;

#define CMD_PREFIX  '*'
#define CMD_CONNECT_CODE        127
#define CMD_DATA_START_CODE     126
#define CMD_DATA_STOP_CODE      125

#define DATA_PREFIX  '$'

#endif	/* Q_CORE_H_ */

