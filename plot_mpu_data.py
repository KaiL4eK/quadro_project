import string
import numpy as np
import math as m
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import sys
import csv

filename = sys.argv[1]

fig = plt.figure()
ax = fig.add_subplot(111)
fig.subplots_adjust(left=0.15, bottom=0.25)

# Main program
with open(filename, 'rb') as csvfile:
	reader = csv.reader(csvfile)
	[ time_array,
	  accel_x, accel_y, accel_z, 
	  gyro_x, gyro_y, gyro_z,
	  port_status ] = np.array( list(reader), dtype=float ).T

__sq_x = np.multiply(accel_x, accel_x)
__sq_z = np.multiply(accel_z, accel_z) + __sq_x
__sq   = np.sqrt(__sq_z)
atan   = np.arctan2( accel_y, __sq)
accel_angle_pitch   = np.degrees( atan )

plot__, = plt.plot(	time_array, accel_angle_pitch, lw=1, color='red', label='Rate' )

# plt.plot( time_array, angle_yaw,  'g-', label='Angle' )

plt.ylabel('Angle')
plt.xlabel('Time')
plt.grid()
plt.title('Pitch')
plt.legend()

# Slider initialization
axcolor = 'lightgoldenrodyellow'
ax = plt.axes([0.25, 0.1, 0.65, 0.03])

alpha_slider = Slider(ax, 'alpha', 0.0, 1.0, valinit=1.0)

def update(val):
    alpha = alpha_slider.val

    __gyro_x = [float(x) * alpha for x in accel_angle_pitch]
    plot__.set_ydata( __gyro_x )

    fig.canvas.draw_idle()

alpha_slider.on_changed(update)

# Text box initialization


plt.show()
