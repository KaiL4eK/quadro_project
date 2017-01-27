#! /usr/bin/python
# coding: utf-8

import serial
import numpy as np
import math as m
import matplotlib.pyplot as plt

import common
import nonblock as nb

from datetime import datetime
import csv

serial_dspic 			= serial.Serial(port='/dev/ttyUSB1', baudrate=460800, timeout=15.0)
serial_dspic.bytesize 	= serial.EIGHTBITS #number of bits per bytes
serial_dspic.parity 	= serial.PARITY_NONE #set parity check: no parity
serial_dspic.stopbits 	= serial.STOPBITS_ONE #number of stop bits

angle_pitch 		= [0]
rate_pitch 			= [0]
rate_pitch_ref 		= [0]
rate_pitch_integral	= [0]

motor_power			= [0]

time_array = [0]

sensor_time = 2.5/1000
angle_rate  = 10.0

POINTS_COUNT	= 1000

t = nb.waitOnInput()

def main():
	print "Hello!"
	
	serial_dspic.write( '1' )
	response = serial_dspic.read( 1 );

	if response != '0':
		exit()

	print "Ok!"
	fulltime = 0

	t.start()

	while nb.program_run:
		if t.is_alive():

			data = serial_dspic.read( 2 * 4 )
			__angle_pitch 			= common.bytes_2_int16( data[0:2] ) / angle_rate
			angle_pitch.append( __angle_pitch )

			__rate_pitch 			= common.bytes_2_int16( data[2:4] )
			rate_pitch.append( __rate_pitch )

			# percents
			__power					= common.bytes_2_int16( data[4:6] ) / 20	# Just for scale
			motor_power.append( __power )

			__rate_pitch_ref 		= common.bytes_2_int16( data[6:8] )
			rate_pitch_ref.append( __rate_pitch_ref )
			
			# __rate_pitch_integral 	= common.bytes_2_int16( data[6:8] )
			# rate_pitch_integral.append( __rate_pitch_integral )

			fulltime += sensor_time
			time_array.append(fulltime)

			# wrt_str = "#T:%05d#G:%05d,%05d,%05d#A:%05d,%05d,%05d#R:%05d\n" % (time, x_gyr_raw, y_gyr_raw, int(z_gyr_raw), x_acc_raw, y_acc_raw, z_acc_raw, real_ang)
			# fd.write( wrt_str )
			# ord() - gets code of char element
			# print wrt_str
			# 
			# print __rate_pitch

		else:

			serial_dspic.write( '1' )
			serial_dspic.close()
			break

	out = np.array([time_array, angle_pitch, rate_pitch, motor_power, rate_pitch_ref])

	csvfile = "{:%Y%d_%H_%M_%S}".format(datetime.now())
	csvfile = "logs/" + csvfile + ".csv"

	with open(csvfile, "w") as output:
		writer = csv.writer(output, lineterminator='\n')
		writer.writerows(out.T)

	plt.plot( time_array, rate_pitch,  'y-', label='Rate_p' )
	plt.plot( time_array, angle_pitch,  'g-', label='Angle_p' )
	plt.plot( time_array, motor_power,  'k-', label='Power' )
	plt.plot( time_array, rate_pitch_ref,  'r-', label='RateReference_p' )
	# plt.plot( time_array, rate_pitch_integral,  'b-', label='Rate_Integral_p' )

	plt.ylabel('Angle')
	plt.xlabel('Time')
	plt.grid()
	plt.title('Serial')
	plt.legend()
	plt.show()


main()