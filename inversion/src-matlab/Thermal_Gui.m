function varargout = Thermal_Gui(varargin)
% THERMAL_GUI M-file for Thermal_Gui.fig
%      THERMAL_GUI, by itself, creates a new THERMAL_GUI or raises the existing
%      singleton*.
%
%      H = THERMAL_GUI returns the handle to a new THERMAL_GUI or the handle to
%      the existing singleton*.
%
%      THERMAL_GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in THERMAL_GUI.M with the given input arguments.
%
%      THERMAL_GUI('Property','Value',...) creates a new THERMAL_GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before Thermal_Gui_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to Thermal_Gui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help Thermal_Gui

% Last Modified by GUIDE v2.5 27-Sep-2011 13:41:42

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
    'gui_Singleton',  gui_Singleton, ...
    'gui_OpeningFcn', @Thermal_Gui_OpeningFcn, ...
    'gui_OutputFcn',  @Thermal_Gui_OutputFcn, ...
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


% --- Executes just before Thermal_Gui is made visible.
function Thermal_Gui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to Thermal_Gui (see VARARGIN)

set(handles.VariableTable, 'ColumnFormat', {[] [] [] [] [] [] });



handles.output = hObject;
data=get(handles.VariableTable,'Data');
initialrow=cell(1,size(data,2));
initialrow(1:3) = {'','', ''};
set(handles.VariableTable,'Data',initialrow);

% calibrationData=get(handles.CalibrationTable,'Data');
% row=cell(1,size(calibrationData,2));
% set(handles.CalibrationTable,'Data',row);

calibrationParameters=get(handles.CalibrationParameterTable,'Data');
set(handles.CalibrationParameterTable, 'ColumnFormat', {{'Temperature' 'Vr' 'Depth' 'BulkDensity' 'Pressure'  'Porosity' 'Permeability' } []  [] [] [] [] });

row=cell(1,size(calibrationParameters,2));
row(1:3) = {'','',''};
set(handles.CalibrationParameterTable,'Data',row);


% Choose default command line output for Thermal_Gui
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes Thermal_Gui wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = Thermal_Gui_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



function BaseName_Callback(hObject, eventdata, handles)
% hObject    handle to BaseName (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of BaseName as text
%        str2double(get(hObject,'String')) returns contents of BaseName as a double

baseName=(get(hObject,'String'));
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function BaseName_CreateFcn(hObject, eventdata, handles)
% hObject    handle to BaseName (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function OutputDir_Callback(hObject, eventdata, handles)
% hObject    handle to OutputDir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of OutputDir as text
%        str2double(get(hObject,'String')) returns contents of OutputDir as a double
outputDir=(get(hObject,'String'));
guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function OutputDir_CreateFcn(hObject, eventdata, handles)
% hObject    handle to OutputDir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function LaunchDir_Callback(hObject, eventdata, handles)
% hObject    handle to LaunchDir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of LaunchDir as text
%        str2double(get(hObject,'String')) returns contents of LaunchDir as a double
launchDir=(get(hObject,'String'));

% --- Executes during object creation, after setting all properties.
function LaunchDir_CreateFcn(hObject, eventdata, handles)
% hObject    handle to LaunchDir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Path_Callback(hObject, eventdata, handles)
% hObject    handle to Path (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Path as text
%        str2double(get(hObject,'String')) returns contents of Path as a double
pathProject=(get(hObject,'String'));
guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function Path_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Path (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Reference_Callback(hObject, eventdata, handles)
% hObject    handle to Reference (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Reference as text
%        str2double(get(hObject,'String')) returns contents of Reference as a double




% --- Executes during object creation, after setting all properties.
function Reference_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Reference (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end





function Base_Callback(hObject, eventdata, handles)
% hObject    handle to Base (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Base as text
%        str2double(get(hObject,'String')) returns contents of Base as a double


% --- Executes during object creation, after setting all properties.
function Base_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Base (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Population_Callback(hObject, eventdata, handles)
% hObject    handle to Population (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Population as text
%        str2double(get(hObject,'String')) returns contents of Population as a double


% --- Executes during object creation, after setting all properties.
function Population_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Population (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Generation_Callback(hObject, eventdata, handles)
% hObject    handle to Generation (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Generation as text
%        str2double(get(hObject,'String')) returns contents of Generation as a double


% --- Executes during object creation, after setting all properties.
function Generation_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Generation (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function FWeight_Callback(hObject, eventdata, handles)
% hObject    handle to FWeight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of FWeight as text
%        str2double(get(hObject,'String')) returns contents of FWeight as a double


% --- Executes during object creation, after setting all properties.
function FWeight_CreateFcn(hObject, eventdata, handles)
% hObject    handle to FWeight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function FCr_Callback(hObject, eventdata, handles)
% hObject    handle to FCr (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of FCr as text
%        str2double(get(hObject,'String')) returns contents of FCr as a double


% --- Executes during object creation, after setting all properties.
function FCr_CreateFcn(hObject, eventdata, handles)
% hObject    handle to FCr (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function IStrategy_Callback(hObject, eventdata, handles)
% hObject    handle to IStrategy (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of IStrategy as text
%        str2double(get(hObject,'String')) returns contents of IStrategy as a double


% --- Executes during object creation, after setting all properties.
function IStrategy_CreateFcn(hObject, eventdata, handles)
% hObject    handle to IStrategy (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Launcher_Callback(hObject, eventdata, handles)
% hObject    handle to Launcher (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Launcher as text
%        str2double(get(hObject,'String')) returns contents of Launcher as a double


% --- Executes during object creation, after setting all properties.
function Launcher_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Launcher (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Command_Callback(hObject, eventdata, handles)
% hObject    handle to Command (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Command as text
%        str2double(get(hObject,'String')) returns contents of Command as a double


% --- Executes during object creation, after setting all properties.
function Command_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Command (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end




function CauldronMode_Callback(hObject, eventdata, handles)
% hObject    handle to CauldronMode (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of CauldronMode as text
%        str2double(get(hObject,'String')) returns contents of CauldronMode as a double


% --- Executes during object creation, after setting all properties.
function CauldronMode_CreateFcn(hObject, eventdata, handles)
% hObject    handle to CauldronMode (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Processors_Callback(hObject, eventdata, handles)
% hObject    handle to Processors (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Processors as text
%        str2double(get(hObject,'String')) returns contents of Processors as a double
processors=str2double(get(hObject,'String'));
if isnan(processors)
    set(hObject,'String',2);
    warndlg('Input must be a number, setting number of processors to 2','Warning');
end



% --- Executes during object creation, after setting all properties.
function Processors_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Processors (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Cluster_Callback(hObject, eventdata, handles)
% hObject    handle to Cluster (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Cluster as text
%        str2double(get(hObject,'String')) returns contents of Cluster as a double


% --- Executes during object creation, after setting all properties.
function Cluster_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Cluster (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end




function Option_Callback(hObject, eventdata, handles)
% hObject    handle to Option (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Option as text
%        str2double(get(hObject,'String')) returns contents of Option as a double


% --- Executes during object creation, after setting all properties.
function Option_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Option (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes during object creation, after setting all properties.
function VariableTable_CreateFcn(hObject, eventdata, handles)
% hObject    handle to VariableTable (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called


% --- Executes on button press in AddVariableButton.
function AddVariableButton_Callback(hObject, eventdata, handles)
% hObject    handle to AddVariableButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = get(handles.VariableTable,'data');
row=cell(1,size(data,2));
row(1:3) = {'','',''};
newRowdata = cat(1,data,row);
set(handles.VariableTable,'data',newRowdata);


% --- Executes on button press in AddCalibrationParamButton.
function AddCalibrationParamButton_Callback(hObject, eventdata, handles)
% hObject    handle to AddCalibrationParamButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


data=get(handles.CalibrationParameterTable,'Data');
row=cell(1,size(data,2));
row(1:3) = {'','',''};
newRowdata = cat(1,data,row);
set(handles.CalibrationParameterTable,'Data',newRowdata);


% --- Executes on button press in AddCalibrationPointButton.
function AddCalibrationPointButton_Callback(hObject, eventdata, handles)
% hObject    handle to AddCalibrationPointButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

data = get(handles.CalibrationTable,'data');
row=cell(1,size(data,2));
% newRowdata = cat(1,data,row);

% calibrationData=get(handles.CalibrationTable,'Data');
% row=cell(1,size(calibrationData,2));
% set(handles.CalibrationTable,'Data',row);

paramData=get(handles.CalibrationParameterTable,'Data');
[rows,columns] = size(paramData);
r=0;
for n = 1:rows
    if isempty(paramData{n,5})
        continue;
    end
    r=r+paramData{n,5};
end

row(1) = paramData(1,1);
%set(handles.CalibrationTable,'data',row);
for n = 1:rows
    for m = 1:str2double(paramData{n,5})
        if(m==1 && n==1)
            continue;
        end
        row(1) = paramData(n,1);
        data=get(handles.CalibrationTable,'Data');
        newRowdata = cat(1,data,row);
        set(handles.CalibrationTable,'data',newRowdata);
    end
end


set(handles.CalibrationTable, 'Visible', 'on');


% --- Executes when entered data in editable cell(s) in VariableTable.
function VariableTable_CellEditCallback(hObject, eventdata, handles)
% hObject    handle to VariableTable (see GCBO)
% eventdata  structure with the following fields (see UITABLE)
%	Indices: row and column indices of the cell(s) edited
%	PreviousData: previous data for the cell(s) edited
%	EditData: string(s) entered by the user
%	NewData: EditData or its converted form set on the Data property. Empty if Data was not changed
%	Error: error string when failed to convert EditData to appropriate value for Data
% handles    structure with handles and user data (see GUIDATA)

% columns are numerical
if (eventdata.Indices(2) >= 3)
    tableData = get(hObject, 'data');
    cellvalue=tableData{eventdata.Indices(1), eventdata.Indices(2)};
    if isnan(cellvalue)
        tableData{eventdata.Indices(1), eventdata.Indices(2)} = 0;
        set(hObject, 'data', tableData);
        errordlg('Input must be a number','Error');
    end
end




if (eventdata.Indices(2) == 5)
    if ( isnan(tableData{eventdata.Indices(1), eventdata.Indices(2)-2})  || isnan( tableData{eventdata.Indices(1), eventdata.Indices(2)-1}))
        tableData{eventdata.Indices(1), eventdata.Indices(2)} = eventdata.PreviousData;
        errordlg('Initial value must be within the limits, give min and max values','Error');
    end
    
    
    if ((eventdata.NewData < (tableData{eventdata.Indices(1), eventdata.Indices(2)-2}) ) || (eventdata.NewData > (tableData{eventdata.Indices(1), eventdata.Indices(2)-1}) ) )
        tableData = get(hObject, 'data');
        tableData{eventdata.Indices(1), eventdata.Indices(2)} = 0.5 *( ( tableData{eventdata.Indices(1), eventdata.Indices(2)-1})  + ( tableData{eventdata.Indices(1), eventdata.Indices(2)-2} ) );
        warndlg('Initial value must be within the limits, taking middle value','Warning');
        set(hObject, 'data', tableData);
    end
end

set(hObject, 'CellEditCallback', @VariableTable_CellEditCallback);



function VariableFile_Callback(hObject, eventdata, handles)
% hObject    handle to VariableFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of VariableFile as text
%        str2double(get(hObject,'String')) returns contents of VariableFile as a double


% --- Executes during object creation, after setting all properties.
function VariableFile_CreateFcn(hObject, eventdata, handles)
% hObject    handle to VariableFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function CalibrationParameterFile_Callback(hObject, eventdata, handles)
% hObject    handle to CalibrationParameterFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of CalibrationParameterFile as text
%        str2double(get(hObject,'String')) returns contents of CalibrationParameterFile as a double


% --- Executes during object creation, after setting all properties.
function CalibrationParameterFile_CreateFcn(hObject, eventdata, handles)
% hObject    handle to CalibrationParameterFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function CalibrationWellDataFile_Callback(hObject, eventdata, handles)
% hObject    handle to CalibrationWellDataFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of CalibrationWellDataFile as text
%        str2double(get(hObject,'String')) returns contents of CalibrationWellDataFile as a double


% --- Executes during object creation, after setting all properties.
function CalibrationWellDataFile_CreateFcn(hObject, eventdata, handles)
% hObject    handle to CalibrationWellDataFile (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function Tolerance_Callback(hObject, eventdata, handles)
% hObject    handle to Tolerance (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of Tolerance as text
%        str2double(get(hObject,'String')) returns contents of Tolerance as a double


% --- Executes during object creation, after setting all properties.
function Tolerance_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Tolerance (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



% --- Executes on button press in LoadProject3dButton.
function LoadProject3dButton_Callback(hObject, eventdata, handles)
% hObject    handle to LoadProject3dButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


[filename, pathname] = uigetfile('*.project3d', 'Pick project3d file');
if isequal(filename,0) || isequal(pathname,0)
    disp('User pressed cancel')
else
    disp(['User selected ', fullfile(pathname, filename)])
end

project3d=[pathname filename];
set(handles.Reference,'String',project3d);


% --- Executes on button press in ChooseLaunchDir.
function ChooseLaunchDir_Callback(hObject, eventdata, handles)
% hObject    handle to ChooseLaunchDir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

dir = uigetdir('/scratch/', 'Pick a Directory');
launchdir = [dir '/'];
set(handles.LaunchDir,'String',launchdir);
% --- Executes on button press in ChooseOutputDir.
function ChooseOutputDir_Callback(hObject, eventdata, handles)
% hObject    handle to ChooseOutputDir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

dir = uigetdir('/home/', 'Pick a Directory');
outdir=[dir '/'];
set(handles.OutputDir,'String',outdir);
% --- Executes on button press in LoadVariablesButton.
function LoadVariablesButton_Callback(hObject, eventdata, handles)
% hObject    handle to LoadVariablesButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

[filename, pathname] = uigetfile('*.dat', 'Pick data file');
if isequal(filename,0) || isequal(pathname,0)
    disp('User pressed cancel')
else
    disp(['User selected ', fullfile(pathname, filename)])
end


set(handles.VariableFile,'String',pathname);

% here read file into cell and then populate the table with data
% Open the inputfile
%filename=get(handles.VariableFile,'String');
fid = fopen([pathname filename],'r');
if(fid == -1)
    fprintf(' Cannot read the inputfile \n');
end
format long e;

%initialise row for the table
data = get(handles.VariableTable,'data');
row=cell({});
set(handles.VariableTable,'data',row);


while ~feof(fid)
    line = fgetl(fid);
    if isempty(line) || ~ischar(line),
        continue,
    end
    if( strcmp(line,'STOP')),
        break,
    end %stop reading when end reached
    % skip comment lines
    cellArray = tokenize(line, '" "');
    if(  isempty(cellArray)==false && strcmp(cellArray{1},'#'))
        continue;
    end
    
    data=get(handles.VariableTable,'Data');
    newRowdata = cat(1,data,cellArray);
    set(handles.VariableTable,'data',newRowdata);
end




% --- Executes on button press in LoadParameterButton.
function LoadParameterButton_Callback(hObject, eventdata, handles)
% hObject    handle to LoadParameterButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


[filename, pathname] = uigetfile('*.dat', 'Pick data file');
if isequal(filename,0) || isequal(pathname,0)
    disp('User pressed cancel')
else
    disp(['User selected ', fullfile(pathname, filename)])
end


set(handles.CalibrationParameterFile,'String',pathname);

% here read file into cell and then populate the table with data
% Open the inputfile
%filename=get(handles.VariableFile,'String');
fid = fopen([pathname filename],'r');
if(fid == -1)
    fprintf(' Cannot read the inputfile \n');
end
format long e;

%initialise row for the table
data = get(handles.CalibrationParameterTable,'data');
row=cell({});
set(handles.CalibrationParameterTable,'data',row);


while ~feof(fid)
    line = fgetl(fid);
    if isempty(line) || ~ischar(line),
        continue,
    end
    if( strcmp(line,'STOP')),
        break,
    end %stop reading when end reached
    % skip comment lines
    cellArray = tokenize(line, '" "');
    if(  isempty(cellArray)==false && strcmp(cellArray{1},'#'))
        continue;
    end
    
    data=get(handles.CalibrationParameterTable,'Data');
    newRowdata = cat(1,data,cellArray);
    set(handles.CalibrationParameterTable,'data',newRowdata);
end


% --- Executes on button press in LoadWellDataButton.
function LoadWellDataButton_Callback(hObject, eventdata, handles)
% hObject    handle to LoadWellDataButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)



[filename, pathname] = uigetfile('*.dat', 'Pick well data file');
if isequal(filename,0) || isequal(pathname,0)
    disp('User pressed cancel')
else
    disp(['User selected ', fullfile(pathname, filename)])
end


set(handles.CalibrationWellDataFile,'String',pathname);

% here read file into cell and then populate the table with data
% Open the inputfile
%filename=get(handles.VariableFile,'String');
fid = fopen([pathname filename],'r');
if(fid == -1)
    fprintf(' Cannot read the inputfile \n');
end
format long e;

%initialise row for the table

row=cell({});
set(handles.CalibrationTable,'data',row);



while ~feof(fid)
    line = fgetl(fid);
    if isempty(line) || ~ischar(line),
        continue,
    end
    if( strcmp(line,'STOP')),
        break,
    end %stop reading when end reached
    % skip comment lines
    cellArray = tokenize(line, '" "');
    if(  isempty(cellArray)==false && (strcmp(cellArray{1},'#') || line(1)=='#'))
        continue;
    end
    %add point data
    data=get(handles.CalibrationTable,'Data');
    newRowdata = cat(1,data,cellArray);
    set(handles.CalibrationTable,'data',newRowdata);
    
    %     if( strcmp(line,'BEGIN WELLS'))
    %         %add well file name data
    %         while ~strcmp(line,'END WELLS')
    %             line  = readline(fid);
    %             cellArray = tokenize(line, '" "');
    %             if(isempty(cellArray))
    %                 continue;
    %             end
    %             if( strcmp(cellArray{1},'NUMBEROFWELLS'))
    %                 nWells   = str2double(cellArray{2});
    %                 Input.NWells = nWells;
    %                 for n = 1:nWells
    %                     line  = readline(fid);
    %                     cellArray = tokenize(line, '" "');
    %                     if(isempty(cellArray))
    %                         continue;
    %                     end
    %                     Input.Well(n).Name = cellArray{1};
    %                     %fprintf('Wellname =%s \n',Input.Well(n).Name);
    %                     wellfile=get(handles.WellFileTable,'Data');
    %                     welldata = cat(1,wellfile,cellArray);
    %                     set(handles.WellFileTable,'data',welldata);
    %
    %                     welllist{n} = cellArray{1};
    %                     %set(handles.WellFileListBox,'String',welllist);
    %
    %                 end
    %                 set(handles.WellFileListBox,'String',welllist);
    %             end
    %         end
    %     else
    %         %add point data
    %         data=get(handles.CalibrationTable,'Data');
    %         newRowdata = cat(1,data,cellArray);
    %         set(handles.CalibrationTable,'data',newRowdata);
    %     end
    
end
set(handles.CalibrationTable, 'Visible', 'on');
%set(handles.WellFileTable, 'Visible', 'on');
%set(handles.WellFileListBox,'Visible','on');


% --- Executes on button press in RunButton.
function RunButton_Callback(hObject, eventdata, handles)
% hObject    handle to RunButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)












% ==== Cauldron Calculation Mode ===%
%gets the selected option
%1 'Fully Coupled'
%2 'Hydrostatic Decompaction'
%3 'Hydrostatic Temperature'
%PTCoupled
%HRDecompaction
%LCHRDecompaction
%Decompaction
%LCPressure
%HTemperature
%LCTemperature

calculationMode = get(handles.CalculationModeMenu,'Value');
switch calculationMode
    case 1
        Input.CauldronMode = '-itcoupled';
    case 2
        Input.CauldronMode = '-decompaction';
    case 3
        Input.CauldronMode = '-temperature';
    otherwise
end

% ==== Diff Evolution Control Parameters === %
%create structure for storing input values
Input.Population  = str2double(get(handles.Population,'String'));
Input.Generation = str2double(get(handles.Generation,'String'));
Input.Fweight    = str2double(get(handles.FWeight,'String'));
Input.Fcr = str2double(get(handles.FCr,'String'));
Input.Istrategy = str2double(get(handles.IStrategy,'String'));
Input.Tolerance=str2double(get(handles.Tolerance,'String'));


% basename is identifier to add before to all files
Input.BaseName  = get(handles.BaseName,'String');
Input.LaunchDir  = get(handles.LaunchDir,'String');
%Input.Path  = get(handles.Path,'String');
Input.ReferenceProjectfile    = get(handles.Reference,'String');

%check .project3d for DataIoTable
checkproject3dfile(Input.ReferenceProjectfile);
boundaryModel = checkbottomboundarymodel(Input.ReferenceProjectfile);
if(strcmp(boundaryModel, '"Fixed Temperature"') == 0)
    errordlg('Only Fixed Temperature boundary model is supported');
end
Input.ThermalModel=boundaryModel;
%if this file is OK, then make a clone which will be used for generating
%child files for each variables

project3dclone = [Input.ReferenceProjectfile '.CLONE' ];
cpcommand = ['cp ' Input.ReferenceProjectfile ' ' project3dclone];
system(cpcommand);
%Input.MotherProjectfile    = get(handles.Mother,'String');
Input.MotherProjectfile    =  project3dclone;



% Get Grid Details

[XCoord YCoord NumberX NumberY DeltaX DeltaY] = findgriddetails(Input.ReferenceProjectfile,' ');

Input.GridOriginXCoord = str2double(XCoord);
Input.GridOriginYCoord = str2double(YCoord);
Input.GridNumberX = str2double(NumberX);
Input.GridNumberY = str2double(NumberY);
Input.GridDeltaX = str2double(DeltaX);
Input.GridDeltaY = str2double(DeltaY);

Input.GridXMax = Input.GridOriginXCoord+Input.GridNumberX*Input.GridDeltaX;
Input.GridYMax = Input.GridOriginYCoord+Input.GridNumberY*Input.GridDeltaY;



%Input.ProjectfileBasename    = get(handles.Base,'String');

Input.OutputDir = get(handles.OutputDir,'String');
Input.CauldronLauncher   = get(handles.Launcher,'String');
Input.CauldronCommand   = get(handles.Command,'String');
Input.CauldronOptions   = get(handles.Option,'String');
Input.Cluster   = get(handles.Cluster,'String');
Input.Processors   = str2double(get(handles.Processors,'String'));
%Input.CauldronMode = get(handles.CauldronMode,'String');



% what norm for objective funtion evaluation
if(get(handles.L1NormButton, 'Value') == 1)
    Input.Norm = 'L1';
else
    Input.Norm='L2';
end

% what norm for objective funtion evaluation
if(get(handles.ThermalCalibrationRadioButton, 'Value') == 1)
    Input.CalibrationType = 'Thermal';
else
    Input.CalibrationType='Pressure';
end


%save input data to a file
inputDataFile = [Input.OutputDir Input.BaseName '_ThermalInputData_bkup.dat'];
finput = fopen(inputDataFile,'w');
% save data files
variableFile = [Input.OutputDir Input.BaseName '_ThermalVariables.dat'];
fvar = fopen(variableFile,'w');
% save data files
calibPointFile = [Input.OutputDir Input.BaseName '_ThermalCalibrationPoints.dat'];
fcalibPoint = fopen(calibPointFile,'w');
calibParamFile = [Input.OutputDir Input.BaseName '_ThermalCalibrationParams.dat'];
fcalibParam = fopen(calibParamFile,'w');

% Display all input parameters

%fprintf('path = %s \n', get(handles.Path,'String'));
%fprintf('referenceProjectfile = %s \n',  get(handles.Reference,'String'));
%fprintf('motherProjectfile = %s \n', get(handles.Mother,'String'));


fprintf(finput,'=== Directory information ===\n');
fprintf(finput,'Experiment Name %s\n',Input.BaseName);
fprintf(finput,'Output Dir %s\n',Input.OutputDir);
fprintf(finput,'Launch Dir %s\n',Input.LaunchDir);
fprintf(finput,'Reference Project %s\n',Input.ReferenceProjectfile);
fprintf(finput,'Original Project %s\n',Input.MotherProjectfile);
%fprintf(finput,'Projectfile Base Name %s\n',Input.ProjectfileBasename);

fprintf(finput,'=== DE control variables ===\n');
fprintf(finput,'Population %g\n',Input.Population);
fprintf(finput,'Generation %g\n',Input.Generation);
fprintf(finput,'FWeight %g\n',Input.Fweight);
fprintf(finput,'FCr %g\n',Input.Fcr);
fprintf(finput,'IStrategy %d\n',Input.Istrategy);
fprintf(finput,'Tolerance %g\n',Input.Tolerance);

fprintf(finput,'=== Cauldron Launcher ===\n');
fprintf(finput,'Launcher %s\n',Input.CauldronLauncher);
fprintf(finput,'Command %s\n',Input.CauldronCommand);
fprintf(finput,'Option %s\n',Input.CauldronOptions);
%fprintf(finput,'Cauldron Mode %s\n',Input.CauldronMode);
fprintf(finput,'Processors %d\n',Input.Processors);
fprintf(finput,'Cluster %s\n',Input.Cluster);

% Reading variable data table
tableData = get(handles.VariableTable, 'data');
[rows,columns] = size(tableData);
fprintf(finput,'=== Variables To Optimize ===\n');
fprintf(finput,'%s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\n', 'Type', 'CauldronName', 'VariableName', 'Name', 'PointType', 'NoOfPoints','PointToChange','Min', 'Max', 'Initial');
for n = 1:rows
    Input.InversionParameters(n).MainVariableType=tableData{n, 1} ;
    Input.InversionParameters(n).CauldronName = tableData{n, 2} ;
    Input.InversionParameters(n).Name = tableData{n, 3} ;
    Input.InversionParameters(n).Min = str2double(tableData{n, 4} );
    Input.InversionParameters(n).Max = str2double(tableData{n, 5} );
    Input.InversionParameters(n).Initial = str2double(tableData{n, 6} );
    fprintf('Variable = %s min =%f, max = %f  cauldronName=%s \n',Input.InversionParameters(n).Name,Input.InversionParameters(n).Min,Input.InversionParameters(n).Max, Input.InversionParameters(n).CauldronName );
    fprintf(fvar,'%s\t %s\t %s\t  %g\t %g\t  %g\n',Input.InversionParameters(n).MainVariableType,  Input.InversionParameters(n).CauldronName,Input.InversionParameters(n).Name, ...
        Input.InversionParameters(n).Min,Input.InversionParameters(n).Max,Input.InversionParameters(n).Initial);
    fprintf(finput,'%s\t %s\t %s\t  %g\t %g\t  %g\n',Input.InversionParameters(n).MainVariableType, Input.InversionParameters(n).CauldronName, Input.InversionParameters(n).Name, ...
        Input.InversionParameters(n).Min,Input.InversionParameters(n).Max,Input.InversionParameters(n).Initial);
end

Input.NVar = rows;

fprintf('Total number of variables for inversion = %d \n',Input.NVar);

% Read calibration parameter table %
calibParamData=get(handles.CalibrationParameterTable,'data');
[r,c] = size(calibParamData);
calibPointData = get(handles.CalibrationTable,'data');
Input.NCalibParam = r;
fprintf(finput,'=== Parameter Data ===\n');
for param = 1:r
    fprintf(fcalibParam,'%s\t %s\t %s\t %g\t  %g\n',calibParamData{param,1}, calibParamData{param,2},calibParamData{param,3},str2double(calibParamData{param,4}),str2double(calibParamData{param,5}));
    Input.CalibrationParameters(param).PropertyName = (calibParamData{param,1});
    Input.CalibrationParameters(param).EvaluateOperator = (calibParamData{param,2});
    Input.CalibrationParameters(param).PlotOperator = (calibParamData{param,3});
    Input.CalibrationParameters(param).Weightage = str2double(calibParamData{param,4});
    Input.CalibrationParameters(param).Points = str2double(calibParamData{param,5});
    fprintf(finput,'%s\t %s\t %s\t %g\t  %g\n',calibParamData{param,1}, calibParamData{param,2},calibParamData{param,3},str2double(calibParamData{param,4}),str2double(calibParamData{param,5}));
end

% Read calibration well data points %
fprintf(finput,'=== Calibration Data Points ===\n');
previousPoints=0;
[r,c] = size(calibParamData);
for param = 1:r
    points=str2double(calibParamData{param,5}) ;
    for n = 1:points % for each points
        index=(param-1)*previousPoints+n;
        Input.CalibrationParameters(param).CalibrationData(n).PropertyName  = calibPointData{index,1}; % parameter name
        Input.CalibrationParameters(param).CalibrationData(n).X     = str2double(calibPointData{index,2});
        Input.CalibrationParameters(param).CalibrationData(n).Y     = str2double(calibPointData{index,3});
        Input.CalibrationParameters(param).CalibrationData(n).Z     = str2double(calibPointData{index,4});
        Input.CalibrationParameters(param).CalibrationData(n).Value = str2double(calibPointData{index,5});
        Input.CalibrationParameters(param).CalibrationData(n).WellIndex = str2double(calibPointData{index,6});
        Input.CalibrationParameters(param).CalibrationData(n).WellFileName = (calibPointData{index,7});
        Input.Well(str2double(calibPointData{index,6})).Name = (calibPointData{index,7});
        fprintf('n=%d, param_name = %s, wellindex =%d, value = %g \n',n,Input.CalibrationParameters(param).CalibrationData(n).PropertyName,Input.CalibrationParameters(param).CalibrationData(n).WellIndex, Input.CalibrationParameters(param).CalibrationData(n).Value );
        fprintf(fcalibPoint,'%s\t %g\t %g\t %g\t  %g\t %g\t %s\n', Input.CalibrationParameters(param).CalibrationData(n).PropertyName, Input.CalibrationParameters(param).CalibrationData(n).X, Input.CalibrationParameters(param).CalibrationData(n).Y, ...
            Input.CalibrationParameters(param).CalibrationData(n).Z, Input.CalibrationParameters(param).CalibrationData(n).Value,Input.CalibrationParameters(param).CalibrationData(n).WellIndex, Input.CalibrationParameters(param).CalibrationData(n).WellFileName) ;
        
        fprintf(finput,'%s\t %g\t %g\t %g\t  %g\t %g\t %s\n', Input.CalibrationParameters(param).CalibrationData(n).PropertyName, Input.CalibrationParameters(param).CalibrationData(n).X, Input.CalibrationParameters(param).CalibrationData(n).Y, ...
            Input.CalibrationParameters(param).CalibrationData(n).Z, Input.CalibrationParameters(param).CalibrationData(n).Value,Input.CalibrationParameters(param).CalibrationData(n).WellIndex, Input.CalibrationParameters(param).CalibrationData(n).WellFileName) ;
    end
    previousPoints=points;
end

% number of calibration points

calibPoints = 0;
for param = 1:r
    calibPoints = calibPoints+str2double(calibParamData{param,5});
end
% Check for number of points and calibration point table 
[r,c] = size(calibPointData);
if calibPoints ~= r
    fprintf('Error... check calibration parameter table and point table, number of points are not matching');
    errordlg('Error','Calibration data tables doe not correspond with number of points');
end

wellindexcolumn=   calibPointData(:,6);
Input.NCalib = calibPoints;
Input.NWells = max(str2double(wellindexcolumn));
fclose(fcalibPoint);
fclose(fcalibParam);
fclose(fvar);
fclose(finput);





% No of wells
%no_of_wells = numel(get(handles.WellFileListBox,'String'))

%create dir if not existing
createDir = ['mkdir ' Input.LaunchDir];
system(createDir);

% use the copied files for creating new project3d files. For that modify
% the code for creating .project3d

% %cp reference and mother projects to dir
cpProjects = ['cp ' Input.ReferenceProjectfile ' ' Input.MotherProjectfile ' ' Input.LaunchDir];
system(cpProjects);



%mv to launch directory
launchDir = ['cd ' Input.LaunchDir];
system(launchDir);
hostname=getenv('HOSTNAME');
fprintf('Host machine: %s \n',hostname);
fprintf('Setting up cauldron environment, gocbm3 \n');
% a bash script with setcauldron is made
system('/nfs/rvl/groups/ept-sg/SWEast/Cauldron/nlskui/bin/setcauldron &');



%call rundeopt routine
rundeoptcauldron(Input)


% --- Executes on selection change in CalculationModeMenu.
function CalculationModeMenu_Callback(hObject, eventdata, handles)
% hObject    handle to CalculationModeMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns CalculationModeMenu contents as cell array
%        contents{get(hObject,'Value')} returns selected item from CalculationModeMenu


% --- Executes during object creation, after setting all properties.
function CalculationModeMenu_CreateFcn(hObject, eventdata, handles)
% hObject    handle to CalculationModeMenu (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end
