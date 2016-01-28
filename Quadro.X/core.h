#ifndef Q_CORE_H_
#define	Q_CORE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xc.h>

#define CONTROL_BYTE    0xff

//Nice setup for external HS 32MHz Quartz
#define FREQ_32MHZ

#ifndef FREQ_32MHZ
#define FOSC        80000000ULL
#else
#define FOSC        32000000ULL
#endif
#define FCY         (FOSC/2)
#define FSENS       400L

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

