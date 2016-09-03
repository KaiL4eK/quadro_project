#ifndef INPUT_CONTROL_H_
#define	INPUT_CONTROL_H_

#include "core.h"

/* Input capture defines */

#define IC_TIMER_2  1
#define IC_TIMER_3  0

#define IC_CE_MODE_DISABLED         0b000
#define IC_CE_MODE_EDGE             0b001
#define IC_CE_MODE_FALLING_EDGE     0b010
#define IC_CE_MODE_RISING_EDGE      0b011
#define IC_CE_MODE_4TH_RISE_EDGE    0b100
#define IC_CE_MODE_16TH_RISE_EDGE   0b101

#define IC_INT_MODE_1ST_CE    0b00
#define IC_INT_MODE_2ND_CE    0b01
#define IC_INT_MODE_3RD_CE    0b10
#define IC_INT_MODE_4TH_CE    0b11

#define THROTTLE_MAX    2000L    //[0 --- 2000]
#define THROTTLE_MIN    0L
#define ANGLES_MAX      1000L   //[-1000 --- 1000]
#define ANGLES_MIN      -1000L

#define TWO_POS_SWITCH_ON   1
#define TWO_POS_SWITCH_OFF  0

#define THROTTLE_OFF_LIMIT (THROTTLE_MAX/10)
#define START_ANGLES    (ANGLES_MIN + (ANGLES_MAX/5))

typedef struct
{
    int32_t    min, 
               max, 
               mid;
}Channel_t;

typedef struct
{
    Channel_t   channel_1,
                channel_2,
                channel_3,
                channel_4,
                channel_5;
}Calibrated_control_t;

typedef struct
{
    int32_t     channel_1,
                channel_2,
                channel_3,
                channel_4,
                channel_5;
}Control_t;

typedef struct
{
    uint16_t    throttle;
    int16_t     roll,
                pitch,
                rudder;
    uint8_t     two_pos_switch;
}Control_values_t;

Control_values_t *ic_init( void );
void ic_make_calibration();
void ic_find_control();
void send_UART_calibration_data( void );
void send_UART_control_raw_data( void );
void send_UART_control_values( void );
int get_control_values( void );

#endif	/* INPUT_CONTROL_H_ */

