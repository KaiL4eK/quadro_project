#!/usr/bin/python
import sys, serial 
import matplotlib.pyplot as plt
import time

ser = serial.Serial(
        port=sys.argv[1],
        baudrate=57600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )

filename = "Serial.log"
bytes_amount = 10;

time20 = [0]
time40 = [0]
time60 = [0]
time80 = [0]
speed_data20 = [0]
speed_data40 = [0]
speed_data60 = [0]
speed_data80 = [0]
thrust_data = [0]

fullTime = 3500
changeSpeedTime = fullTime - 1500
delayTime = 1

# 20 power
time_summ = 0
next_flag = False
ser.write('0')
while time_summ < fullTime:
    data = ser.read( bytes_amount )

    current_speed = ord(data[0]) << 24 | ord(data[1]) << 16 | ord(data[2]) << 8 | ord(data[3])
    time_summ = ord(data[4]) << 24 | ord(data[5]) << 16 | ord(data[6]) << 8 | ord(data[7])
    thrust = ord(data[8]) << 8 | ord(data[9])

    time20.append( time_summ )
    speed_data20.append( current_speed )
    thrust_data.append( thrust )
    wrt_str = "#S:%d#Th:%d#T:%d" % (current_speed, thrust, time_summ)
    print wrt_str

    if ( not next_flag and time_summ > changeSpeedTime ):
        ser.write('2')
        next_flag = True

ser.write('s')

ser.close()
plt.plot( time20, speed_data20, 'k-', label='P20' )
plt.grid()
plt.legend()
plt.ylabel('Speed, rpm')
plt.xlabel('Time, ms')
plt.show()