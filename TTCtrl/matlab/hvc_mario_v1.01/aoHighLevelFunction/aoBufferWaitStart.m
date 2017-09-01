function waitTime = aoBufferWaitStart(selection,varargin)
% aoBufferWaitStart(selection,[firstDsp],[connectionNr])
% waits until the supplied buffer number has started. Useful in conjunction
% with aoBufferTrigger
%
% selection=array of buffer numbers to wait for start
% firstDsp (mgp) default = 0
% lastDsp = firstDsp
% connectonNr default = 1
%
%
% returns waitTime which is the number of time waited for start.
% this is useful to make sure that the buffer is still waiting for start
% when aoBufferWaitStart is called. If nrIter is 0 maybe the buffer was
% already start before!!!


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 12/10/2008
% 09/06/09 solved a bug on startMask creation and check (while loop) - by mario
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

mask((selection)*2-1)=1;
maskString=num2str(mask(end:-1:1));
startMask=bin2dec(strrep(maskString,' ',''));
bufferStatus=mgp_op_rd_diagbuf(firstDsp,lastDsp,1,1,connectionNr);
tic;
while bitand(bufferStatus,startMask)
    bufferStatus=mgp_op_rd_diagbuf(firstDsp,lastDsp,1,1,connectionNr);
    pause(.1);
end;
waitTime=toc;
