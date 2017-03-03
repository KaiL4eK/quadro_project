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

import sys

port_tty = sys.argv[1]

serial_dspic 			= serial.Serial(port=port_tty, baudrate=460800, timeout=15.0)
serial_dspic.bytesize 	= serial.EIGHTBITS #number of bits per bytes
serial_dspic.parity 	= serial.PARITY_NONE #set parity check: no parity
serial_dspic.stopbits 	= serial.STOPBITS_ONE #number of stop bits

angle_pitch 	= [0]
angle_roll 		= [0]
angle_yaw 		= [0]

rate_pitch		= [0]
rate_roll		= [0]
rate_yaw		= [0]

integr_pitch	= [0]
integr_roll		= [0]
integr_yaw		= [0]

control_pitch 	= [0]
control_roll 	= [0]
control_yaw 	= [0]

motor_power		= [0]
battery			= [0]

time_array 		= [0]

sensor_time = 2.5/1000;
angle_rate  = 100.

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

	try:
		while nb.program_run:
			if t.is_alive():

				data = serial_dspic.read( 2 * 14 )

				angle_pitch.append( common.bytes_2_int16( data[0:2] ) / angle_rate )
				angle_roll.append( common.bytes_2_int16( data[2:4] ) / angle_rate )
				angle_yaw.append( common.bytes_2_int16( data[4:6] ) / angle_rate )

				rate_pitch.append( common.bytes_2_int16( data[6:8] ) / angle_rate )
				rate_roll.append( common.bytes_2_int16( data[8:10] ) / angle_rate )
				rate_yaw.append( common.bytes_2_int16( data[10:12] ) / angle_rate )

				integr_pitch.append( common.bytes_2_int16( data[12:14] ) )
				integr_roll.append( common.bytes_2_int16( data[14:16] ) )
				integr_yaw.append( common.bytes_2_int16( data[16:18] ) )

				control_pitch.append( common.bytes_2_int16( data[18:20] ) )
				control_roll.append( common.bytes_2_int16( data[20:22] ) )
				control_yaw.append( common.bytes_2_int16( data[22:24] ) )

				motor_power.append( common.bytes_2_int16( data[24:26] ) )
				battery.append( common.bytes_2_int16( data[26:28] ) / 10.0 )

				fulltime += sensor_time
				time_array.append(fulltime)
			else:
				break

	finally:
		serial_dspic.write( '1' )
		serial_dspic.close()

		out = np.array([time_array, angle_pitch, angle_roll, angle_yaw, rate_pitch, rate_roll, rate_yaw, integr_pitch, integr_roll, integr_yaw, control_pitch, control_roll, control_yaw, motor_power, battery])

		csvfile = "{:%Y%d%m_%H_%M_%S}".format(datetime.now())
		csvfile = "logs/" + csvfile + ".csv"

		with open(csvfile, "w") as output:
			writer = csv.writer(output, lineterminator='\n')
			writer.writerows(out.T)

		plt.plot( time_array, rate_yaw,  'y-', label='Rate' )
		plt.plot( time_array, angle_yaw,  'g-', label='Angle' )
		plt.plot( time_array, integr_yaw,  'k-', label='Integr' )
		plt.plot( time_array, control_yaw,  'r-', label='Control' )

		plt.ylabel('Angle')
		plt.xlabel('Time')
		plt.grid()
		plt.title('Serial')
		plt.legend()
		plt.show()


main()
