function varargout = GUI_first(varargin)
% GUI_FIRST MATLAB code for GUI_first.fig
%      GUI_FIRST, by itself, creates a new GUI_FIRST or raises the existing
%      singleton*.
%
%      H = GUI_FIRST returns the handle to a new GUI_FIRST or the handle to
%      the existing singleton*.
%
%      GUI_FIRST('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in GUI_FIRST.M with the given input arguments.
%
%      GUI_FIRST('Property','Value',...) creates a new GUI_FIRST or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before GUI_first_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to GUI_first_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help GUI_first

% Last Modified by GUIDE v2.5 03-Nov-2015 19:48:02

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @GUI_first_OpeningFcn, ...
                   'gui_OutputFcn',  @GUI_first_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT

% --- Executes just before GUI_first is made visible.
function GUI_first_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to GUI_first (see VARARGIN)

% Choose default command line output for GUI_first
handles.output = hObject;
% Update handles structure
guidata(hObject, handles);

% UIWAIT makes GUI_first wait for user response (see UIRESUME)
% uiwait(handles.figure1);

% --- Outputs from this function are returned to the command line.
function varargout = GUI_first_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;

% --- Executes on button press in rcv_btn.
function rcv_btn_Callback(hObject, eventdata, handles)
% hObject    handle to rcv_btn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
arr_index = 2;
GYRO_SENSITIVITY = 250; %Get it from settings of gyroscope
RADIANS_TO_DEGREES = 180/3.14159;
% receive_time_str = get(handles.rcv_time, 'String');
% receive_time = str2double(receive_time_str);
set(hObject,'Visible', 'off');
log_fname = '/home/alex/Serial_str.log';
log_fileID = fopen( log_fname,'r');
elapsed_time = 0;
while ~feof(log_fileID)
    read_line = fgetl(log_fileID);
    quadro_data = sscanf( read_line, '#T:%d#G:%d,%d,%d#A:%d,%d,%d#R:%d' );
    sens_time = quadro_data(1)/1000000;
    x_gyr_raw = quadro_data(2);
    y_gyr_raw = quadro_data(3);
    z_gyr_raw = quadro_data(4);
    x_acc_raw = quadro_data(5);
    y_acc_raw = quadro_data(6);
    z_acc_raw = quadro_data(7);
    handles.real_angle(arr_index) = quadro_data(8);
    
    handles.accel_angle_x(arr_index) = atan(y_acc_raw / sqrt(x_acc_raw^2 + z_acc_raw^2)) * RADIANS_TO_DEGREES;
    handles.accel_angle_y(arr_index) = atan(-1 * x_acc_raw / sqrt(y_acc_raw^2 + z_acc_raw^2)) * RADIANS_TO_DEGREES;
    
    GYR_SEN = 65535/2/GYRO_SENSITIVITY;
    handles.gyro_delta_x(arr_index) = (x_gyr_raw/GYR_SEN) * sens_time;
    handles.gyro_delta_y(arr_index) = (y_gyr_raw/GYR_SEN) * sens_time;
    handles.gyro_delta_z(arr_index) = (z_gyr_raw/GYR_SEN) * sens_time;
    
    %50 Hz transmition T = 20 ms
    handles.time(arr_index) = elapsed_time;
    set(handles.data2, 'String', elapsed_time);
    elapsed_time = elapsed_time + sens_time;
    arr_index = arr_index + 1;
    drawnow();
end
fclose(log_fileID);
set(hObject,'Visible', 'on');
set(handles.show_data_btn,'Visible', 'on');
guidata(hObject, handles);



% --- Executes on button press in show_data_btn.
function show_data_btn_Callback(hObject, eventdata, handles)
% hObject    handle to show_data_btn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
last_angle_x = 0;
last_angle_y = 0;
last_angle_z = 0;
last_gyro_angle_x = 0;
last_gyro_angle_y = 0;
% last_gyro_angle_z = 0;
alpha_str = get(handles.alpha, 'String');
alpha = str2double(alpha_str);
ymax_str = get(handles.y_max, 'String');
ymax = str2double(ymax_str);
ymin_str = get(handles.y_min, 'String');
ymin = str2double(ymin_str);
tmax_str = get(handles.t_max, 'String');
tmax = str2double(tmax_str);
tmin_str = get(handles.t_min, 'String');
tmin = str2double(tmin_str);
adc_coeff_str = get(handles.rcv_time, 'String');
adc_coeff = str2double(adc_coeff_str);
if ymax <= ymin
   error('Bad region'); 
end
angle_pitch = [0];
angle_roll = [0];
angle_gyro_pitch = [0];
angle_gyro_roll = [0];
pitch_plot = subplot( 1, 1, 1, 'Parent', handles.plots_panel );
% roll_plot = subplot( 2, 1, 2, 'Parent', handles.plots_panel );
cla(pitch_plot);
% cla(roll_plot);
for i = 1:numel(handles.time)
    angle_gyro_pitch(i) = handles.gyro_delta_x(i) + last_gyro_angle_x;
    angle_gyro_roll(i) = handles.gyro_delta_y(i) + last_gyro_angle_y;
    
    last_gyro_angle_x = angle_gyro_pitch(i);
    last_gyro_angle_y = angle_gyro_roll(i);
    
    angle_x = alpha * (handles.gyro_delta_x(i) + last_angle_x) + (1.0 - alpha) * handles.accel_angle_x(i);
    angle_y = alpha * (handles.gyro_delta_y(i) + last_angle_y) + (1.0 - alpha) * handles.accel_angle_y(i);
    angle_z = handles.gyro_delta_z(i) + last_angle_z;
    
    angle_pitch(i) = angle_x;
    angle_roll(i) = angle_y;
    
    last_angle_x = angle_x;
    last_angle_y = angle_y;
    last_angle_z = angle_z;
end

int_ = 1:numel(handles.time);
plot( pitch_plot, handles.time(int_), handles.accel_angle_x(int_), 'RED' ); hold(pitch_plot, 'on'); 
plot( pitch_plot, handles.time(int_), angle_gyro_pitch(int_), 'GREEN' );
plot( pitch_plot, handles.time(int_), handles.real_angle(int_)/adc_coeff, 'BLACK', 'LineWidth', 2 );
plot( pitch_plot, handles.time(int_), angle_pitch(int_), 'LineWidth', 2 ); hold off;
title( pitch_plot, 'Pitch' );
axis( pitch_plot, [tmin tmax ymin ymax]);
grid(pitch_plot, 'on');

% plot( roll_plot, handles.time(int_), handles.accel_angle_y(int_), 'RED' ); hold(roll_plot, 'on');
% plot( roll_plot, handles.time(int_), angle_gyro_roll(int_), 'GREEN' );
% plot( roll_plot, handles.time(int_), angle_roll(int_), 'LineWidth', 2 ); hold off;
% title( roll_plot, 'Roll' );
% axis( roll_plot, [tmin tmax ymin ymax]);
% grid(roll_plot, 'on');


% --------------------------------------------------------------------
function FileMenu_Callback(hObject, eventdata, handles)
% hObject    handle to FileMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function OpenMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to OpenMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
file = uigetfile('*.fig');
if ~isequal(file, 0)
    open(file);
end

% --------------------------------------------------------------------
function PrintMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to PrintMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
printdlg(handles.figure1)

% --------------------------------------------------------------------
function CloseMenuItem_Callback(hObject, eventdata, handles)
% hObject    handle to CloseMenuItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
selection = questdlg(['Close ' get(handles.figure1,'Name') '?'],...
                     ['Close ' get(handles.figure1,'Name') '...'],...
                     'Yes','No','Yes');
if strcmp(selection,'No')
    return;
end

delete(handles.figure1)


function alpha_Callback(hObject, eventdata, handles)
% hObject    handle to alpha (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of alpha as text
%        str2double(get(hObject,'String')) returns contents of alpha as a double


% --- Executes during object creation, after setting all properties.
function alpha_CreateFcn(hObject, eventdata, handles)
% hObject    handle to alpha (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function y_max_Callback(hObject, eventdata, handles)
% hObject    handle to y_max (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of y_max as text
%        str2double(get(hObject,'String')) returns contents of y_max as a double


% --- Executes during object creation, after setting all properties.
function y_max_CreateFcn(hObject, eventdata, handles)
% hObject    handle to y_max (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function rcv_time_Callback(hObject, eventdata, handles)
% hObject    handle to rcv_time (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of rcv_time as text
%        str2double(get(hObject,'String')) returns contents of rcv_time as a double


% --- Executes during object creation, after setting all properties.
function rcv_time_CreateFcn(hObject, eventdata, handles)
% hObject    handle to rcv_time (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function y_min_Callback(hObject, eventdata, handles)
% hObject    handle to y_min (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of y_min as text
%        str2double(get(hObject,'String')) returns contents of y_min as a double


% --- Executes during object creation, after setting all properties.
function y_min_CreateFcn(hObject, eventdata, handles)
% hObject    handle to y_min (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function t_min_Callback(hObject, eventdata, handles)
% hObject    handle to t_min (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of t_min as text
%        str2double(get(hObject,'String')) returns contents of t_min as a double


% --- Executes during object creation, after setting all properties.
function t_min_CreateFcn(hObject, eventdata, handles)
% hObject    handle to t_min (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function t_max_Callback(hObject, eventdata, handles)
% hObject    handle to t_max (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of t_max as text
%        str2double(get(hObject,'String')) returns contents of t_max as a double


% --- Executes during object creation, after setting all properties.
function t_max_CreateFcn(hObject, eventdata, handles)
% hObject    handle to t_max (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

function data2_Callback(hObject, eventdata, handles)
% hObject    handle to data2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of data2 as text
%        str2double(get(hObject,'String')) returns contents of data2 as a double


% --- Executes during object creation, after setting all properties.
function data2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to data2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
