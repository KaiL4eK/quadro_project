/*
 * File:   atan2_fx.c
 * Author: Alex Devyatkin
 *
 * Created on February 22, 2016, 11:10 AM
 */

#include <stdint.h>
#include "math_proto.h"

#define MULTIPLY_FP_RESOLUTION_BITS        15

int16_t atan2_fp ( int16_t y_fp, int16_t x_fp )
{
        int32_t coeff_1 = 45;
        int32_t coeff_1b = -56;        // 56.24;
        int32_t coeff_1c = 11;        // 11.25
        int16_t coeff_2 = 135;

        int16_t angle = 0;

        int32_t r;
        int32_t r3;

        int16_t y_abs_fp = y_fp;
        if (y_abs_fp < 0)
                y_abs_fp = -y_abs_fp;

        if (y_fp == 0)
        {
                if (x_fp >= 0)
                {
                        angle = 0;
                }
                else
                {
                        angle = 180;
                }
        }
        else if (x_fp >= 0)
        {
                r = (((int32_t)(x_fp - y_abs_fp)) << MULTIPLY_FP_RESOLUTION_BITS) /
                    ((int32_t)(x_fp + y_abs_fp));

                r3 = r * r;
                r3 =  r3 >> MULTIPLY_FP_RESOLUTION_BITS;
                r3 *= r;
                r3 =  r3 >> MULTIPLY_FP_RESOLUTION_BITS;
                r3 *= coeff_1c;
                angle = (int16_t) ( coeff_1 + ((coeff_1b * r + r3) >>
                        MULTIPLY_FP_RESOLUTION_BITS) );
        }
        else
        {
                r = (((int32_t)(x_fp + y_abs_fp)) << MULTIPLY_FP_RESOLUTION_BITS) /
                    ((int32_t)(y_abs_fp - x_fp));
                r3 = r * r;
                r3 =  r3 >> MULTIPLY_FP_RESOLUTION_BITS;
                r3 *= r;
                r3 =  r3 >> MULTIPLY_FP_RESOLUTION_BITS;
                r3 *= coeff_1c;
                angle = coeff_2 + ((int16_t)(((coeff_1b * r + r3) >>
                        MULTIPLY_FP_RESOLUTION_BITS)) );
        }

        if (y_fp < 0)
                return (-angle);     // negate if in quad III or IV
        else
                return (angle);
}