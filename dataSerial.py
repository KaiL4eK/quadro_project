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

serial_dspic 			= serial.Serial(port='/dev/ttyUSB0', baudrate=460800, timeout=15.0)
serial_dspic.bytesize 	= serial.EIGHTBITS #number of bits per bytes
serial_dspic.parity 	= serial.PARITY_NONE #set parity check: no parity
serial_dspic.stopbits 	= serial.STOPBITS_ONE #number of stop bits

angle 			= [0]
rate 			= [0]
rate_ref 		= [0]
rate_integral	= [0]

control 		= [0]

motor_power		= [0]
integr 			= [0]

time_array 		= [0]

sensor_time = 2.5/1000
angle_rate  = 10.0

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

			data = serial_dspic.read( 2 * 5 )

			angle.append( common.bytes_2_int16( data[0:2] ) / angle_rate )
			rate.append( common.bytes_2_int16( data[2:4] ) )
			integr.append( common.bytes_2_int16( data[4:6] ) )
			rate_ref.append( common.bytes_2_int16( data[6:8] ) )
			control.append( common.bytes_2_int16( data[8:10] ) )

			fulltime += sensor_time
			time_array.append(fulltime)

		else:

			serial_dspic.write( '1' )
			serial_dspic.close()
			break

	out = np.array([time_array, angle, rate, integr, rate_ref])

	csvfile = "{:%Y%d_%H_%M_%S}".format(datetime.now())
	csvfile = "logs/" + csvfile + ".csv"

	with open(csvfile, "w") as output:
		writer = csv.writer(output, lineterminator='\n')
		writer.writerows(out.T)

	plt.plot( time_array, rate,  'y-', label='Rate' )
	plt.plot( time_array, angle,  'g-', label='Angle' )
	plt.plot( time_array, integr,  'k-', label='Integr' )
	plt.plot( time_array, rate_ref,  'r-', label='RateReference' )
	plt.plot( time_array, control,  'b-', label='Control' )

	plt.ylabel('Angle')
	plt.xlabel('Time')
	plt.grid()
	plt.title('Serial')
	plt.legend()
	plt.show()


main()