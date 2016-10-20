data = transp(Thrust);
t = 0:10:(length(data)-1)*10;
data_size = size(data);
strings_count = data_size(2);
av_data = [];
for i = 1:strings_count
   av_data = [av_data smooth(data(:, i))];
end
plot( t, av_data )
% plot( t, data, t, av_data )
