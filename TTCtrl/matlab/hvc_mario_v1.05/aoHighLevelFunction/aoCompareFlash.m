function [flashContentDefault,flashContentUser]=aoCompareFlash(flashBlock,varargin)
% aoCompareFlash(flashBlock,[filename],[firstDsp,[lastDsp]])
% Compares default and user flash blocks between each other and versus a
% file
%
% flashBlock can be 1 or 2
% 1 -> default logic
% 2 -> default program
% filename is the location of file to compare if not specified an open
% file GUI will appear
% firstDsp/lastDsp are the target address (see mgp_op_xxxx protocol) 
% default is 0
%
% Returns:
% flashContentDefault = content of Default flash
% flashContentUser = content of User flash

% Author(s): R. Biasi
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005

%include for MGP_FLASH_TABLE
MGP_DEFINE();

firstDsp=0;
lastDsp=0;


%processing parameters
if nargin>1
    if isstr(varargin{1})
        fileName=varargin{1};
        if nargin > 2
            firstDsp=varargin{2};
        end;
        if nargin > 3
            lastDsp=varargin{3};
        else
            lastDsp=firstDsp;
        end;
    else
        firstDsp=varargin{1};
        if nargin > 2
            lastDsp=varargin{2};
        else
            lastDsp=firstDsp;
        end;
    end;
end;

if strcmp(MGP_FLASH_TABLE(flashBlock).name,'default_logic'),
    for i=1:length(MGP_FLASH_TABLE),
        if strcmp(MGP_FLASH_TABLE(i).name,'user_logic'), break, end;
    end
elseif strcmp(MGP_FLASH_TABLE(flashBlock).name,'default_program'),
    for i=1:length(MGP_FLASH_TABLE),
        if strcmp(MGP_FLASH_TABLE(i).name,'user_program'), break, end;
    end
else 
    error('Wrong Flash block, shall be either default_logic or default_program');
end;
idxUser=i;

%get file
if ~exist('fileName')
    [filename, pathname] = uigetfile('*.*', 'selecting flash file');   
    fileName=[pathname,filename];
end;

fid=fopen(fileName);
fileContent=fread(fid,inf,'uint32');

startAddress=MGP_FLASH_TABLE(flashBlock).startAddress;
fileLen=length(fileContent);
memLen=min(MGP_FLASH_TABLE(idxUser).len,MGP_FLASH_TABLE(flashBlock).len);
%compare
flashContentDefault=mgp_op_rdseq_flash(firstDsp,firstDsp,memLen,startAddress);
if(all(flashContentDefault(1:fileLen)==fileContent))
    disp('Default block = file');
else
    disp('Default block ~= file');
end;

startAddress=MGP_FLASH_TABLE(idxUser).startAddress;
%compare
flashContentUser=mgp_op_rdseq_flash(firstDsp,firstDsp,memLen,startAddress);
if(all(flashContentUser(1:fileLen)==fileContent))
    disp('User block = file');
    if fileLen ~= MGP_FLASH_TABLE(idxUser).len,
        disp('Warning: different file length');
    end
else
    disp('User block ~= file');
end;

if(all(flashContentUser==flashContentDefault))
    disp('User block = Default block');
else
    disp('User block ~= Default block');
end;


fclose(fid);
    