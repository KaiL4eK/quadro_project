#ifndef Q_CORE_H_
#define	Q_CORE_H_

#include "per_proto.h"
#include "error_.h"

#define CONTROL_BYTE    0xff

#define FREQ_CONTROL_SYSTEM   400L

typedef struct
{
    int32_t roll,
            pitch,
            yaw;
    int32_t acc_x,
            acc_y;
    int32_t gyr_delta_x,
            gyr_delta_y,
            gyr_delta_z;
}Angles_t;

#endif	/* Q_CORE_H_ */

