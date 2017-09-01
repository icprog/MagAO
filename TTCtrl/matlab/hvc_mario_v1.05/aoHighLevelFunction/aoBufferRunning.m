function runningBuffers = aoBufferRunning(selection,varargin)
% aoBufferRunning(selection,[firstDsp],[connectionNr])
% returns the running status of the supplied buffers
%
% selection=array of buffer numbers to check
% firstDsp (mgp) default = 0
% lastDsp = firstDsp
% connectonNr default = 1
%
%
% returns a bit string where the bits set indicate the running buffers


% Author(s): R. Biasi
%
% Copyright 2004-2012 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2011/12/01

firstDsp=0;
lastDsp=0;
connectionNr=1;

if nargin==2
    firstDsp=varargin{1};
    lastDsp=varargin{1};
elseif nargin==3
    firstDsp=varargin{1};
    lastDsp=varargin{1};
    connectionNr=varargin{2};
end;

% first verifies that the start buffer command has been processed
bufferStatus=mgp_op_rd_diagbuf(firstDsp,lastDsp,1,0,connectionNr);
if bufferStatus
   error('Buffer command has not been processed... is the control irq enabled?');
end

mask((selection)*2)=1;
maskString=num2str(mask(end:-1:1));
stopMask=bin2dec(strrep(maskString,' ',''));
bufferStatus=mgp_op_rd_diagbuf(firstDsp,lastDsp,1,1,connectionNr);
runningBuffers=bitand(bufferStatus,stopMask);
