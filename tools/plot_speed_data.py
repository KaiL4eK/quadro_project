
import matplotlib.pyplot as plt
import sys

filename = sys.argv[1]

speed_f = False
thrust_f = False
target_f = False
electr_f = False

speed_data = {  }
thrust_data = {  }
target_data = {  }
electr_data = {  }

def reset_flags():
	speed_f = False
	thrust_f = False
	target_f = False
	electr_f = False

with open( filename, 'r' ) as logFile:
	for line in logFile:
		if line == '#Speed\n':
			print 'Speed!'
			reset_flags()
			speed_f = True
			continue
		elif line == '#Thrust\n':
			print 'Thrust!'
			reset_flags()
			thrust_f = True
			continue
		elif line == '#Target\n':
			print 'Target!'
			reset_flags()
			target_f = True
			continue
		elif line == '#Electr\n':
			print 'Electr!'
			reset_flags()
			electr_f = True
			continue

		data = line.split(',')

		if speed_f:
			speed_data[float(data[0])] = int(data[1])

		if thrust_f:
			thrust_data[float(data[0])] = int(data[1])

		if target_f:
			target_data[float(data[0])] = int(data[1])

		if electr_f:
			electr_data[float(data[0])] = int(data[1])



f, ax1 = plt.subplots()
plt.grid()
ax1.plot( speed_data.keys(), speed_data.values(), 'k.' )
ax1.plot( target_data.keys(), target_data.values(), '.' )
ax1.plot( electr_data.keys(), electr_data.values(), 'g' )
for tl in ax1.get_yticklabels():
    tl.set_color( 'k' )
# ax1.set_ylabel( 'Speed' )

ax2 = ax1.twinx()
ax2.plot( thrust_data.keys(), thrust_data.values(), 'm.' )
for tl in ax2.get_yticklabels():
    tl.set_color( 'm' )
ax2.set_ylabel( 'Thrsut' )

plt.xlabel('Time, ms')
plt.show()


