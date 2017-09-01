function aoEnableDrives(enableList,varargin)
% controls the drives enable
% aoEnableDrives(enableList,[fistDsp],[lastDsp],[connectionNr])
% array is the list of which drives should be enabled
% all drives which are not listed will be disabled
% example:
% aoEnableDrives([]) disables all drives
% aoEnableDrives([1:8]) enables all drives
% aoEnableDrives([1:3,6:8]) enables all drives but drive 4 and 5
% connectonNr default = 1

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller
% $Revision 0.3 $ $Date: 03/10/2008 -modified to drive up to 16 channels - M.Andrighettoni

if min(enableList)<=0
    error('channel must be >= 1');
end
if max(enableList)>16
    error('channel must be <= 16');
end
firstDsp=0;
lastDsp=0;
connectionNr=1;
if nargin==2
    firstDsp=varargin{1};
    lastDsp=varargin{1};
elseif nargin==3
    firstDsp=varargin{1};
    lastDsp=varargin{2};
elseif nargin==4
    firstDsp=varargin{1};
    lastDsp=varargin{2};
    connectionNr=varargin{3};
end

enableList=unique(enableList);

enableMask='01010101010101010101010101010101';
enableMask(32-(enableList-1)*2)='0';
enableMask(32-(enableList)*2+1)='1';

mgp_op_reset_devices(firstDsp,lastDsp,[0,bin2dec(enableMask)],connectionNr);
