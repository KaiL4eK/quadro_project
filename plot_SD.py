#! /usr/bin/python
# coding: utf-8

import string
import numpy as np
import math as m
import matplotlib.pyplot as plt
import sys


angle_multiplyer = 1000.0

fulltime = 0.0
time = [0]
pressure = [0]
gyr_delta_roll = [0]
gyr_delta_pitch = [0]
gyr_delta_yaw = [0]

acc_pitch = [0]
acc_roll = [0]

# angle_roll = [0]
# angle_pitch = [0]
# angle_yaw = [0]

last_angle_roll = 0
last_angle_pitch = 0
last_angle_yaw = 0

sens_data = [0]
real_data = [0]
input = [0]
accel_data = [0]
last_gyro_angle = 0.0
gyro_data = [0]
count_data = [0]
last_angle = 0.0

gyro_pitch			= [0]
gyro_roll 			= [0]
gyro_yaw			= [0]

angle_pitch 		= [0]
angle_roll			= [0]
angle_yaw			= [0]

integr_pitch 		= [0]
integr_roll			= [0]
integr_yaw			= [0]

control_pitch		= [0]
control_roll		= [0]
control_throttle 	= [0]

motor_1				= [0]
motor_2				= [0]
motor_3				= [0]
motor_4				= [0]

filename = sys.argv[1]

token_size = 32
sensor_time = 10.0/1000
gyro_sensitivity = 65535/2/250.0
alpha = 0.98

def bytes_2_int16( bytes ):
	int16 = ord(bytes[0]) << 8 | ord(bytes[1])

	if int16 > (2**15 - 1):
		int16 = int16 - (2**16 - 1)

	return int16

# altitude_multiplyer = 1000.0

with open( filename, 'r' ) as f:
	while True:
		rdata = f.read(token_size)
		if not rdata:
			break

		_gyr_pitch 			= bytes_2_int16( rdata[0:2] )/gyro_sensitivity
		_gyr_roll 			= bytes_2_int16( rdata[2:4] )/gyro_sensitivity
		_gyr_yaw 			= bytes_2_int16( rdata[4:6] )/gyro_sensitivity

		_angle_pitch 		= bytes_2_int16( rdata[6:8] )
		_angle_roll 		= bytes_2_int16( rdata[8:10] )
		_angle_yaw 			= bytes_2_int16( rdata[10:12] )

		_integr_pitch 		= bytes_2_int16( rdata[12:14] )

		_control_throttle	= bytes_2_int16( rdata[18:20] )
		_control_pitch 		= bytes_2_int16( rdata[20:22] )
		_control_roll 		= bytes_2_int16( rdata[22:24] )


		fulltime += sensor_time
		time.append(fulltime)

		gyro_pitch.append( _gyr_pitch )

		control_throttle.append( _control_throttle )
		control_pitch.append( _control_pitch )
		integr_pitch.append( _integr_pitch )

		angle_pitch.append( _angle_pitch )

		# _acc_roll = m.atan2( -1 * acc_x, m.sqrt( acc_y*acc_y + acc_z*acc_z ) ) * 180 / m.pi
		# _acc_pitch = m.atan2( acc_y, m.sqrt( acc_x*acc_x + acc_z*acc_z ) ) * 180 / m.pi

		# acc_pitch.append( _acc_pitch )
		# acc_roll.append( _acc_roll )

		# gyr_delta_roll = gyr_y / gyro_sensitivity * sensor_time
		# gyr_delta_pitch = gyr_x / gyro_sensitivity * sensor_time
		# gyr_delta_yaw = gyr_z / gyro_sensitivity * sensor_time

		# angle_roll_tmp = alpha * ( gyr_delta_roll + last_angle_roll ) + ( 1.0 - alpha ) * _acc_roll
		# angle_pitch_tmp = alpha * ( gyr_delta_pitch + last_angle_pitch ) + ( 1.0 - alpha ) * _acc_pitch
		# angle_yaw_tmp = gyr_delta_yaw + last_angle_yaw

		# angle_roll.append( angle_roll_tmp * 100 )
		# angle_pitch.append( angle_pitch_tmp * 100 )
		# angle_yaw.append( angle_yaw_tmp )

		# last_angle_roll = angle_roll_tmp
		# last_angle_pitch = angle_pitch_tmp
		# last_angle_yaw = angle_yaw_tmp


# fig = plt.figure()
# fig.subplots_adjust(right=0.99, left=0.04, top=0.98, bottom=0.06)

# plt.plot( time, control_throttle, 'y-', label='Cntrl_t' )
# plt.plot( time, integr_pitch, 'r-', label='Integr_p' )
# plt.plot( time, angle_pitch,  'k-', label='Angle_p' )
# plt.plot( time, control_pitch,  'b-', label='Cntrl_p' )
# plt.plot( time, gyro_pitch,  'g-', label='Gyro_p' )

# # plt.axis([ 0, fulltime, -20, 20 ])
# plt.ylabel('Angle')
# plt.xlabel('Time')
# plt.grid()
# plt.title(filename)
# plt.legend()
# plt.show()
