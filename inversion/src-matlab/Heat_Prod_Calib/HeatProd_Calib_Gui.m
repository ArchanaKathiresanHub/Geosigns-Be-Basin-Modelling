function varargout = HeatProd_Calib_Gui(varargin)
% HEATPROD_CALIB_GUI M-file for HeatProd_Calib_Gui.fig
%      HEATPROD_CALIB_GUI, by itself, creates a new HEATPROD_CALIB_GUI or raises the existing
%      singleton*.
%
%      H = HEATPROD_CALIB_GUI returns the handle to a new HEATPROD_CALIB_GUI or the handle to
%      the existing singleton*.
%
%      HEATPROD_CALIB_GUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in HEATPROD_CALIB_GUI.M with the given input arguments.
%
%      HEATPROD_CALIB_GUI('Property','Value',...) creates a new HEATPROD_CALIB_GUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before HeatProd_Calib_Gui_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to HeatProd_Calib_Gui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help HeatProd_Calib_Gui

% Last Modified by GUIDE v2.5 05-Oct-2011 17:18:41

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
    'gui_Singleton',  gui_Singleton, ...
    'gui_OpeningFcn', @HeatProd_Calib_Gui_OpeningFcn, ...
    'gui_OutputFcn',  @HeatProd_Calib_Gui_OutputFcn, ...
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


% --- Executes just before HeatProd_Calib_Gui is made visible.
function HeatProd_Calib_Gui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to HeatProd_Calib_Gui (see VARARGIN)

set(handles.VariableTable, 'ColumnFormat', {[] [] [] [] {'Temperature' 'Vr'} [] [] [] [] []});
handles.output = hObject;
data=get(handles.VariableTable,'Data');
initialrow=cell(1,size(data,2));
initialrow(1:1) = {''};
initialrow(5:5) = {''};
set(handles.VariableTable,'Data',initialrow);

% calibrationData=get(handles.CalibrationTable,'Data');
% row=cell(1,size(calibrationData,2));
% set(handles.CalibrationTable,'Data',row);

calibrationParameters=get(handles.CalibrationParameterTable,'Data');
set(handles.CalibrationParameterTable, 'ColumnFormat', {{'Temperature' 'Vr' 'Depth' 'BulkDensity' 'Pressure'  'Porosity' 'Permeability' } []  [] [] [] [] });

row=cell(1,size(calibrationParameters,2));
row(1:3) = {'','',''};
set(handles.CalibrationParameterTable,'Data',row);


% Choose default command line output for HeatProd_Calib_Gui
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes HeatProd_Calib_Gui wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = HeatProd_Calib_Gui_OutputFcn(hObject, eventdata, handles)
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



function CauldronVersion_Callback(hObject, eventdata, handles)
% hObject    handle to CauldronVersion (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of CauldronVersion as text
%        str2double(get(hObject,'String')) returns contents of CauldronVersion as a double


% --- Executes during object creation, after setting all properties.
function CauldronVersion_CreateFcn(hObject, eventdata, handles)
% hObject    handle to CauldronVersion (see GCBO)
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

% --- Executes on button press in LoadHDFMapButton.
function LoadHDFMapButton_Callback(hObject, eventdata, handles)
% hObject    handle to LoadHDFMapButton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

[filename, pathname] = uigetfile('*.HDF', 'Pick map file');
if isequal(filename,0) || isequal(pathname,0)
    disp('User pressed cancel')
else
    disp(['User selected ', fullfile(pathname, filename)])
end

hdffile=[pathname filename];
set(handles.MapHDF,'String',hdffile);

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
fid = fopen(filename,'r');
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
    cellArray = tokenize(line, ' ');    
    if(  isempty(cellArray)==true || strcmp(cellArray{1}(1),'#'))
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
fid = fopen(filename,'r');
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
fid = fopen(filename,'r');
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
    if(  isempty(cellArray)==false && strcmp(cellArray{1},'#'))
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


% basename is identifier to add before to all files
    BaseName  = get(handles.BaseName,'String');
    LaunchDir  = get(handles.LaunchDir,'String');
    %create dir if not existing
    createDir = ['mkdir ' LaunchDir];
    system(createDir);
    
    %Input.Path  = get(handles.Path,'String');
    ReferenceProjectfile    = get(handles.Reference,'String');
    %Input.ProjectfileBasename    = get(handles.Base,'String');
    OutputDir = get(handles.OutputDir,'String');
    %Input.CauldronLauncher   = get(handles.CauldronVersion,'String');
    CauldronLauncher   = 'gempis';
    %Input.CauldronCommand   = get(handles.Command,'String');
    CauldronCommand   = 'fastcauldron';
    CauldronOptions   = get(handles.Option,'String');
    Cluster   = get(handles.Cluster,'String');
    Processors   = str2double(get(handles.Processors,'String'));
    CauldronVersion   = get(handles.CauldronVersion,'String');
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
            CauldronMode = '-itcoupled';
        case 2
            CauldronMode = '-decompaction';
        case 3
            CauldronMode = '-temperature';
        otherwise
    end
    % what norm for objective funtion evaluation
    if(get(handles.L1NormButton, 'Value') == 1)
        Norm = 'L1';
    else
        Norm='L2';
    end
    
    
    
     % ==== Diff Evolution Control Parameters === %
    %create structure for storing input values
    Population  = str2double(get(handles.Population,'String'));
    Generation = str2double(get(handles.Generation,'String'));
    Fweight    = str2double(get(handles.FWeight,'String'));
    Fcr = str2double(get(handles.FCr,'String'));
    Istrategy = str2double(get(handles.IStrategy,'String'));
    Tolerance=str2double(get(handles.Tolerance,'String'));
    
    
    
    % thermal or pressure
    % if(get(handles.ThermalCalibrationRadioButton, 'Value') == 1)
    %     Input.CalibrationType = 'Thermal';
    % else
    %     Input.CalibrationType='Pressure';
    % end
    CalibrationType = 'Thermal';
    %check only once

        %check .project3d for DataIoTable
        checkproject3dfile(ReferenceProjectfile);
        boundaryModel = checkbottomboundarymodel(ReferenceProjectfile);
        if(strcmp(boundaryModel, '"Fixed Temperature"') == 0)
            errordlg('Only Fixed Temperature boundary model is supported');
        end
    ThermalModel=boundaryModel;
% Reading variable data table
tableData = get(handles.VariableTable, 'data');
[rows,columns] = size(tableData);
% we need to create small models around each well. 
% no of wells = rows

    
    
    %save input data to a file
    inputDataFile = [OutputDir BaseName '_ThermalInputData_bkup.dat'];
    finput = fopen(inputDataFile,'w');
    % save data files
    variableFile = [OutputDir BaseName '_ThermalVariables.dat'];
    fvar = fopen(variableFile,'w');
    % save data files
    calibPointFile = [OutputDir BaseName '_ThermalCalibrationPoints.dat'];
    fcalibPoint = fopen(calibPointFile,'w');
    calibParamFile = [OutputDir BaseName '_ThermalCalibrationParams.dat'];
    fcalibParam = fopen(calibParamFile,'w');
    % Display all input parameters
    
    fprintf(finput,'=== Directory information ===\n');
    fprintf(finput,'Experiment Name %s\n',BaseName);
    fprintf(finput,'Output Dir %s\n',OutputDir);
    fprintf(finput,'Launch Dir %s\n',LaunchDir);
    fprintf(finput,'Reference Project %s\n',ReferenceProjectfile);
    %fprintf(finput,'Original Project %s\n',Input.MotherProjectfile);
    fprintf(finput,'=== DE control variables ===\n');
    fprintf(finput,'Population %g\n',Population);
    fprintf(finput,'Generation %g\n',Generation);
    fprintf(finput,'FWeight %g\n',Fweight);
    fprintf(finput,'FCr %g\n',Fcr);
    fprintf(finput,'IStrategy %d\n',Istrategy);
    fprintf(finput,'Tolerance %g\n',Tolerance);
    fprintf(finput,'=== Cauldron Launcher ===\n');
    fprintf(finput,'Launcher %s\n',CauldronLauncher);
    fprintf(finput,'Command %s\n',CauldronCommand);
    fprintf(finput,'Option %s\n',CauldronOptions);
    fprintf(finput,'Cauldron Mode %s\n',CauldronMode);
    fprintf(finput,'Processors %d\n',Processors);
    fprintf(finput,'Cluster %s\n',Cluster);     
    % thermal or pressure
    % if(get(handles.ThermalCalibrationRadioButton, 'Value') == 1)
    %     Input.CalibrationType = 'Thermal';
    % else
    %     Input.CalibrationType='Pressure';
    % end
    CalibrationType = 'Thermal';
    %check only once

        %check .project3d for DataIoTable
        checkproject3dfile(ReferenceProjectfile);
        boundaryModel = checkbottomboundarymodel(ReferenceProjectfile);
        if(strcmp(boundaryModel, '"Fixed Temperature"') == 0)
            errordlg('Only Fixed Temperature boundary model is supported');
        end
    ThermalModel=boundaryModel;
% Reading variable data table
tableData = get(handles.VariableTable, 'data');
[rows,columns] = size(tableData);
% we need to create small models around each well. 
% no of wells = rows

for wellmodel = 1: rows
    fprintf(finput,'########### Well Model Data #############\n');
    
     %for each wellmodel, one parameter (heat_prod) to invert
    Input(wellmodel).NVar = 1;
    Input(wellmodel).WellName = tableData{wellmodel, 1};
    Input(wellmodel).X = str2double(tableData{wellmodel, 2} );
    Input(wellmodel).Y = str2double(tableData{wellmodel, 3} );
    Input(wellmodel).Z = str2double(tableData{wellmodel, 4} );
    
    
    
    Input(wellmodel).BaseName  = [BaseName '_' Input(wellmodel).WellName];
    
    Input(wellmodel).LaunchDir = LaunchDir;
    Input(wellmodel).ReferenceProjectfile    = ReferenceProjectfile; 
    Input(wellmodel).OutputDir = OutputDir;
    Input(wellmodel).CauldronLauncher   = CauldronLauncher;
    Input(wellmodel).CauldronCommand   = CauldronCommand;
    Input(wellmodel).CauldronOptions   = CauldronOptions;
    Input(wellmodel).Cluster   = Cluster;
    Input(wellmodel).Processors   = Processors;
    Input(wellmodel).CauldronVersion   = CauldronVersion;
    Input(wellmodel).CauldronMode = CauldronMode;
    Input(wellmodel).Norm =Norm;
    Input(wellmodel).Population  = Population;
    Input(wellmodel).Generation = Generation;
    Input(wellmodel).Fweight    = Fweight;
    Input(wellmodel).Fcr = Fcr;
    Input(wellmodel).Istrategy = Istrategy;
    Input(wellmodel).Tolerance= Tolerance;
    Input(wellmodel).CalibrationType = CalibrationType;
    Input(wellmodel).ThermalModel = ThermalModel;
  
    
    fprintf(finput,'=== Directory information ===\n');
    fprintf(finput,'Experiment Name %s\n',Input(wellmodel).BaseName);
    fprintf(finput,'Output Dir %s\n',Input(wellmodel).OutputDir);
    fprintf(finput,'Launch Dir %s\n',Input(wellmodel).LaunchDir);
    fprintf(finput,'Reference Project %s\n',Input(wellmodel).ReferenceProjectfile);
    %fprintf(finput,'Original Project %s\n',Input(wellmodel).MotherProjectfile);
    fprintf(finput,'=== DE control variables ===\n');
    fprintf(finput,'Population %g\n',Input(wellmodel).Population);
    fprintf(finput,'Generation %g\n',Input(wellmodel).Generation);
    fprintf(finput,'FWeight %g\n',Input(wellmodel).Fweight);
    fprintf(finput,'FCr %g\n',Input(wellmodel).Fcr);
    fprintf(finput,'IStrategy %d\n',Input(wellmodel).Istrategy);
    fprintf(finput,'Tolerance %g\n',Input(wellmodel).Tolerance);
    fprintf(finput,'=== Cauldron Launcher ===\n');
    fprintf(finput,'Launcher %s\n',Input(wellmodel).CauldronLauncher);
    fprintf(finput,'Command %s\n',Input(wellmodel).CauldronCommand);
    fprintf(finput,'Option %s\n',Input(wellmodel).CauldronOptions);
    fprintf(finput,'Cauldron Mode %s\n',Input(wellmodel).CauldronMode);
    fprintf(finput,'Processors %d\n',Input(wellmodel).Processors);
    fprintf(finput,'Cluster %s\n',Input(wellmodel).Cluster);
    
    
    
    
    
    
   
    
    
    % create small project3d files for each wellmodel
    
    Input(wellmodel).WellProject3dFile = createprojectaroundwells(Input(wellmodel).ReferenceProjectfile, Input(wellmodel));
    fprintf('Created model area around well = %s in project3d = %s\n',Input(wellmodel).WellName,Input(wellmodel).WellProject3dFile);
    
    %set new reference project
    Input(wellmodel).ReferenceProjectfile = Input(wellmodel).WellProject3dFile;
     %if this file is OK, then make a clone which will be used for generating
    %child files for each variables
    
         project3dclone = [Input(wellmodel).ReferenceProjectfile '.CLONE' ];
         cpcommand = ['cp ' Input(wellmodel).ReferenceProjectfile ' ' project3dclone];
         system(cpcommand);    
         Input(wellmodel).MotherProjectfile    =  project3dclone;
         
    %for each wellmodel, one parameter (heat_prod) to invert
    for n = 1:1 % only one parameter per well model
        Input(wellmodel).InversionParameters(n).MainVariableType='Basement' ;
        Input(wellmodel).InversionParameters(n).CauldronName = 'TopCrustHeatProd' ;
        Input(wellmodel).InversionParameters(n).Name = 'Heat_Prod' ;
        Input(wellmodel).InversionParameters(n).Min = str2double(tableData{wellmodel, 7} );
        Input(wellmodel).InversionParameters(n).Max = str2double(tableData{wellmodel, 8} );
        Input(wellmodel).InversionParameters(n).Initial = str2double(tableData{wellmodel, 9} );
    end
    
    
   
    


    fprintf('Total number of variables for inversion = %d \n',Input(wellmodel).NVar);
    
    % only one parameter either T or Vr
    Input(wellmodel).NCalibParam = 1;
    fprintf(finput,'=== Parameter Data ===\n');
    for param = 1:Input(wellmodel).NCalibParam
        %fprintf(fcalibParam,'%s\t %s\t %s\t %g\t  %g\n',calibParamData{param,1}, calibParamData{param,2},calibParamData{param,3},str2double(calibParamData{param,4}),str2double(calibParamData{param,5}));
        Input(wellmodel).CalibrationParameters(param).PropertyName = (tableData{wellmodel, 5} );
        Input(wellmodel).CalibrationParameters(param).EvaluateOperator = '1*';
        Input(wellmodel).CalibrationParameters(param).PlotOperator = '1*';
        Input(wellmodel).CalibrationParameters(param).Weightage = 1.0;
        Input(wellmodel).CalibrationParameters(param).Points = 1; % always one point per parameter (per temp or per vr)
        fprintf(finput,'%s\t %s\t %s\t %g\t  %d\n',Input(wellmodel).CalibrationParameters(param).PropertyName, Input(wellmodel).CalibrationParameters(param).EvaluateOperator,...
            Input(wellmodel).CalibrationParameters(param).PlotOperator, Input(wellmodel).CalibrationParameters(param).Weightage, Input(wellmodel).CalibrationParameters(param).Points);
    end
    
    fprintf(finput,'=== Calibration Data Points ===\n');
for param = 1:1 % only one parameter
    points=1;
    for n = 1:points % for each points     
        Input(wellmodel).CalibrationParameters(param).CalibrationData(n).PropertyName  = tableData{wellmodel, 5}; % parameter name
        Input(wellmodel).CalibrationParameters(param).CalibrationData(n).X     = str2double(tableData{wellmodel, 2});
        Input(wellmodel).CalibrationParameters(param).CalibrationData(n).Y     = str2double(tableData{wellmodel, 3});
        Input(wellmodel).CalibrationParameters(param).CalibrationData(n).Z     = str2double(tableData{wellmodel, 4});
        Input(wellmodel).CalibrationParameters(param).CalibrationData(n).Value = str2double(tableData{wellmodel, 6}); % value of T or Vr
        Input(wellmodel).CalibrationParameters(param).CalibrationData(n).WellIndex = 1;
%         Input(wellmodel).CalibrationParameters(param).CalibrationData(n).WellFileName = (calibPointData{index,7});
%         Input(wellmodel).Well(str2double(calibPointData{index,6})).Name = (calibPointData{index,7});       
        %fprintf('n=%d, param_name = %s, wellindex =%d, value = %g \n',n,Input(wellmodel).CalibrationParameters(param).CalibrationData(n).PropertyName,...
         %   Input(wellmodel).CalibrationParameters(param).CalibrationData(n).WellIndex, Input(wellmodel).CalibrationParameters(param).CalibrationData(n).Value );
%         fprintf(fcalibPoint(wellmodel),'%s\t %g\t %g\t %g\t  %g\t %g\t %s\n', Input(wellmodel).CalibrationParameters(param).CalibrationData(n).PropertyName, Input(wellmodel).CalibrationParameters(param).CalibrationData(n).X,...
%             Input(wellmodel).CalibrationParameters(param).CalibrationData(n).Y, Input(wellmodel).CalibrationParameters(param).CalibrationData(n).Z, ...
%             Input(wellmodel).CalibrationParameters(param).CalibrationData(n).Value,Input.CalibrationParameters(param).CalibrationData(n).WellIndex, Input(wellmodel).CalibrationParameters(param).CalibrationData(n).WellFileName) ;       
        %fprintf(finput(wellmodel),'%s\t %g\t %g\t %g\t  %g\t %g\t %s\n', Input.CalibrationParameters(param).CalibrationData(n).PropertyName, Input.CalibrationParameters(param).CalibrationData(n).X, Input.CalibrationParameters(param).CalibrationData(n).Y, ...
         %   Input.CalibrationParameters(param).CalibrationData(n).Z, Input.CalibrationParameters(param).CalibrationData(n).Value,Input.CalibrationParameters(param).CalibrationData(n).WellIndex, Input.CalibrationParameters(param).CalibrationData(n).WellFileName) ;
    end
end

Input(wellmodel).NCalib = 1;
Input(wellmodel).NWells = 1;
    


end


fclose(fcalibPoint);
fclose(fcalibParam);
fclose(fvar);
fclose(finput);


% fprintf(finput,'=== Variables To Optimize ===\n');
% fprintf(finput,'%s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\n', 'Type', 'CauldronName', 'VariableName', 'Name', 'PointType', 'NoOfPoints','PointToChange','Min', 'Max', 'Initial');
% for n = 1:rows
%     Input.InversionParameters(n).MainVariableType=tableData{n, 1} ;
%     Input.InversionParameters(n).CauldronName = tableData{n, 2} ;
%     Input.InversionParameters(n).Name = tableData{n, 3} ;
%     Input.InversionParameters(n).Min = str2double(tableData{n, 4} );
%     Input.InversionParameters(n).Max = str2double(tableData{n, 5} );
%     Input.InversionParameters(n).Initial = str2double(tableData{n, 6} );
%     fprintf('Variable = %s min =%f, max = %f  cauldronName=%s \n',Input.InversionParameters(n).Name,Input.InversionParameters(n).Min,Input.InversionParameters(n).Max, Input.InversionParameters(n).CauldronName );
%     fprintf(fvar,'%s\t %s\t %s\t  %g\t %g\t  %g\n',Input.InversionParameters(n).MainVariableType,  Input.InversionParameters(n).CauldronName,Input.InversionParameters(n).Name, ...
%         Input.InversionParameters(n).Min,Input.InversionParameters(n).Max,Input.InversionParameters(n).Initial);
%     fprintf(finput,'%s\t %s\t %s\t  %g\t %g\t  %g\n',Input.InversionParameters(n).MainVariableType, Input.InversionParameters(n).CauldronName, Input.InversionParameters(n).Name, ...
%         Input.InversionParameters(n).Min,Input.InversionParameters(n).Max,Input.InversionParameters(n).Initial);
% end


% Get Grid Details
% [XCoord YCoord NumberX NumberY DeltaX DeltaY] = findgriddetails(Input.ReferenceProjectfile,' ');
% 
% Input.GridOriginXCoord = str2double(XCoord);
% Input.GridOriginYCoord = str2double(YCoord);
% Input.GridNumberX = str2double(NumberX);
% Input.GridNumberY = str2double(NumberY);
% Input.GridDeltaX = str2double(DeltaX);
% Input.GridDeltaY = str2double(DeltaY);
% 
% Input.GridXMax = Input.GridOriginXCoord+Input.GridNumberX*Input.GridDeltaX;
% Input.GridYMax = Input.GridOriginYCoord+Input.GridNumberY*Input.GridDeltaY;



% % Read calibration parameter table %
% calibParamData=get(handles.CalibrationParameterTable,'data');
% [r,c] = size(calibParamData);
% calibPointData = get(handles.CalibrationTable,'data');
% Input.NCalibParam = r;
% fprintf(finput,'=== Parameter Data ===\n');
% for param = 1:r
%     fprintf(fcalibParam,'%s\t %s\t %s\t %g\t  %g\n',calibParamData{param,1}, calibParamData{param,2},calibParamData{param,3},str2double(calibParamData{param,4}),str2double(calibParamData{param,5}));
%     Input.CalibrationParameters(param).PropertyName = (calibParamData{param,1});
%     Input.CalibrationParameters(param).EvaluateOperator = (calibParamData{param,2});
%     Input.CalibrationParameters(param).PlotOperator = (calibParamData{param,3});
%     Input.CalibrationParameters(param).Weightage = str2double(calibParamData{param,4});
%     Input.CalibrationParameters(param).Points = str2double(calibParamData{param,5});
%     fprintf(finput,'%s\t %s\t %s\t %g\t  %g\n',calibParamData{param,1}, calibParamData{param,2},calibParamData{param,3},str2double(calibParamData{param,4}),str2double(calibParamData{param,5}));
% end

% Read calibration well data points %
% fprintf(finput,'=== Calibration Data Points ===\n');
% previousPoints=0;
% [r,c] = size(calibPointData);
% for param = 1:r
%     points=str2double(calibParamData{param,5}) ;
%     for n = 1:points % for each points
%         index=(param-1)*previousPoints+n;
%         Input.CalibrationParameters(param).CalibrationData(n).PropertyName  = calibPointData{index,1}; % parameter name
%         Input.CalibrationParameters(param).CalibrationData(n).X     = str2double(calibPointData{index,2});
%         Input.CalibrationParameters(param).CalibrationData(n).Y     = str2double(calibPointData{index,3});
%         Input.CalibrationParameters(param).CalibrationData(n).Z     = str2double(calibPointData{index,4});
%         Input.CalibrationParameters(param).CalibrationData(n).Value = str2double(calibPointData{index,5});
%         Input.CalibrationParameters(param).CalibrationData(n).WellIndex = str2double(calibPointData{index,6});
%         Input.CalibrationParameters(param).CalibrationData(n).WellFileName = (calibPointData{index,7});
%         Input.Well(str2double(calibPointData{index,6})).Name = (calibPointData{index,7});
%         fprintf('n=%d, param_name = %s, wellindex =%d, value = %g \n',n,Input.CalibrationParameters(param).CalibrationData(n).PropertyName,Input.CalibrationParameters(param).CalibrationData(n).WellIndex, Input.CalibrationParameters(param).CalibrationData(n).Value );
%         fprintf(fcalibPoint,'%s\t %g\t %g\t %g\t  %g\t %g\t %s\n', Input.CalibrationParameters(param).CalibrationData(n).PropertyName, Input.CalibrationParameters(param).CalibrationData(n).X, Input.CalibrationParameters(param).CalibrationData(n).Y, ...
%             Input.CalibrationParameters(param).CalibrationData(n).Z, Input.CalibrationParameters(param).CalibrationData(n).Value,Input.CalibrationParameters(param).CalibrationData(n).WellIndex, Input.CalibrationParameters(param).CalibrationData(n).WellFileName) ;
%         
%         fprintf(finput,'%s\t %g\t %g\t %g\t  %g\t %g\t %s\n', Input.CalibrationParameters(param).CalibrationData(n).PropertyName, Input.CalibrationParameters(param).CalibrationData(n).X, Input.CalibrationParameters(param).CalibrationData(n).Y, ...
%             Input.CalibrationParameters(param).CalibrationData(n).Z, Input.CalibrationParameters(param).CalibrationData(n).Value,Input.CalibrationParameters(param).CalibrationData(n).WellIndex, Input.CalibrationParameters(param).CalibrationData(n).WellFileName) ;
%     end
%     previousPoints=points;
% end

% % number of calibration points
% calibPoints = 0;
% for param = 1:r
%     calibPoints = calibPoints+str2double(calibParamData{param,5});
% end
% % Check for number of points and calibration point table 
% if calibPoints ~= r
%     fprintf('Error... check calibration parameter table and point table, number of points are not matching');
%     errordlg('Error','Calibration data tables doe not correspond with number of points');
% end
% 
% wellindexcolumn=   calibPointData(:,6);
% Input.NCalib = calibPoints;
% Input.NWells = max(str2double(wellindexcolumn));
% fclose(fcalibPoint);
% fclose(fcalibParam);
% fclose(fvar);
% fclose(finput);


% use the copied files for creating new project3d files. For that modify
% the code for creating .project3d

% %cp reference and mother projects to dir
%cpProjects = ['cp ' Input.ReferenceProjectfile ' ' Input.MotherProjectfile ' ' Input.LaunchDir];
%system(cpProjects);



%mv to launch directory
launchDir = ['cd ' LaunchDir];
system(launchDir);
hostname=getenv('HOSTNAME');
fprintf('Host machine: %s \n',hostname);
fprintf('Setting up cauldron environment, gocbm3 \n');
% a bash script with setcauldron is made
system('/nfs/rvl/groups/ept-sg/SWEast/Cauldron/nlskui/bin/setcauldron &');


for wellmodel = 1: rows
%call rundeopt routine
rundeoptcauldron(Input(wellmodel));
end

%create a list to launch


