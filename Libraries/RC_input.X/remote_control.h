#ifndef INPUT_CONTROL_H_
#define	INPUT_CONTROL_H_

#include <per_proto.h>

/*
 * Modules utilized:
 *      TIMER           3
 *      INPUT_CAPTURE   1   RD8
 *      INPUT_CAPTURE   2   RD9
 *      INPUT_CAPTURE   3   RD10
 *      INPUT_CAPTURE   4   RD11
 *      INPUT_CAPTURE   5   RD12
 */

#ifdef DSPIC_ENABLE_PLL
    #error "PLL not supported yet"
#endif

#define THROTTLE_MAX            2000L    //[0 --- 2000]
#define THROTTLE_MIN            0L
#define ANGLES_MAX              1000L   //[-1000 --- 1000]
#define ANGLES_MIN              -1000L

#define TWO_POS_SWITCH_ON           1
#define TWO_POS_SWITCH_OFF          0

typedef struct
{
    uint16_t    throttle;
    int16_t     roll,
                pitch,
                rudder;
    uint8_t     two_pos_switch;
}RC_input_values_t;

typedef struct
{
    int32_t     channel_1,
                channel_2,
                channel_3,
                channel_4,
                channel_5;
}RC_input_raw_t;

RC_input_values_t *remote_control_init( void );
void remote_control_make_calibration( uart_module_t module );
bool remote_control_find_controller();
int remote_control_update_control_values( void );

void remote_control_send_UART_control_raw_data( uart_module_t module );
void remote_control_send_UART_control_values( uart_module_t module );

// For debug only
RC_input_raw_t *remote_control_get_raw_prt ( void );

#endif	/* INPUT_CONTROL_H_ */

