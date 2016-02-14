#! /usr/bin/python
# coding: utf-8

import serial

control_byte_val = 0xff

# serialName = '/dev/rfcomm0'
serialName = '/dev/ttyUSB0'

bt_ser = serial.Serial(serialName, baudrate=57600, timeout=15.0)
bt_ser.parity = serial.PARITY_NONE;
bt_ser.bytesize = serial.EIGHTBITS
bt_ser.stopbits = serial.STOPBITS_ONE

filename = "Serial.log"
# bytes_amount = 12
bytes_amount = 6

def main():
	print "Hello!"
	rest_cntr = 0
	flag_ok = 0
	# while True:
	# 	data = bt_ser.read( bytes_amount )
	# 	if (ord(data[7]) != control_byte_val) & (ord(data[0]) != control_byte_val):
	# 		bt_ser.close()
	# 		bt_ser.open()
	# 		continue
	# 	break
	fd = open( filename, 'w' )
	while True:
		try:
			data = bt_ser.read( bytes_amount )
			if (ord(data[5]) != control_byte_val) & (ord(data[0]) != control_byte_val):
				bt_ser.close()
				bt_ser.open()
				fd.close()
				fd = open( filename, 'w' )
				print "Restart!", rest_cntr
				rest_cntr += 1
				flag_ok = 0;
				continue
			
			if flag_ok == 0:
				flag_ok = 1;
				print "Ok!"

			sens_ang = ord(data[2]) << 8 | ord(data[1])
			# real_ang = ord(data[4]) << 8 | ord(data[3])
			control  = ord(data[4]) << 8 | ord(data[3])
			# acc_x 	 = ord(data[9]) << 8 | ord(data[8])
			# gyr_x 	 = ord(data[11]) << 8 | ord(data[10])

			if sens_ang > 32767:
				sens_ang = sens_ang - 65536

			# if real_ang > 32767:
			# 	real_ang = real_ang - 65536			

			if control > 32767:
				control = control - 65536	

			# if acc_x > 32767:
			# 	acc_x = acc_x - 65536

			# if gyr_x > 32767:
			# 	gyr_x = gyr_x - 65536

			# wrt_str = "#S:%d#R:%d#I:%d#A:%d#G:%d\n" % (sens_ang, real_ang, control, acc_x, gyr_x)
			# wrt_str = "#S:%d#R:%d#I:%d\n" % (sens_ang, real_ang, control)
			wrt_str = "#S:%d#I:%d\n" % (sens_ang, control)
			fd.write( wrt_str )
			# ord() - gets code of char element
			# print time_cycle
		except KeyboardInterrupt:
			bt_ser.close()
			print "Close!"
			break
		except:
			bt_ser.close()
			bt_ser.open()
			print "Reconnect!"
			continue

main()