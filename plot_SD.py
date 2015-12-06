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

angle_roll = [0]
angle_pitch = [0]
angle_yaw = [0]

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

filename = sys.argv[1]
fd = open( filename, 'r' )
# alpha = 0.005
token_size = 16
sensor_time = 2.5/1000
gyro_sensitivity = 65535/2/250
alpha = 0.995

with open( filename, 'r' ) as f:
	while True:
		rdata = f.read(token_size)
		if not rdata: 
			break
		
		press = ord(rdata[0]) << 24 | ord(rdata[1]) << 16 | ord(rdata[2]) << 8 | ord(rdata[3])
		gyr_x = ord(rdata[4]) << 8 | ord(rdata[5])
		gyr_y = ord(rdata[6]) << 8 | ord(rdata[7])
		gyr_z = ord(rdata[8]) << 8 | ord(rdata[9])
		acc_x = ord(rdata[10]) << 8 | ord(rdata[11])
		acc_y = ord(rdata[12]) << 8 | ord(rdata[13])
		acc_z = ord(rdata[14]) << 8 | ord(rdata[15])

		if gyr_x > 32767:
			gyr_x = gyr_x - 65536

		if gyr_y > 32767:
			gyr_y = gyr_y - 65536

		if gyr_z > 32767:
			gyr_z = gyr_z - 65536

		if acc_x > 32767:
			acc_x = acc_x - 65536

		if acc_y > 32767:
			acc_y = acc_y - 65536

		if acc_z > 32767:
			acc_z = acc_z - 65536

		fulltime += sensor_time
		time.append(fulltime)

		acc_tmp_roll = m.atan2( -1 * acc_x, m.sqrt( acc_y*acc_y + acc_z*acc_z ) ) * 180 / m.pi
		acc_tmp_pitch = m.atan2( acc_y, m.sqrt( acc_x*acc_x + acc_z*acc_z ) ) * 180 / m.pi

		acc_pitch.append( acc_tmp_pitch )
		acc_roll.append( acc_tmp_roll )

		gyr_delta_roll = gyr_y / gyro_sensitivity * sensor_time
		gyr_delta_pitch = gyr_x / gyro_sensitivity * sensor_time
		gyr_delta_yaw = gyr_z / gyro_sensitivity * sensor_time

		angle_roll_tmp = alpha * ( gyr_delta_roll + last_angle_roll ) + ( 1.0 - alpha ) * acc_tmp_roll
		angle_pitch_tmp = alpha * ( gyr_delta_pitch + last_angle_pitch ) + ( 1.0 - alpha ) * acc_tmp_pitch
		angle_yaw_tmp = gyr_delta_yaw + last_angle_yaw

		angle_roll.append( angle_roll_tmp )
		angle_pitch.append( angle_pitch_tmp )
		angle_yaw.append( angle_yaw_tmp )

		last_angle_roll = angle_roll_tmp
		last_angle_pitch = angle_pitch_tmp
		last_angle_yaw = angle_yaw_tmp

f.close()

# fig = plt.figure()
# fig.subplots_adjust(right=0.99, left=0.04, top=0.98, bottom=0.06)

# plt.plot( time, input, 	    'm-', label='Inpt' )
plt.plot( time, acc_roll, 'r-', label='Accl' )
# plt.plot( time, gyro_data,  'c-', label='Gyro' )
# plt.plot( time, real_data,  'b-', label='Ptnc' )
plt.plot( time, angle_roll,  'k-', label='Snsr' )
# plt.plot( time, signal,  	'g-', label='Sign' )
# plt.plot( time, count_data,  'y-', label='Cnt' )

plt.axis([ 0, fulltime, -20, 20 ])
plt.ylabel('Angle')
plt.xlabel('Time')
plt.grid()
plt.title(filename)
plt.legend()
plt.show()