function expandStruct(varStruct,varargin)
% expandStruct(varStruct,[1]) 
% this fuction expands a struct
% varStruct is an array of structs wich has at least the following fields
% name
% data
% a variable of name=data variable is created in the calling workspace
% if the remove axis flag is set to 1 the AZ or EL prefix is removed from the var
% name AZPos => Pos

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09
if (nargin>1)&(varargin{1}==1)
    remove=1;
else
    remove=0;
end;
if (nargin>1)&(isstr(varargin{1})==1)
postfix=varargin{1};
else;
postfix='';
end;

for i=1:length(varStruct)
varname=varStruct(i).name;
vardata=varStruct(i).data;
    if remove
         if(strfind(varname,'AZ'))
             varname(1:2)=[];
         end;
         if(strfind(varname,'EL'))
             varname(1:2)=[];
         end;
     end;
     varname=[varname,postfix];
     assignin('caller',varname,vardata);
end;