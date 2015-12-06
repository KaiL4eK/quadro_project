#! /usr/bin/python
# coding: utf-8

import serial

# serialName = '/dev/rfcomm0'
serialName = '/dev/ttyUSB0'

bt_ser = serial.Serial(serialName, baudrate=57600, timeout=15.0)
bt_ser.parity = serial.PARITY_NONE;
bt_ser.bytesize = serial.EIGHTBITS
bt_ser.stopbits = serial.STOPBITS_ONE

filename = "Serial.log"
bytes_amount = 17

def main():
	print "Hello!"
	
	while True:
		data = bt_ser.read( bytes_amount )
		if ord(data[0]) != 90:
			bt_ser.close()
			bt_ser.open()
			continue
		break
	fd = open( filename, 'w' )
	print "Ok!"
	while True:
		data = bt_ser.read( bytes_amount )
		x_gyr_raw = ord(data[2]) << 8 | ord(data[1])
		y_gyr_raw = ord(data[4]) << 8 | ord(data[3])
		z_gyr_raw = ord(data[6]) << 8 | ord(data[5])
		x_acc_raw = ord(data[8]) << 8 | ord(data[7])
		y_acc_raw = ord(data[10]) << 8 | ord(data[9])
		z_acc_raw = ord(data[12]) << 8 | ord(data[11])
		time = ord(data[14]) << 8 | ord(data[13])
		real_ang = ord(data[16]) << 8 | ord(data[15])
		
		if x_gyr_raw > 32767:
			x_gyr_raw = x_gyr_raw - 65536

		if y_gyr_raw > 32767:
			y_gyr_raw = y_gyr_raw - 65536

		if z_gyr_raw > 32767:
			z_gyr_raw = z_gyr_raw - 65536

		if x_acc_raw > 32767:
			x_acc_raw = x_acc_raw - 65536

		if y_acc_raw > 32767:
			y_acc_raw = y_acc_raw - 65536

		if z_acc_raw > 32767:
			z_acc_raw = z_acc_raw - 65536

		if real_ang > 32767:
			real_ang = real_ang - 65536			

		wrt_str = "#T:%05d#G:%05d,%05d,%05d#A:%05d,%05d,%05d#R:%05d\n" % (time, x_gyr_raw, y_gyr_raw, int(z_gyr_raw), x_acc_raw, y_acc_raw, z_acc_raw, real_ang)
		fd.write( wrt_str )
		# ord() - gets code of char element
		# print wrt_str

	bt_ser.close()


main()