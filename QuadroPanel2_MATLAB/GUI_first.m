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
arr_index = 1;
% receive_time_str = get(handles.rcv_time, 'String');
% receive_time = str2double(receive_time_str);
set(hObject,'Visible', 'off');
log_fname = '/home/alex/Serial.log';
log_fileID = fopen( log_fname,'r');
elapsed_time = 0;
sens_time = 2.5/1000;
while ~feof(log_fileID)
    read_line = fgetl(log_fileID);
    quadro_data = sscanf( read_line, '#S%d#R:%d#I:%d' );
    handles.sens_angle(arr_index) = quadro_data(1)/1000;
    handles.real_angle(arr_index) = quadro_data(2);
    handles.input(arr_index) = quadro_data(3)/1000;

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
pitch_plot = subplot( 1, 1, 1, 'Parent', handles.plots_panel );
% roll_plot = subplot( 2, 1, 2, 'Parent', handles.plots_panel );
cla(pitch_plot);
% cla(roll_plot);

int_ = 1:numel(handles.time);
plot( pitch_plot, handles.time(int_), handles.real_angle(int_)/adc_coeff, 'BLACK', 'LineWidth', 2 ); hold on;
plot( pitch_plot, handles.time(int_), handles.sens_angle(int_), 'LineWidth', 2 );
plot( pitch_plot, handles.time(int_), handles.input(int_), 'RED', 'LineWidth', 2 ); hold off;
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
