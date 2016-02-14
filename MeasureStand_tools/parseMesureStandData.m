function [ speed_data thrust_data target_data ] = parseMesureStandData( filename )
    speed_data = [];
    thrust_data = [];
    target_data = [];

    fid = fopen( filename, 'r' );
    
    data_type = 0;
    line = fgets( fid );
    while line ~= -1
        switch sscanf( line, '#%s' )
            case 'Speed'
                data_type = 1;
            case 'Thrust'
                data_type = 2;
            case 'Target'
                data_type = 3;
            otherwise
                switch data_type
                    case 1
                        data = sscanf( line, '%f,%d' );
                        speed_data = [ speed_data; data(1) data(2) ];
                    case 2
                        data = sscanf( line, '%f,%d' );
                        thrust_data = [ thrust_data; data(1) data(2) ];
                    case 3
                        data = sscanf( line, '%f,%d' );
                        target_data = [ target_data; data(1) data(2) ];
                end    
        end        
        line = fgets( fid );
    end

    fclose( fid );

end



