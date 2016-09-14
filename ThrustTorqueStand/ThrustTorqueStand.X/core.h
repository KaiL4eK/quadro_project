#ifndef CORE_H_
#define	CORE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <xc.h>
#include "per_proto.h"

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

void tacho_init ( void );
int16_t tacho_get_round_speed ( void );
void tacho_start_cmd ( void );
void tacho_stop_cmd ( void );

int init_sinus ( uint16_t halfAmpl, uint16_t period_time_ms, uint16_t offset );
int init_square ( uint16_t lowLevel, uint16_t highLevel, uint16_t halfTime );
uint16_t get_next_signal_value ( void );
uint16_t get_signal_zero_lvl ( void );

UART_frame_t *cmdProcessor_rcvFrame ( void );
void cmdProcessor_write_cmd ( UART_moduleNum_t module, uint8_t prefix, uint8_t code );

#endif	/* CORE_H_ */






