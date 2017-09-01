function waitTime = aoBufferWaitStop(selection,varargin)
% aoBufferWaitStop(selection,[firstDsp],[connectionNr])
% waits until the supplied buffer number has finished
%
% selection=array of buffer numbers to wait for stop
% firstDsp (mgp) default = 0
% lastDsp = firstDsp
% connectonNr default = 1
%
%
% returns nrIter which is the number of iterations it has to wait.
% this is useful to make sure that the buffer is still running
% when aoBufferWaitStop is called. If nrIter is 0 maybe the buffer was
% already stopped before!!!


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller
% $Revision 0.3 $ $Date: 12/10/2008 - return the time waited instead of number of iterations
% 18/06/09 added the start check to be sure that the control irq is running - by mario

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
tic;
while bitand(bufferStatus,stopMask)
    bufferStatus=mgp_op_rd_diagbuf(firstDsp,lastDsp,1,1,connectionNr);
    pause(.1);
end;
waitTime=toc;
