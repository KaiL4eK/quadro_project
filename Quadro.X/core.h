#ifndef Q_CORE_H_
#define	Q_CORE_H_

#include "per_proto.h"
#include "error_.h"

#define CONTROL_BYTE    0xff

#define clip_value( power, min, max ) (power > max ? max : power < min ? min : power)

#define FREQ_CONTROL_SYSTEM   400L

typedef struct
{
    int32_t     roll,
                pitch,
                yaw;
    int32_t     acc_x,
                acc_y;
    int16_t     motor1_power,
                motor2_power,
                motor3_power,
                motor4_power;
}quadrotor_state_t;

#endif	/* Q_CORE_H_ */

