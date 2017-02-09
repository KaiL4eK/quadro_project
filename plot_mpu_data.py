import string
import numpy as np
import math as m
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import sys
import csv

filename = sys.argv[1]

# Slider initialization
axcolor = 'lightgoldenrodyellow'
axfreq = plt.axes([0.25, 0.1, 0.65, 0.03], facecolor=axcolor)
axamp = plt.axes([0.25, 0.15, 0.65, 0.03], facecolor=axcolor)

alpha_slider = Slider(axfreq, 'alpha', 0.0, 1.0)

def update(val):
    alpha = alpha_slider.val

    l.set_ydata(amp*np.sin(2*np.pi*freq*t))

    fig.canvas.draw_idle()

alpha_slider.on_changed(update)
				

# Main program
with open(filename, 'rb') as csvfile:
	reader = csv.reader(csvfile)
	[ accel_x, accel_y, accel_z, 
	  gyro_x, gyro_y, gyro_z ] = np.array( list(reader) ).T

accel_angle_pitch   = atan2( accel_y, sqrt(accel_x*accel_x + accel_z*accel_z)) * RADIANS_TO_DEGREES;

plt.plot( time_array, rate_yaw,  'y-', label='Rate' )
plt.plot( time_array, angle_yaw,  'g-', label='Angle' )


plt.ylabel('Angle')
plt.xlabel('Time')
plt.grid()
plt.title('Yaw')
plt.legend()

plt.show()
