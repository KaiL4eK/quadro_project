
def bytes_2_int16( bytes ):
	int16 = ord(bytes[0]) << 8 | ord(bytes[1])

	if int16 > (2**15 - 1):
		int16 = int16 - (2**16 - 1)

	return int16