#! /usr/bin/python
# coding: utf-8

import serial
import numpy as np
import math as m
import matplotlib.pyplot as plt

import common


serial_dspic 			= serial.Serial(port='/dev/ttyUSB1', baudrate=460800, timeout=15.0)
serial_dspic.bytesize 	= serial.EIGHTBITS #number of bits per bytes
serial_dspic.parity 	= serial.PARITY_NONE #set parity check: no parity
serial_dspic.stopbits 	= serial.STOPBITS_ONE #number of stop bits

filename = "Serial.log"
bytes_amount = 2

angle_pitch = [0]

time_array = [0]

sensor_time = 2.5/1000
angle_rate  = 100.0

POINTS_COUNT	= 1000

def main():
	print "Hello!"
	
	# serial_dspic.open()
	serial_dspic.write( '1' )
	response = serial_dspic.read( 1 );

	if response != '0':
		exit()

	# fd = open( filename, 'w' )
	print "Ok!"
	fulltime = 0

	for i in range( POINTS_COUNT ):
		data = serial_dspic.read( bytes_amount )
		__angle_pitch = common.bytes_2_int16( data[0:2] ) / angle_rate

		angle_pitch.append( __angle_pitch )


		fulltime += sensor_time
		time_array.append(fulltime)

		# wrt_str = "#T:%05d#G:%05d,%05d,%05d#A:%05d,%05d,%05d#R:%05d\n" % (time, x_gyr_raw, y_gyr_raw, int(z_gyr_raw), x_acc_raw, y_acc_raw, z_acc_raw, real_ang)
		# fd.write( wrt_str )
		# ord() - gets code of char element
		# print wrt_str

	serial_dspic.write( '1' )

	serial_dspic.close()

	plt.plot( time_array, angle_pitch,  'g-', label='Angle_p' )

	plt.ylabel('Angle')
	plt.xlabel('Time')
	plt.grid()
	plt.title('Serial')
	plt.legend()
	plt.show()


main()