function b=aoBufferCreate()
% creates a sample bufferStruct() b defined as follow:
%
% b.bufferName='sample';  %name of buffer, is not mandatory, just useful
% b.triggerPointer=0;     %pointer of trigger
% b.triggerDataType=1;    %trigger data type 1='single' 2='int32' 3='uint32'
% b.triggerDsp=0;         %number of DSP 0 or 1
% b.triggerMask=0;        %trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
% b.triggerValue=0;       %trigger value
% b.triggerCompare=0;     %trigger compare type: 0=none 1="value<*pointer" 2="value>*pointer" 3="value=*pointer"
% b.dspPointer=0;         %pointer of read/write element
% b.nrItem=0;             %size of single read/write operation normally=1
% b.dataType=1;           %data type 1='single' 2='int32' 3='uint32'
% b.dsp=0;                %which DSP 0 or 1
% b.len=0;                %length of buffer
% b.decFactor=0;          %decimation factor
% b.sdramPointerValue=0;  %pointer where to/from store/read data
% b.direction=0;          %direction 0=read data 1=write data
% b.circular=0;           %0=linear 1=circular
% b.bufferNumber=1;       %which buffer number 1 to 6 for each board
% b.firstDsp=0;           %first DSP where to write setup
% b.lastDsp=0;            %last DSP where to write setup
% see also aoBuffer


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

%buffer internal

b.bufferName='sample';  %name of buffer is not mandatory, just useful
b.triggerPointer=0;     %pointer of trigger
b.triggerDataType=1;    %trigger data type 1='single' 2='int32' 3='uint32'
b.triggerDsp=0;         %number of DSP 0 or 1
b.triggerMask=0;        %trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
b.triggerValue=0;       %trigger value
b.triggerCompare=0;     %trigger compare type: 0=none 1="value<*pointer" 2="value>*pointer" 3="value=*pointer"
b.dspPointer=0;         %pointer of read/write element
b.nrItem=0;             %size of single read/write operation normally=1
b.dataType=1;           %dataType
b.dsp=0;                %which DSP 0 or 1
b.len=0;                %length of buffer
b.decFactor=0;          %decimation factor
b.sdramPointerValue=0;  %pointer where to/from store/read data
b.direction=0;          %direction 0=read data 1=write data
b.circular=0;           %0=linear 1=circular
b.bufferNumber=1;       %which buffer number 1 to 6 for each board
b.firstDsp=0;           %first DSP where to write setup
b.lastDsp=0;            %last DSP where to write setup

x=b;

