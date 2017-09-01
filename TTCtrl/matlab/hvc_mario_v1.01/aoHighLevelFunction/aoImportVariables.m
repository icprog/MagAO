function aoImportVariables(varArray,database)
% aoImportVariables(varArray,database)
% This function imports "varArray" into "database", e.g. aoVariablesLBT.mat
% varArray is the varArray got by aoReadMapFile
% database is the name of file contining all aovariables 

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/09/25
AO_VARIABLE_DEFINE();
try
    load(database);
catch
   disp('warning!!! database filename was not loaded creating from scratch!!');
end;

for i=1:length(varArray)   

    var=varArray(i);
    %search varName in database
    if(exist('aoVariables'))
        index=find(strcmp({aoVariables(:).name},var.name));
    else
        index=[];
    end;
    %if not found we add a new variable
    if(isempty(index))
       newVar=var;
       newVar.description=[];
       newVar.type=1;
       newVar.operation='';
       if strfind(newVar.name,'num')
           newVar.type=TYPE_UINT32;
       end;
       if strfind(newVar.name,'ptr')
           newVar.type=TYPE_UINT32;
       end;
       if strfind(newVar.name,'size')
           newVar.type=TYPE_UINT32;
       end;
       if strfind(newVar.name,'counter')
           newVar.type=TYPE_UINT32;
       end;
       
       [newVar,fig]=aoVariable(newVar);
       if(~isempty(newVar))
           try
             aoVariables(end+1)=newVar;
           catch
             aoVariables=newVar;
           end;
       end;
    else
        aoVariables(index).memPointer=var.memPointer;
        aoVariables(index).nrItem=var.nrItem;
    end;
end;
if exist('fig')
    close(fig);
end;

for i =[1:length(aoVariables)]; aoVariables(i).memPointer=getNumeric(aoVariables(i).memPointer); end
disp(['saving aoVariables to file:',database]);
save(database,'aoVariables');
changeVarDatabaseRefresh();

