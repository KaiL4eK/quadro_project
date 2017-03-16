/*
 * File:   inv_root.c
 * Author: alex
 *
 * Created on March 16, 2017, 2:55 PM
 */

#include <stdint.h>
#include "math_proto.h"

float inv_sqrt ( float x ) 
{
	float halfx = 0.5f * x;
	float y = x;
	int32_t i = *(int32_t*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
	return y;
}