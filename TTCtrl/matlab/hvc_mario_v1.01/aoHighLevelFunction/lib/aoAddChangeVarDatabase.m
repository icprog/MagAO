function aoAddChangeVarDatabase(var)
% aoAddChangeVarDatabase(var)
% searches var in database (by name) and sets the new settings
% if var is not found it is added to the database
% var is a varStruct

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/09/25


AO_VARIABLE_DEFINE();
% AO_VAR_DATABASE_FULL_PATH=which(AO_VAR_DATABASE);
% load(AO_VAR_DATABASE_FULL_PATH);
aoVariables=evalin('base','aoVariables');

varName=var.name;
var.memPointer=getNumeric(var.memPointer);
%search varName in database
index=find(strcmp({aoVariables(:).name},varName));
if(isempty(index))
    aoVariables(end+1)=var;
    disp('Adding ao var');
else
    aoVariables(index)=var;
    disp('Changing ao var');
end;

%make all mempointers numeric
for i =[1:length(aoVariables)]; aoVariables(i).memPointer=getNumeric(aoVariables(i).memPointer); end

% save(AO_VAR_DATABASE_FULL_PATH,'aoVariables');
assignin('base','aoVariables',aoVariables);

changeVarDatabaseRefresh();

