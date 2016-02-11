#!/usr/bin/python
import serial

try: 
	ser = serial.Serial(
	    port='/dev/ttyUSB0',
	    baudrate=57600,
	    parity=serial.PARITY_NONE,
	    stopbits=serial.STOPBITS_ONE,
	    bytesize=serial.EIGHTBITS
	)
except Exception, e:
	print "Unable to open serial port /dev/ttyUSB0"
	ser = serial.Serial(
	    port='/dev/ttyUSB1',
	    baudrate=57600,
	    parity=serial.PARITY_NONE,
	    stopbits=serial.STOPBITS_ONE,
	    bytesize=serial.EIGHTBITS
	)

try: 
    while ser.isOpen():
        var = raw_input("Please enter command: ")
        if len(var):
            ser.write(var[0])
except Exception, e:
    print "Serial port error: " + str(e)
finally:
    ser.close()





