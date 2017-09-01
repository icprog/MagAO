function b=aoBufferReadSetup(varargin)
% this reads a buffer struct from dsp
% b=aoBufferReadSetup([bufferNumber],[triggerDataType],[dataType],[firstDsp],[connectionNr])
% stBuffer needs at least the foolowing fields
% bufferNumber default is 1
% triggerDataType default is 1
% dataType default is 1
% firstDsp  default is 1
% connectionNr

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller


firstDsp=0;
lastDsp=0;
connectionNr=1;

if nargin==0
    stBuffer.bufferNumber=1;
    stBuffer.triggerDataType=1;
    stBuffer.dataType=1;
elseif nargin==1
    stBuffer.bufferNumber=varargin{1};
    stBuffer.triggerDataType=1;
    stBuffer.dataType=1;
elseif nargin==2
    stBuffer.bufferNumber=varargin{1};
    stBuffer.triggerDataType=varargin{2};
    stBuffer.dataType=1;
elseif nargin==3
    stBuffer.bufferNumber=varargin{1};
    stBuffer.triggerDataType=varargin{2};
    stBuffer.dataType=varargin{3};
elseif nargin==4
    stBuffer.bufferNumber=varargin{1};
    stBuffer.triggerDataType=varargin{2};
    stBuffer.dataType=varargin{3};
    firstDsp=varargin{4};
    lastDsp=varargin{4};
elseif nargin==5
    stBuffer.bufferNumber=varargin{1};
    stBuffer.triggerDataType=varargin{2};
    stBuffer.dataType=varargin{3};
    firstDsp=varargin{4};
    lastDsp=varargin{4};
    connectionNr=varargin{5};    
end;
    



AO_VARIABLE_DEFINE();

b=stBuffer;
%write data to dsp
len=8;
startAddress=8*b.bufferNumber;
aoMem=mgp_op_rd_diagbuf(firstDsp,lastDsp,len,startAddress,connectionNr,'uint32');

%read trigger value again if it is not an int32
if(~strcmp(varTypeArray(b.triggerDataType).name,'uint32'))
    tmp=mgp_op_rd_diagbuf(firstDsp,lastDsp,1,startAddress+2,connectionNr,varTypeArray(b.triggerDataType).name);
    b.triggerValue=num2str(tmp);
else
    b.triggerValue=num2str(aoMem(3));
end;



b.triggerPointer=['0x',dec2hex(bitand(aoMem(1),hex2dec('1FFFFF')))];
b.triggerCompare=bitshift(aoMem(1),-30);
b.triggerDsp=bitand(bitshift(aoMem(1),-22),1);
b.triggerMask=dec2hex(aoMem(2));
b.decFactor=num2str(aoMem(4));
b.len=num2str(bitand(aoMem(5),hex2dec('FFFF')));
b.sdramPointerCounter=num2str(aoMem(6));
b.dspPointer=['0x',dec2hex(bitand(aoMem(7),hex2dec('1FFFFF')))];
b.dsp=bitand(bitshift(aoMem(7),-22),1);
b.nrItem=num2str(bitand(bitshift(aoMem(7),-24),hex2dec('FF')));
b.sdramPointerValue=b.sdramPointerCounter;
b.circular=bitand(bitshift(aoMem(8),-31),1);
b.direction=bitand(bitshift(aoMem(8),-30),1);
b.firstDsp=firstDsp;
b.lastDsp=lastDsp;







