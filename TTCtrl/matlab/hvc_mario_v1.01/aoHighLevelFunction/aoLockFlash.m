function aoLockFlash(flashBlock,varargin)
% aoLockFlash(flashBlock,[firstDsp,[lastDsp]],[connectionNr])
% locks a specified flashBlock
%
% flashBlock can be 1 to 5 
% 1 -> default logik
% 2 -> user logik
% 3 -> default program
% 4 -> user program
% 5 -> configuration area see MGP_FLASH_TABLE in MGP_DEFINE for details;
% filename is the location of file to download if not specified an open
% file GUI will appear
% firstDsp/lastDsp are the target address (see mgp_op_xxxx protocol) 
% default is 0

% Author(s): M. Andrighettoni
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005

%include for MGP_FLASH_TABLE
MGP_DEFINE();

firstDsp = 0;
lastDsp  = 0;
connectionNr=1;

%processing parameters
if nargin > 1
    firstDsp=varargin{1};
    lastDsp=firstDsp;
end;
if nargin > 2
    lastDsp=varargin{2};
end;
if nargin > 3
    connectionNr=varargin{3};
end;


startAddress=MGP_FLASH_TABLE(flashBlock).startAddress;
blockLen=MGP_FLASH_TABLE(flashBlock).len;
disp(['locking ',MGP_FLASH_TABLE(flashBlock).name]);
% %flashing
mgp_op_lock_flash(firstDsp,lastDsp,1,startAddress,blockLen,connectionNr);
