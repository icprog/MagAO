function varargout = aoVariable(varargin)
% M-file for aoVariable.fig
%      AOVARIABLE, by itself, creates a new AOVARIABLE or raises the existing
%      singleton*.
% usage is:
%      [A] = AOVARIABLE([A])
%      A is a AO variable struct
%

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004



gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @aoVariable_OpeningFcn, ...
                   'gui_OutputFcn',  @aoVariable_OutputFcn, ...
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
function aoVariable_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to aoVariable (see VARARGIN)

% Choose default command line output for aoVariable
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);
if (nargin>=4)
    if(ischar(varargin{1}));
        var=aoGetVar(varargin{1});
    else
        var=varargin{1};
    end;
    setVar(var,handles);
end;
% UIWAIT makes aoVariable wait for user response (see UIRESUME)
%uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = aoVariable_OutputFcn(hObject, eventdata, handles) 
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
        if nargout==2
            varargout{2}=handles.figure1;
        else
            close(handles.figure1);
        end;
    else
        if nargout==2
            varargout{2}=[];
        end;
            varargout{1}=[];
    end;
else
    set(handles.cancel,'Enable','off');
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




% --- Executes during object creation, after setting all properties.
function type_CreateFcn(hObject, eventdata, handles)
% hObject    handle to type (see GCBO)
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
var=getVar(handles);
x=aoRead(var);
set(handles.readResult,'String',num2str(x'));



function readResult_Callback(hObject, eventdata, handles)
% hObject    handle to readResult (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of readResult as text
%        str2double(get(hObject,'String')) returns contents of readResult as a double


% --- Executes during object creation, after setting all properties.
function readResult_CreateFcn(hObject, eventdata, handles)
% hObject    handle to readResult (see GCBO)
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
data=guidata(handles.figure1);
var=getVar(handles);

data.var=var;
guidata(handles.figure1,data);
uiresume(handles.figure1);

% --- Executes on button press in cancel.
function cancel_Callback(hObject, eventdata, handles)
% hObject    handle to cancel (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
close(handles.figure1);


function var=getVar(handles)
var.name=get(handles.name,'String');
var.memPointer=get(handles.memPointer,'String');
var.description=get(handles.description,'String');
var.type=get(handles.type,'Value');
var.nrItem=str2double(get(handles.nrItem,'String'));
var.category=get(handles.category,'Value');
var.operation=get(handles.operation,'String');

function setVar(var,handles)
set(handles.name,'String',var.name);
set(handles.memPointer,'String',['0x',dec2hex(getNumeric(var.memPointer))]);
set(handles.description,'String',var.description);
set(handles.type,'Value',var.type);
set(handles.category,'Value',var.category);
set(handles.nrItem,'String',num2str(var.nrItem));
set(handles.operation,'String',var.operation);

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


% --- Executes during object creation, after setting all properties.
function category_CreateFcn(hObject, eventdata, handles)
% hObject    handle to category (see GCBO)
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
set(hObject,'String',{varCatArray.name});


% --- Executes on button press in addChange.
function addChange_Callback(hObject, eventdata, handles)
% hObject    handle to addChange (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=getVar(handles);
aoAddChangeVarDatabase(var);


% --------------------------------------------------------------------
function selectVar_Callback(hObject, eventdata, handles)
% hObject    handle to selectVar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=aoGetVar();
setVar(var,handles)

% --------------------------------------------------------------------
function NAME_Callback(hObject, eventdata, handles)
% hObject    handle to NAME (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)



function operation_Callback(hObject, eventdata, handles)
% hObject    handle to operation (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of operation as text
%        str2double(get(hObject,'String')) returns contents of operation as a double


% --- Executes during object creation, after setting all properties.
function operation_CreateFcn(hObject, eventdata, handles)
% hObject    handle to operation (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in nextVar.
function nextVar_Callback(hObject, eventdata, handles)
% hObject    handle to nextVar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=getVar(handles);
var=aoGetVar(var.name,1);
setVar(var,handles);


% --- Executes on button press in backVar.
function backVar_Callback(hObject, eventdata, handles)
% hObject    handle to backVar (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=getVar(handles);
var=aoGetVar(var.name,-1);
setVar(var,handles);


% --- Executes on button press in delete.
function delete_Callback(hObject, eventdata, handles)
% hObject    handle to delete (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
var=getVar(handles);
varNameToDelete=var.name;
var=aoGetVar(var.name,1);
aoDeleteVarDatabase(varNameToDelete);
setVar(var,handles);

% --- Executes on button press in delete.
function save_to_file_Callback(hObject, eventdata, handles)
% hObject    handle to delete (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
aoVariables=evalin('base','aoVariables');
uisave('aoVariables','aoVariables.mat')



