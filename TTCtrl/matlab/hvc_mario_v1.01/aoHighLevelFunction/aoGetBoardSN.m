function data=aoGetBoardSN(varargin)
% data_struct=aoGetBoardSN([firstDsp] , [lastDsp] , [connection])
% Gests DSP status and returns a struct with 4 fields.
% Author(s): Mario
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 04/06/2009

firstDsp=0;
lastDsp=0;
connectionNr=1;

if nargin==1
    firstDsp=varargin{1};
    lastDsp=varargin{1};
elseif nargin==2
    firstDsp=varargin{1};
    lastDsp=varargin{2};
elseif nargin==3
    firstDsp=varargin{1};
    lastDsp=varargin{2};
    connectionNr=varargin{3};
end;

% device detection
if firstDsp==255
   data=bitand(mgp_op_rd_sram(firstDsp,lastDsp,1,'0x3800e',connectionNr),65535);
else   
   data=bitand(mgp_op_rd_sram(firstDsp,lastDsp,1,'0x18173',connectionNr),65535);
end
