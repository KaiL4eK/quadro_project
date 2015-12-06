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

#define THROTTLE_RANGE  2000L    //[0 --- 2000]
#define ANGLES_RANGE    1000L   //[-1000 --- 1000]
#define THROTTLE_OFF_LIMIT (THROTTLE_RANGE/25)

#define START_ANGLES    (-ANGLES_RANGE + (ANGLES_RANGE/20))

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

void init_input_control();
void ic_make_calibration();
void ic_find_control();
void send_UART_calibration_data( void );
void send_UART_control_raw_data( void );
void send_UART_control_values( void );
int8_t get_direction_values( Control_values_t *out_dir_vals );

#endif	/* INPUT_CONTROL_H_ */

