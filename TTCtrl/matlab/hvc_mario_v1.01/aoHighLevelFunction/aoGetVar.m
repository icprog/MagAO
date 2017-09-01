function [varStruct, varIndex]=aoGetVar(varargin)
% varStruct=aoGetVar([varName],[indexOffset])
% search variable which name is varName and return its full varStruct
% varName can also be the ao address decimal value or hex value ('0x')
% offset is used to navigate variables database
% for example:
% varStruct=aoGetVar('global_counter')
% varStruct=aoGetVar('0x80000')
% varStruct=aoGetVar(524288)
% varStruct=aoGetVar() in this case a var selectGui will be displayed

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
% 31/05/10 in the return argument added the variable index in the aoVariable struct

varIndexOffset=0;

persistent aoVariables;
AO_VARIABLE_DEFINE();
if(isstruct(aoVariables)==0)
    try
        disp('loading aoVariables from workspace')
        aoVariables=evalin('base','aoVariables');
    catch
        error('Unable to find aoVariables within workspace')
    end;
end;

% sort aoVaribles
[str,index] = sortrows({aoVariables.name}');
aoVariables = aoVariables([index]);
if nargin==0
      [s,v] = listdlg('PromptString','Select a variable:',...
                      'SelectionMode','single',...
                      'ListString',str);
      if(v)
          varStruct=aoVariables(s);
          
      else
          varStruct=[];
      end;
      return;
else
    varName=varargin{1};
end;
if nargin==2
    varIndexOffset=varargin{2};
end;

%if we pass only the name we search in the database
if ischar(varName)
    %search varName in database
    varIndex=find(strcmp({aoVariables(:).name},varName))+varIndexOffset;
    if(isempty(varIndex))
        try
            varIndex=find([aoVariables.memPointer]==getNumeric(varName))+varIndexOffset;
        catch
            varIndex=[];
        end;
    end;
else
    varIndex=find([aoVariables(:).memPointer]==varName)+varIndexOffset;
    if(isempty(varIndex))
        error([num2str(varName),' was not found in aoVariables database']);
    end;
end;

if(isempty(varIndex))
    error([varName,' was not found in aoVariables database']);
end;

 
if     varIndex<1
   varIndex=length(aoVariables);
elseif varIndex>length(aoVariables)
   varIndex=1;
end;

varStruct=aoVariables(varIndex);
