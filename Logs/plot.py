#! /usr/bin/python
# coding: utf-8

import string
import scanf as sc
import numpy as np
import matplotlib.pyplot as plt
import sys

angle_multiplyer = 1000.0

filename = sys.argv[1]
fulltime = 0.0
time = [0]
sens_data = [0]
real_data = [0]
input = [0]
accel_data = [0]
last_gyro_angle = 0.0
gyro_data = [0]
count_data = [0]
last_angle = 0.0

fd = open( filename, 'r' )

# alpha = 0.005

for line in fd:
	# data = sc.sscanf( line , "#S:%d#R:%d#I:%d#A:%d#G:%d" )
	data = sc.sscanf( line , "#S:%d#I:%d" )
	fulltime += 2.5/1000
	time.append(fulltime)
	sens_data.append(data[0]/angle_multiplyer)
	# real_data.append(data[1]/11.5)
	input.append(data[1]/angle_multiplyer)
	# accel_data.append(data[3]/angle_multiplyer)
	# last_gyro_angle += data[4]/angle_multiplyer
	# gyro_data.append(last_gyro_angle)

	# count_data.append( (1 - alpha)*(last_angle + data[4]/angle_multiplyer) + alpha*(data[3]/angle_multiplyer) );
	# last_angle = data[0]/angle_multiplyer
# fig = plt.figure()
# fig.subplots_adjust(right=0.99, left=0.04, top=0.98, bottom=0.06)

plt.plot( time, input, 	    'm-', label='Inpt' )
# plt.plot( time, accel_data, 'r-', label='Accl' )
# plt.plot( time, gyro_data,  'c-', label='Gyro' )
# plt.plot( time, real_data,  'b-', label='Ptnc' )
plt.plot( time, sens_data,  'k-', label='Snsr' )
# plt.plot( time, signal,  	'g-', label='Sign' )
# plt.plot( time, count_data,  'y-', label='Cnt' )

plt.axis([ 0, fulltime, -20, 20 ])
plt.ylabel('Angle')
plt.xlabel('Time')
plt.grid()
plt.title(filename)
plt.legend()
plt.show()