function varargout = aoBuffer(varargin)
% Graphical user interface to setup aoBuffers
% the buffer setup data is stored in a local variable called aoBufferArray


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004



gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @aoBuffer_OpeningFcn, ...
                   'gui_OutputFcn',  @aoBuffer_OutputFcn, ...
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


% --- Executes just before aoVariable is made visible.
function aoBuffer_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to aoVariable (see VARARGIN)

% Choose default command line output for aoVariable
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);
if (nargin==4)
    var=varargin{1};
    updateGui(handles,var);
end;
% UIWAIT makes aoVariable wait for user response (see UIRESUME)
%uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = aoBuffer_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure

if nargout
    uiwait(handles.figure1);
    if(ishandle(hObject))
        data=guidata(hObject);
        varargout{1} = data.var;
        close(handles.figure1);
    else
        varargout={[]};
    end;
else
%     set(handles.cancel,'Enable','off');
      set(handles.ok,'Enable','off');
end;



function name_Callback(hObject, eventdata, handles)
% hObject    handle to name (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of name as text
%        str2double(get(hObject,'String')) returns contents of name as a double


% --- Executes during object creation, after setting all properties.
function name_CreateFcn(hObject, eventdata, handles)
% hObject    handle to name (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function memPointer_Callback(hObject, eventdata, handles)
% hObject    handle to memPointer (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of memPointer as text
%        str2double(get(hObject,'String')) returns contents of memPointer as a double


% --- Executes during object creation, after setting all properties.
function memPointer_CreateFcn(hObject, eventdata, handles)
% hObject    handle to memPointer (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function description_Callback(hObject, eventdata, handles)
% hObject    handle to description (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of description as text
%        str2double(get(hObject,'String')) returns contents of description as a double


% --- Executes during object creation, after setting all properties.
function description_CreateFcn(hObject, eventdata, handles)
% hObject    handle to description (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end




function nrItem_Callback(hObject, eventdata, handles)
% hObject    handle to nrItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of nrItem as text
%        str2double(get(hObject,'String')) returns contents of nrItem as a double


% --- Executes during object creation, after setting all properties.
function nrItem_CreateFcn(hObject, eventdata, handles)
% hObject    handle to nrItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on button press in readTest.
function readTest_Callback(hObject, eventdata, handles)
% hObject    handle to readTest (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)



function edit5_Callback(hObject, eventdata, handles)
% hObject    handle to edit5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit5 as text
%        str2double(get(hObject,'String')) returns contents of edit5 as a double


% --- Executes during object creation, after setting all properties.
function edit5_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on button press in ok.
function ok_Callback(hObject, eventdata, handles)
% hObject    handle to ok (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=getGuiVar(handles);
data=guidata(handles.figure1);
data.var=var;
guidata(handles.figure1,data);
uiresume(handles.figure1);

% --- Executes on button press in cancel.
function cancel_Callback(hObject, eventdata, handles)
% hObject    handle to cancel (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
close(handles.figure1);



function triggerPointer_Callback(hObject, eventdata, handles)
% hObject    handle to triggerPointer (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of triggerPointer as text
%        str2double(get(hObject,'String')) returns contents of triggerPointer as a double


% --- Executes during object creation, after setting all properties.
function triggerPointer_CreateFcn(hObject, eventdata, handles)
% hObject    handle to triggerPointer (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function triggerMask_Callback(hObject, eventdata, handles)
% hObject    handle to triggerMask (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of triggerMask as text
%        str2double(get(hObject,'String')) returns contents of triggerMask as a double


% --- Executes during object creation, after setting all properties.
function triggerMask_CreateFcn(hObject, eventdata, handles)
% hObject    handle to triggerMask (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function triggerValue_Callback(hObject, eventdata, handles)
% hObject    handle to triggerValue (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of triggerValue as text
%        str2double(get(hObject,'String')) returns contents of triggerValue as a double


% --- Executes during object creation, after setting all properties.
function triggerValue_CreateFcn(hObject, eventdata, handles)
% hObject    handle to triggerValue (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on selection change in triggerCompare.
function triggerCompare_Callback(hObject, eventdata, handles)
% hObject    handle to triggerCompare (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns triggerCompare contents as cell array
%        contents{get(hObject,'Value')} returns selected item from triggerCompare


% --- Executes during object creation, after setting all properties.
function triggerCompare_CreateFcn(hObject, eventdata, handles)
% hObject    handle to triggerCompare (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end
AO_BUFFER_DEFINE();
set(hObject,'String',{COMPARE_TYPE.name});




function dspPointer_Callback(hObject, eventdata, handles)
% hObject    handle to dspPointer (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of dspPointer as text
%        str2double(get(hObject,'String')) returns contents of dspPointer as a double


% --- Executes during object creation, after setting all properties.
function dspPointer_CreateFcn(hObject, eventdata, handles)
% hObject    handle to dspPointer (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function edit11_Callback(hObject, eventdata, handles)
% hObject    handle to nrItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of nrItem as text
%        str2double(get(hObject,'String')) returns contents of nrItem as a double


% --- Executes during object creation, after setting all properties.
function edit11_CreateFcn(hObject, eventdata, handles)
% hObject    handle to nrItem (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function edit12_Callback(hObject, eventdata, handles)
% hObject    handle to edit12 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit12 as text
%        str2double(get(hObject,'String')) returns contents of edit12 as a double


% --- Executes during object creation, after setting all properties.
function edit12_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit12 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on selection change in popupmenu2.
function popupmenu2_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmenu2 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu2


% --- Executes during object creation, after setting all properties.
function popupmenu2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on selection change in triggerDsp.
function triggerDsp_Callback(hObject, eventdata, handles)
% hObject    handle to triggerDsp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns triggerDsp contents as cell array
%        contents{get(hObject,'Value')} returns selected item from triggerDsp


% --- Executes during object creation, after setting all properties.
function triggerDsp_CreateFcn(hObject, eventdata, handles)
% hObject    handle to triggerDsp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end
AO_BUFFER_DEFINE();
set(hObject,'String',{DSP_NUMBERS});




% --- Executes on selection change in dsp.
function dsp_Callback(hObject, eventdata, handles)
% hObject    handle to dsp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns dsp contents as cell array
%        contents{get(hObject,'Value')} returns selected item from dsp


% --- Executes during object creation, after setting all properties.
function dsp_CreateFcn(hObject, eventdata, handles)
% hObject    handle to dsp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end
AO_BUFFER_DEFINE();
set(hObject,'String',{DSP_NUMBERS});




function len_Callback(hObject, eventdata, handles)
% hObject    handle to len (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of len as text
%        str2double(get(hObject,'String')) returns contents of len as a double


% --- Executes during object creation, after setting all properties.
function len_CreateFcn(hObject, eventdata, handles)
% hObject    handle to len (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function decFactor_Callback(hObject, eventdata, handles)
% hObject    handle to decFactor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of decFactor as text
%        str2double(get(hObject,'String')) returns contents of decFactor as a double


% --- Executes during object creation, after setting all properties.
function decFactor_CreateFcn(hObject, eventdata, handles)
% hObject    handle to decFactor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function data_Callback(hObject, eventdata, handles)
% hObject    handle to data (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of data as text
%        str2double(get(hObject,'String')) returns contents of data as a double


% --- Executes during object creation, after setting all properties.
function data_CreateFcn(hObject, eventdata, handles)
% hObject    handle to data (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on button press in circular.
function circular_Callback(hObject, eventdata, handles)
% hObject    handle to circular (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of circular



function sdramPointerValue_Callback(hObject, eventdata, handles)
% hObject    handle to sdramPointerValue (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of sdramPointerValue as text
%        str2double(get(hObject,'String')) returns contents of sdramPointerValue as a double


% --- Executes during object creation, after setting all properties.
function sdramPointerValue_CreateFcn(hObject, eventdata, handles)
% hObject    handle to sdramPointerValue (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on selection change in triggerDataType.
function triggerDataType_Callback(hObject, eventdata, handles)
% hObject    handle to triggerDataType (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns triggerDataType contents as cell array
%        contents{get(hObject,'Value')} returns selected item from triggerDataType


% --- Executes during object creation, after setting all properties.
function triggerDataType_CreateFcn(hObject, eventdata, handles)
% hObject    handle to triggerDataType (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end
AO_VARIABLE_DEFINE();
set(hObject,'String',{varTypeArray.name});



% --- Executes on selection change in dataType.
function dataType_Callback(hObject, eventdata, handles)
% hObject    handle to dataType (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns dataType contents as cell array
%        contents{get(hObject,'Value')} returns selected item from dataType


% --- Executes during object creation, after setting all properties.
function dataType_CreateFcn(hObject, eventdata, handles)
% hObject    handle to dataType (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end
AO_VARIABLE_DEFINE();
set(hObject,'String',{varTypeArray.name});


% --- Executes on selection change in bufferNumber.
function bufferNumber_Callback(hObject, eventdata, handles)
% hObject    handle to bufferNumber (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns bufferNumber contents as cell array
%        contents{get(hObject,'Value')} returns selected item from bufferNumber


% --- Executes during object creation, after setting all properties.
function bufferNumber_CreateFcn(hObject, eventdata, handles)
% hObject    handle to bufferNumber (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end
AO_BUFFER_DEFINE();
set(hObject,'String',{BUFFER_NUMBERS});



% --- Executes on button press in readSetup.
function readSetup_Callback(hObject, eventdata, handles)
% hObject    handle to readSetup (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=getGuiVar(handles);
var=aoBufferReadSetup(var.bufferNumber,var.triggerDataType,var.dataType,var.firstDsp,var.connectionNr);
updateGui(handles,var);


% --- Executes on button press in writeSetup.
function writeSetup_Callback(hObject, eventdata, handles)
% hObject    handle to writeSetup (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=getGuiVar(handles);
aoBufferWriteSetup(var,var.connectionNr);

function updateGui(handles,var)
    if isfield(var,'bufferName');
        set(handles.bufferName,'String',var.bufferName);
    end;
    set(handles.triggerPointer,'String',var.triggerPointer);
    set(handles.triggerDataType,'Value',var.triggerDataType);
    set(handles.triggerDsp,'Value',var.triggerDsp+1);
    set(handles.triggerMask,'String',var.triggerMask);
    set(handles.triggerValue,'String',var.triggerValue);
    set(handles.triggerCompare,'Value',var.triggerCompare+1);

    set(handles.dspPointer,'String',var.dspPointer);
    set(handles.dspPointer,'String',var.dspPointer);
    set(handles.nrItem,'String',var.nrItem);
    set(handles.dataType,'Value',var.dataType);
    set(handles.dsp,'Value',var.dsp+1);
    set(handles.len,'String',var.len);
    set(handles.decFactor,'String',var.decFactor);
    set(handles.sdramPointerValue,'String',var.sdramPointerValue);
    if(var.direction==1)
        set(handles.sdramDsp,'Value',1);
    else
        set(handles.dspSdram,'Value',1);
    end;        
    set(handles.circular,'Value',var.circular);
    set(handles.bufferNumber,'Value',var.bufferNumber);
    set(handles.firstDsp,'Value',var.firstDsp+1);
    set(handles.lastDsp,'Value',var.lastDsp+1);


function var=getGuiVar(handles)
    var.bufferName=get(handles.bufferName,'String');
    var.triggerPointer=get(handles.triggerPointer,'String');
    var.triggerDataType=get(handles.triggerDataType,'Value');
    var.triggerDsp=get(handles.triggerDsp,'Value')-1;
    var.triggerMask=get(handles.triggerMask,'String');
    var.triggerValue=get(handles.triggerValue,'String');
    var.triggerCompare=get(handles.triggerCompare,'Value')-1;

    var.dspPointer=get(handles.dspPointer,'String');
    var.dspPointer=get(handles.dspPointer,'String');
    var.nrItem=get(handles.nrItem,'String');
    var.dataType=get(handles.dataType,'Value');
    var.dsp=get(handles.dsp,'Value')-1;
    var.len=get(handles.len,'String');
    var.decFactor=get(handles.decFactor,'String');
    var.sdramPointerValue=get(handles.sdramPointerValue,'String');
    var.direction=get(handles.sdramDsp,'Value');
    var.circular=get(handles.circular,'Value');
    var.bufferNumber=get(handles.bufferNumber,'Value');
    var.firstDsp=get(handles.firstDsp,'Value')-1;
    var.lastDsp=get(handles.lastDsp,'Value')-1;
    var.connectionNr=get(handles.connectionNr,'Value');
    




% --------------------------------------------------------------------
function Untitled_3_Callback(hObject, eventdata, handles)
% hObject    handle to Untitled_3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function VARIABLE_Callback(hObject, eventdata, handles)
% hObject    handle to VARIABLE (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function selectTriggerVar_Callback(hObject, eventdata, handles)
% hObject    handle to selectTriggerVar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=aoGetVar();
if isempty(var);
    return;
end;
set(handles.triggerPointer,'string',var.name);
set(handles.triggerDataType,'value',var.type);




% --------------------------------------------------------------------
function TIGGER_VARIBLE_Callback(hObject, eventdata, handles)
% hObject    handle to TIGGER_VARIBLE (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function selectDataVar_Callback(hObject, eventdata, handles)
% hObject    handle to selectDataVar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=aoGetVar();
if isempty(var);
    return;
end;
set(handles.dspPointer,'string',var.name);
set(handles.nrItem,'string',num2str(var.nrItem));
set(handles.dataType,'value',var.type);



% --- Executes during object creation, after setting all properties.
function bufferList_CreateFcn(hObject, eventdata, handles)
% hObject    handle to bufferList (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end

% --- Executes on selection change in bufferList.
function bufferList_Callback(hObject, eventdata, handles)
% hObject    handle to bufferList (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns bufferList contents as cell array
%        contents{get(hObject,'Value')} returns selected item from bufferList
persistent old_index;
index=min(get(hObject,'value'));
if isempty(index)
    old_index=index;
    return
end;
if isequal(index,old_index)||isempty(index);
    return;
end;
if ~isempty(old_index)
    var=getGuiVar(handles);
    bufferArray=evalin('base','aoBufferArray');
    bufferArray(old_index)=var;
    set(handles.bufferList,'String',{bufferArray.bufferName});
    assignin('base','aoBufferArray',bufferArray);
end;
var=evalin('base',['aoBufferArray(',num2str(index),')']);
updateGui(handles,var);
old_index=index;

% --- Executes on button press in addBuffer.
function addBuffer_Callback(hObject, eventdata, handles)
% hObject    handle to addBuffer (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=getGuiVar(handles);
try
    bufferArray=evalin('base','aoBufferArray');
    index=min(get(handles.bufferList,'value'));
    bufferArray(end+1)=var;
catch
    bufferArray=var;
end;
set(handles.bufferList,'String',{bufferArray.bufferName});
set(handles.bufferList,'Value',length(bufferArray));
assignin('base','aoBufferArray',bufferArray);
bufferList_Callback(handles.bufferList, eventdata, handles)

function bufferName_Callback(hObject, eventdata, handles)
% hObject    handle to bufferName (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of bufferName as text
%        str2double(get(hObject,'String')) returns contents of bufferName as a double


% --- Executes during object creation, after setting all properties.
function bufferName_CreateFcn(hObject, eventdata, handles)
% hObject    handle to bufferName (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --------------------------------------------------------------------
function bufferListDelete_Callback(hObject, eventdata, handles)
% hObject    handle to bufferListDelete (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
bufferArray=evalin('base','aoBufferArray');
selection=get(handles.bufferList,'value');
bufferArray(selection)=[];
set(handles.bufferList,'value',[]);
set(handles.bufferList,'String',{bufferArray.bufferName});
assignin('base','aoBufferArray',bufferArray);
bufferList_Callback(handles.bufferList, eventdata, handles)





% --------------------------------------------------------------------
function bufferListRefresh_Callback(hObject, eventdata, handles)
% hObject    handle to bufferListRefresh (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
bufferArray=evalin('base','aoBufferArray');
set(handles.bufferList,'String',{bufferArray.bufferName});
set(handles.bufferList,'value',[]);
bufferList_Callback(handles.bufferList, eventdata, handles)


% --------------------------------------------------------------------
function bufferListWriteSetup_Callback(hObject, eventdata, handles)
% hObject    handle to bufferListWriteSetup (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
bufferArray=evalin('base','aoBufferArray');
selection=get(handles.bufferList,'value');
aoBufferWriteSetup(bufferArray(selection));


% --- Executes on key press over bufferList with no controls selected.
function bufferList_KeyPressFcn(hObject, eventdata, handles)
% hObject    handle to bufferList (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on selection change in firstDsp.
function firstDsp_Callback(hObject, eventdata, handles)
% hObject    handle to firstDsp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns firstDsp contents as cell array
%        contents{get(hObject,'Value')} returns selected item from firstDsp


% --- Executes during object creation, after setting all properties.
function firstDsp_CreateFcn(hObject, eventdata, handles)
% hObject    handle to firstDsp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in lastDsp.
function lastDsp_Callback(hObject, eventdata, handles)
% hObject    handle to lastDsp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns lastDsp contents as cell array
%        contents{get(hObject,'Value')} returns selected item from lastDsp


% --- Executes during object creation, after setting all properties.
function lastDsp_CreateFcn(hObject, eventdata, handles)
% hObject    handle to lastDsp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


