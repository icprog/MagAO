function var=picReadMapFile(filename)
% reads file filename and returns an array of var
% var is a struct containing the following fields
%           name:
%         nrItem:
%     memPointer:
%           type:
%         device:

% usage:
% varArray=picReadMapFile(filename)

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2014/02/17

VERBOSE=0;

f1=fopen(filename);
content=fread(f1);

START_MARKER='External Symbols in Data Memory (by address):';
STOP_MARKER='External Symbols in Data Memory (by name):';

start=strfind(content',START_MARKER)+length(START_MARKER);
stop=strfind(content',STOP_MARKER)-1;
varSection=char(content(start:stop)');
% fprintf(1,'This portion of file will be used:\n %s',varSection);
[memPointer name]=strread(varSection,'%s %s');
parCounter=0;
for ii=1:length(memPointer)
    var(ii).name=name{ii};
    if (var(ii).name(1)=='_')
        var(ii).name(1)=[];
    end
    var(ii).memPointer=memPointer{ii};
    var(ii).nrItem=1;
    if (length(var(ii).name)<5)
        if VERBOSE
            warning('Variable <%s> prefix is not compliant only partially imported',var(ii).name);
        end
        parCounter=parCounter+1;
        continue
    end
    size=sscanf(var(ii).name(5:end),'%i');
    switch var(ii).name(4)
        case 'i'
            var(ii).type=sprintf('int%i',size);
        case 'u'
            var(ii).type=sprintf('uint%i',size);
        case 's'
            % structure
            var(ii).type='uint8';
        otherwise
            if VERBOSE
                warning('Variable <%s> prefix is not compliant only partially imported',var(ii).name);
            end
            parCounter=parCounter+1;
    end
end
fprintf(1,'picReadMapFile: %i variables imported; %i variables have only partial information\n',length(var),parCounter);
fclose(f1);