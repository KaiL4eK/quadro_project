import string
import numpy as np
import math as m
import matplotlib.pyplot as plt
import sys
import csv

filename = sys.argv[1]

with open(filename, 'rb') as csvfile:
	reader = csv.reader(csvfile)
	[ time_array, angle_pitch, angle_roll, angle_yaw, rate_pitch, 
	  rate_roll, rate_yaw, integr_pitch, integr_roll, integr_yaw, 
	  control_pitch, control_roll, control_yaw, motor_power, battery ] = np.array( list(reader), dtype=float ).T

plt.figure(1)
plt.plot( time_array, rate_yaw,  'y-', label='Rate' )
plt.plot( time_array, angle_yaw,  'g-', label='Angle' )
plt.plot( time_array, integr_yaw,  'k-', label='Integr' )
plt.plot( time_array, control_yaw,  'r-', label='Control' )

plt.ylabel('Angle')
plt.xlabel('Time')
plt.grid()
plt.title('Yaw')
plt.legend()

plt.figure(2)
plt.plot( time_array, rate_roll,  'y-', label='Rate' )
plt.plot( time_array, angle_roll,  'g-', label='Angle' )
plt.plot( time_array, integr_roll,  'k-', label='Integr' )
plt.plot( time_array, control_roll,  'r-', label='Control' )

plt.ylabel('Angle')
plt.xlabel('Time')
plt.grid()
plt.title('Roll')
plt.legend()

plt.figure(3)
plt.plot( time_array, rate_pitch,  'y-', label='Rate' )
plt.plot( time_array, angle_pitch,  'g-', label='Angle' )
plt.plot( time_array, integr_pitch,  'k-', label='Integr' )
# plt.plot( time_array, control_pitch,  'r-', label='Control' )

plt.ylabel('Angle')
plt.xlabel('Time')
plt.grid()
plt.title('Pitch')
plt.legend()

plt.figure(4)
motor_power = [float(x) / 20 for x in motor_power]
plt.plot( time_array, motor_power,  'y-', label='Power' )
plt.plot( time_array, battery,  'g-', label='Battery' )

plt.ylabel('Value')
plt.xlabel('Time')
plt.grid()
plt.title('Power')
plt.legend()

plt.show()
