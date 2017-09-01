function aoImportVariables(varArray,database,varargin)
% aoImportVariables(varArray,database,[newImport],[device])
% This function imports "varArray" into "database", e.g. aoVariablesLBT.mat
% varArray is the varArray got by aoReadMapFile
% database is the name of file contining all aovariables
% newImport if equal 1 defines also the types by looking to the prefix

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/09/25
% new version, the DSP variables name should be compliant with the following format: _xxxYZ_
% where: xxx is the module name (e.g. dm2, bm2, etc...)
%        Y   is the C class type (supported types: f=float, u=uint32, i=int32, s=struct(casted as uint32))
%        Z   is the adressing type (supported types: b=board, c=channel)
% $Revision 0.2 $ $Date: 18/02/2014 - extended for LATT; - by D. Pescoller
%     device is a new parameter telling which device the varibales belongs to
%     default id 'DSP', other supported are 'M4', 'SAB'

AO_VARIABLE_DEFINE();
try
    load(database);
    scratch=0;
catch
    disp('warning!!! database filename was not loaded creating from scratch!!');
    aoVariables=[];
    scratch=1;
end;

newImport=0;
if nargin>=3
    newImport=varargin{1};
end

device='DSP';
if nargin>=4
    device=varargin{2};
end
if newImport==0
    for i=1:length(varArray)
        var=varArray(i);
        % search varName in database
        if exist('aoVariables')
            index=find(strcmp({aoVariables(:).name},var.name));
        else
            index=[];
        end
        % if not found we add a new variable
        if isempty(index)
            newVar=var;
            newVar.description=[];
            newVar.type=1;
            newVar.operation='';
            if strfind(newVar.name,'num')
                newVar.type=TYPE_UINT32;
            end
            if strfind(newVar.name,'ptr')
                newVar.type=TYPE_UINT32;
            end
            if strfind(newVar.name,'size')
                newVar.type=TYPE_UINT32;
            end
            if strfind(newVar.name,'counter')
                newVar.type=TYPE_UINT32;
            end
            [newVar,fig]=aoVariable(newVar);
            if ~isempty(newVar)
                try
                    aoVariables(end+1)=newVar;
                catch
                    aoVariables=newVar;
                end
            end
        else
            aoVariables(index).memPointer=var.memPointer;
            aoVariables(index).nrItem=var.nrItem;
        end
    end
    if exist('fig')
        close(fig);
    end
else
    variable_types=['fuis'];
    category_types=['bc'];
    for i=1:length(varArray)
        var=varArray(i);
        if ~scratch
            index=find(strcmp({aoVariables(:).name},var.name));
        else
            index=[];
        end
        if isempty(index)
            switch upper(device)
                case 'SAB'
                    newVar=var;
                    newVar.description=[];
                    newVar.category=CAT_BOARD;
                case 'DSP'
                    % search varName in database
                    if length(var.name)<7 || var.name(1)~='_' || var.name(7)~='_' || min(var.name(5)~=variable_types) || min(var.name(6)~=category_types)
                        continue
                    end
                    newVar=var;
                    newVar.description=[];
                    switch find(var.name(5)==variable_types)
                        case 1
                            newVar.type=TYPE_SINGLE;
                        case 2
                            newVar.type=TYPE_UINT32;
                        case 3
                            newVar.type=TYPE_INT32;
                        case 4
                            newVar.type=TYPE_STRUCT;
                        otherwise
                            error('Unsupported variable type');
                    end
                    switch find(var.name(6)==category_types)
                        case 1
                            newVar.category=CAT_BOARD;
                        case 2
                            newVar.category=CAT_CHANNEL;
                        otherwise
                            error('Unsupported category type');
                    end
                otherwise
                    error('device not defined');
            end
            newVar.operation='';
            newVar.device=device;
            aoVariables=[aoVariables newVar];
        else
            aoVariables(index).memPointer=var.memPointer;
            aoVariables(index).nrItem=var.nrItem;
        end
    end
end

for i =1:length(aoVariables)
    aoVariables(i).memPointer=getNumeric(aoVariables(i).memPointer);
end
disp(['saving aoVariables to file:',database]);
save(database,'aoVariables');
changeVarDatabaseRefresh();

