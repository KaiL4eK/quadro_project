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
bytes_amount = 6;

time_speed = [ 0.0 ]
time_other = []

speed_data = [ 0 ]
thrust_data = [ ]
target_data = [ ]
electr_data = [ ]
time_sum = 0.0 # milliseconds for all time_* variables
time_step = 2.5; 

fullTime = 7000
changeSpeedTime = 500
delayTime = 1

next_flag = False
ser.write('w')
while time_sum < fullTime:
    data = ser.read( bytes_amount )

    current_speed =  ord(data[0]) << 8 | ord(data[1])
    thrust = ord(data[2]) << 8 | ord(data[3])
    target = ord(data[4]) << 8 | ord(data[5])
    # electr = ord(data[6]) << 8 | ord(data[7])

    time_sum = time_sum + time_step

    if current_speed != 65535:
    	time_speed.append( time_sum )
    	speed_data.append( current_speed )

    # electr_data.append( electr )
    target_data.append( target )
    thrust_data.append( thrust )
    time_other.append( time_sum )

    # wrt_str = "#S:%d#Th:%d#T:%d#C:%d" % (current_speed, thrust, target, electr)
    wrt_str = "#S:%d#Th:%d#T:%d" % (current_speed, thrust, target)
    print wrt_str

    if ( not next_flag and time_sum > changeSpeedTime ):
        ser.write('q')
        next_flag = True

ser.write('s')
ser.close()

plt.close('all')

# Save to logsile

speed_d = dict( zip( time_speed, speed_data ) )
thrust_d = dict( zip( time_other, thrust_data ) )
target_d = dict( zip( time_other, target_data ) )
# electr_d = dict( zip( time_other, electr_data ) )

with open( filename, 'w' ) as logFile:
	logFile.write("#Speed\n");
	for time_s in sorted( speed_d ):
	    logFile.write( "%f,%d\n" % (time_s, speed_d[time_s]) )
	logFile.write("#Thrust\n");
	for time_t in sorted( thrust_d ):
	    logFile.write( "%f,%d\n" % (time_t, thrust_d[time_t]) )
	logFile.write("#Target\n");
	for time_t in sorted( target_d ):
	    logFile.write( "%f,%d\n" % (time_t, target_d[time_t]) )
	# logFile.write("#Electr\n");
	# for time_e in sorted( electr_d ):
	#     logFile.write( "%f,%d\n" % (time_e, electr_d[time_e]) )

f, ax1 = plt.subplots()
plt.grid()
ax1.plot( time_speed, speed_data, 'k', label='Speed' )
ax1.plot( time_other, target_data, label='Target' )
# ax1.plot( time_other, electr_data, label='Current' )
for tl in ax1.get_yticklabels():
    tl.set_color( 'k' )
# ax1.set_ylabel( 'Speed' )
ax1.legend()

ax2 = ax1.twinx()
ax2.plot( time_other, thrust_data, 'm' )
for tl in ax2.get_yticklabels():
    tl.set_color( 'm' )
ax2.set_ylabel( 'Thrsut' )


plt.xlabel('Time, ms')
plt.show()
