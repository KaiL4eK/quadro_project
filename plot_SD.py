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

angle_pitch 	= [0]
angle_roll		= [0]
angle_yaw		= [0]

integr_pitch 	= [0]
integr_roll		= [0]
integr_yaw		= [0]

control_pitch	= [0]
control_roll	= [0]
control_throttle = [0]

motor_1			= [0]
motor_2			= [0]
motor_3			= [0]
motor_4			= [0]

filename = sys.argv[1]
# fd = open( filename, 'r' )
# alpha = 0.005
token_size = 32
sensor_time = 10.0/1000
gyro_sensitivity = 65535/2/250
alpha = 0.98

# altitude_multiplyer = 1000.0

with open( filename, 'r' ) as f:
	while True:
		rdata = f.read(token_size)
		if not rdata: 
			break

		gyr_x = ord(rdata[0]) << 8 | ord(rdata[1])
		gyr_y = ord(rdata[2]) << 8 | ord(rdata[3])
		gyr_z = ord(rdata[4]) << 8 | ord(rdata[5])
		_angle_pitch = ord(rdata[6]) << 8 | ord(rdata[7])
		_angle_roll = ord(rdata[8]) << 8 | ord(rdata[9])
		_angle_yaw = ord(rdata[10]) << 8 | ord(rdata[11])

		_integr_pitch 		= ord(rdata[12]) << 8 | ord(rdata[13])

		_control_pitch 		= ord(rdata[20]) << 8 | ord(rdata[21])
		_control_roll 		= ord(rdata[22]) << 8 | ord(rdata[23])
		_control_throttle	= ord(rdata[18]) << 8 | ord(rdata[19])

		if gyr_x > 32767:
			gyr_x = gyr_x - 65536

		if gyr_y > 32767:
			gyr_y = gyr_y - 65536

		if gyr_z > 32767:
			gyr_z = gyr_z - 65536

		if _angle_pitch > 32767:
			_angle_pitch = _angle_pitch - 65536

		if _angle_roll > 32767:
			_angle_roll = _angle_roll - 65536

		if _angle_yaw > 32767:
			_angle_yaw = _angle_yaw - 65536

		if _control_pitch > 32767:
			_control_pitch = _control_pitch - 65536

		if _control_roll > 32767:
			_control_roll = _control_roll - 65536

		if _control_throttle > 32767:
			_control_throttle = _control_throttle - 65536

		if _integr_pitch > 32767:
			_integr_pitch = _integr_pitch - 65536

		fulltime += sensor_time
		time.append(fulltime)

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


plt.plot( time, control_throttle, 'y-', label='Cntrl_t' )
plt.plot( time, integr_pitch, 'r-', label='Integr_p' )
plt.plot( time, angle_pitch,  'k-', label='Angle_p' )
plt.plot( time, control_pitch,  'b-', label='Cntrl_p' )

# plt.axis([ 0, fulltime, -20, 20 ])
plt.ylabel('Angle')
plt.xlabel('Time')
plt.grid()
plt.title(filename)
plt.legend()
plt.show()