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
serial_dspic.bytesize 	= serial.EIGHTBITS 		#number of bits per bytes
serial_dspic.parity 	= serial.PARITY_NONE 	#set parity check: no parity
serial_dspic.stopbits 	= serial.STOPBITS_ONE 	#number of stop bits

accel_x 	= [0]
accel_y 	= [0]
accel_z 	= [0]

gyro_x		= [0]
gyro_y		= [0]
gyro_z		= [0]

gyro_sensitivity	= 65.5

sensor_time = 2./1000;
time_array	= [0]

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

			data = serial_dspic.read( 2 * 6 )

			accel_x.append( common.bytes_2_int16( data[0:2] ) )
			accel_y.append( common.bytes_2_int16( data[2:4] ) )
			accel_z.append( common.bytes_2_int16( data[4:6] ) )

			gyro_x.append( common.bytes_2_int16( data[6:8] ) / gyro_sensitivity )
			gyro_y.append( common.bytes_2_int16( data[8:10] ) / gyro_sensitivity )
			gyro_z.append( common.bytes_2_int16( data[10:12] ) / gyro_sensitivity )

			fulltime += sensor_time
			time_array.append(fulltime)

		else:

			serial_dspic.write( '1' )
			serial_dspic.close()
			break

	out = np.array([time_array, 
					accel_x, accel_y, accel_z, 
					gyro_x, gyro_y, gyro_z])

	csvfile = "{:%Y%d%m_%H_%M_%S}".format(datetime.now())
	csvfile = "mpu_logs/" + csvfile + ".csv"

	with open(csvfile, "w") as output:
		writer = csv.writer(output, lineterminator='\n')
		writer.writerows(out.T)

	plt.plot( time_array, gyro_x,  'y-', label='Rate' )

	plt.ylabel('Angle')
	plt.xlabel('Time')
	plt.grid()
	plt.title('Serial')
	plt.legend()
	plt.show()

main()
