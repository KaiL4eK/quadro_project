#ifndef INPUT_CONTROL_H_
#define	INPUT_CONTROL_H_

#include "core.h"

#define THROTTLE_MAX        2000L    //[0 --- 2000]
#define THROTTLE_MIN        0L
#define ANGLES_MAX          1000L   //[-1000 --- 1000]
#define ANGLES_MIN          -1000L

#define THROTTLE_OFF_LIMIT  (200)
#define START_ANGLES        (ANGLES_MIN + (ANGLES_MAX/5))

#define TWO_POS_SWITCH_ON           1
#define TWO_POS_SWITCH_OFF          0

typedef struct
{
    uint16_t    throttle;
    int16_t     roll,
                pitch,
                rudder;
    uint8_t     two_pos_switch;
}Control_values_t;

Control_values_t *remote_control_init( void );
void remote_control_make_calibration( UART_moduleNum_t module );
void remote_control_find_controller();
int remote_control_update_control_values( void );

void remote_control_send_UART_control_raw_data( UART_moduleNum_t module );
void remote_control_send_UART_control_values( UART_moduleNum_t module );


#endif	/* INPUT_CONTROL_H_ */

