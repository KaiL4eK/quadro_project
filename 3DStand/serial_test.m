delete(instrfind);

dspic_serial_obj = serial('/dev/ttyUSB0', 'baudrate', 921600);
fopen(dspic_serial_obj);
data = [];

encoder_rate = 360 / 1000.0 / 2;

for i = 1:10000
    data = [data fread(dspic_serial_obj, 2, 'int32')];
end

fclose(dspic_serial_obj);

angle_data = data' * encoder_rate;

plot(angle_data);
