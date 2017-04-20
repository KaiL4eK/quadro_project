import string
import numpy as np
import math as m
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
import sys
import csv


RADIANS_TO_DEGREES = 180./m.pi
deltat = 5./1000
# beta = 1.8

def update_nomag(accel, gyro, beta, q):    # 3-tuples (x, y, z) for accel, gyro
	# global q
	ax, ay, az = accel                  # Units G (but later normalised)
	gx, gy, gz = (m.radians(x) for x in gyro) # Units deg/s
	q1, q2, q3, q4 = (q[x] for x in range(4))   # short name local variable for readability
	# Auxiliary variables to avoid repeated arithmetic
	_2q1 = 2 * q1
	_2q2 = 2 * q2
	_2q3 = 2 * q3
	_2q4 = 2 * q4
	_4q1 = 4 * q1
	_4q2 = 4 * q2
	_4q3 = 4 * q3
	_8q2 = 8 * q2
	_8q3 = 8 * q3
	q1q1 = q1 * q1
	q2q2 = q2 * q2
	q3q3 = q3 * q3
	q4q4 = q4 * q4

	# Normalise accelerometer measurement
	norm = m.sqrt(ax * ax + ay * ay + az * az)
	if (norm == 0):
		return None# handle NaN
	norm = 1 / norm        # use reciprocal for division
	ax *= norm
	ay *= norm
	az *= norm

	# Gradient decent algorithm corrective step
	s1 = _4q1 * q3q3 + _2q3 * ax + _4q1 * q2q2 - _2q2 * ay
	s2 = _4q2 * q4q4 - _2q4 * ax + 4 * q1q1 * q2 - _2q1 * ay - _4q2 + _8q2 * q2q2 + _8q2 * q3q3 + _4q2 * az
	s3 = 4 * q1q1 * q3 + _2q1 * ax + _4q3 * q4q4 - _2q4 * ay - _4q3 + _8q3 * q2q2 + _8q3 * q3q3 + _4q3 * az
	s4 = 4 * q2q2 * q4 - _2q2 * ax + 4 * q3q3 * q4 - _2q3 * ay
	norm = 1 / m.sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4)    # normalise step magnitude
	s1 *= norm
	s2 *= norm
	s3 *= norm
	s4 *= norm

	# Compute rate of change of quaternion
	qDot1 = 0.5 * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1
	qDot2 = 0.5 * (q1 * gx + q3 * gz - q4 * gy) - beta * s2
	qDot3 = 0.5 * (q1 * gy - q2 * gz + q4 * gx) - beta * s3
	qDot4 = 0.5 * (q1 * gz + q2 * gy - q3 * gx) - beta * s4

	# Integrate to yield quaternion
	q1 += qDot1 * deltat
	q2 += qDot2 * deltat
	q3 += qDot3 * deltat
	q4 += qDot4 * deltat
	norm = 1 / m.sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4)    # normalise quaternion
	q = q1 * norm, q2 * norm, q3 * norm, q4 * norm

	sqw = q[0] * q[0];
	sqx = q[1] * q[1];
	sqy = q[2] * q[2];
	sqz = q[3] * q[3];
	
	rotxrad = m.atan2(2.0 * ( q3 * q4 + q2 * q1 ) , ( -sqx - sqy + sqz + sqw ));
	rotyrad = m.asin(-2.0 * ( q2 * q4 - q3 * q1 ));
	rotzrad = m.atan2(2.0 * ( q2 * q3 + q4 * q1 ) , (  sqx - sqy - sqz + sqw ));

	angle_pitch  = rotxrad * RADIANS_TO_DEGREES;
	angle_roll   = rotyrad * RADIANS_TO_DEGREES;
	angle_yaw    = rotzrad * RADIANS_TO_DEGREES;

	return (angle_pitch, angle_roll, angle_yaw, q)

filename = sys.argv[1]

fig, ax = plt.subplots()
# ax = fig.add_subplot(111)
ax.autoscale(axis='x')
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
atan   = np.arctan2(accel_y, __sq)
accel_angle_pitch   = np.degrees( atan )

pitch 	= np.zeros(len(time_array))
roll 	= np.zeros(len(time_array))
yaw 	= np.zeros(len(time_array))

def runningMeanFast(x, N):
    return np.convolve(x, np.ones((N,))/N)[(N-1):]

plot__, = plt.plot(time_array, accel_angle_pitch, label='Angle')
plt.plot(time_array, runningMeanFast(accel_angle_pitch, 50), label='Filtered')

# plt.plot( time_array, angle_yaw,  'g-', label='Angle' )

plt.ylabel('Angle')
plt.xlabel('Time')
plt.grid()
plt.title('Pitch')
plt.legend()


# Slider initialization
axcolor = 'lightgoldenrodyellow'
ax = plt.axes([0.25, 0.1, 0.65, 0.03])

def update(val):
	pitch 	= np.zeros(time_array.shape)
	roll 	= np.zeros(time_array.shape)
	yaw 	= np.zeros(time_array.shape)
	q = [1.0, 0.0, 0.0, 0.0]

	beta = beta_slider.val
	print 'New beta', beta

	for i in range(1, len(time_array)):
		accel = (accel_x[i], accel_y[i], accel_z[i])
		gyro  = (gyro_x[i], gyro_y[i], gyro_z[i])

		pitch[i], roll[i], yaw[i], q = update_nomag(accel, gyro, beta, q)
		# print update_nomag(accel, gyro)

	# angle_pitch = [float(x) * alpha for x in accel_angle_pitch]
	plot__.set_ydata(pitch)
	# ax.autoscale()

	fig.canvas.draw_idle()

beta_slider = Slider(ax, 'alpha', 0.0, 2.0, valinit=1.0)
beta_slider.on_changed(update)

# Text box initialization
plt.show()
