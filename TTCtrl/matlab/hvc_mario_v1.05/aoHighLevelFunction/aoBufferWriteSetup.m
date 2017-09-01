function aoBufferWriteSetup(bufferArray,varargin)
% aoBufferWriteSetup(bufferArray,[connectionNr]) writes a buffer setup to dsp
% connectonNr default = 1
%
% see also aoBufferCreate

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller


AO_VARIABLE_DEFINE();

connectionNr=1;
if nargin>=2
    connectionNr=varargin{1};
end

for i=1:length(bufferArray)
    stBuffer=bufferArray(i);
    b=stBuffer;
    %convert / interpret stBuffer

    b.triggerPointer=getNumeric(stBuffer.triggerPointer);
    if isempty(b.triggerPointer)
        var=aoGetVar(stBuffer.triggerPointer);
        b.triggerPointer=uint32(var.memPointer);
    end
    b.triggerMask=getNumeric(stBuffer.triggerMask);
    b.triggerValue=getNumeric(stBuffer.triggerValue);
    b.decFactor=getNumeric(stBuffer.decFactor);
    b.len=getNumeric(stBuffer.len);
    b.nrItem=getNumeric(stBuffer.nrItem);
    b.sdramPointerValue=getNumeric(stBuffer.sdramPointerValue);     
    b.sdramPointerCounter=b.sdramPointerValue;
    b.dspPointer=getNumeric(stBuffer.dspPointer);     
    if isempty(b.dspPointer)
        var=aoGetVar(stBuffer.dspPointer);
        b.dspPointer=uint32(var.memPointer);
    end

    aoMem(1)=b.triggerPointer+bitshift(b.triggerCompare,30)+bitshift(b.triggerDsp,22);
    aoMem(2)=b.triggerMask;
    switch (varTypeArray(b.triggerDataType).name)
       case 'single'
          aoMem(3)=typecast(single(b.triggerValue),'uint32');
       case 'int32'
          aoMem(3)=typecast(int32(b.triggerValue),'uint32');
       case 'uint32'
          aoMem(3)=b.triggerValue;
       otherwise
          error('Invalid variable type');
    end
    aoMem(4)=b.decFactor;
    aoMem(5)=b.len+bitshift(b.len,16);
    aoMem(6)=b.sdramPointerCounter;
    aoMem(7)=b.dspPointer+bitshift(b.dsp,22)+bitshift(b.nrItem,24);
    aoMem(8)=b.sdramPointerValue+bitshift(b.circular,31)+bitshift(b.direction,30);

    %write data to dsp
    firstDsp=stBuffer.firstDsp;
    lastDsp=stBuffer.lastDsp;
    len=8;
    startAddress=8*b.bufferNumber;
    data=uint32(aoMem);
    mgp_op_wrsame_diagbuf(firstDsp,lastDsp,len,startAddress,data,connectionNr,'uint32');

end



