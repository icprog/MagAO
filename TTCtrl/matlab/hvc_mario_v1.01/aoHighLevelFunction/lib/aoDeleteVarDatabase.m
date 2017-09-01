function aoDeleteVarDatabase(varName)
% aoDeleteVarDatabase(var)
% searches var in database (by name) deletes it from database


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/09/25


AO_VARIABLE_DEFINE();
% AO_VAR_DATABASE_FULL_PATH=which(AO_VAR_DATABASE);
% load(AO_VAR_DATABASE_FULL_PATH);
aoVariables=evalin('base','aoVariables');

%search varName in database
index=find(strcmp({aoVariables(:).name},varName));
if(isempty(index))
    error('var not found');
else
    aoVariables(index)=[];
    disp('deleting ao var');
end;
% save(AO_VAR_DATABASE_FULL_PATH,'aoVariables');
assignin('base','aoVariables',aoVariables);

changeVarDatabaseRefresh();

