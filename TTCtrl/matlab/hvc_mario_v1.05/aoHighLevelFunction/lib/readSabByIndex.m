function x=readSabByIndex(sabArray,nrItem,startAddress,varargin)
% reads data from SAB
% usage:
% x = readSabByIndex(sabArray,len,startAddress|varName,[dataType],[contExecFlag],[order])
%
% sabArray: is an array containing all sabId to read from
% nrItem: number of items to read
% startAddress: start address to read from
% [dataType]: optional argument default is 'uint16' other possible values
% are  'int16' 'double' 'single' 'int64' i.e. all matlab basic types
% [contExecFlag]: continuous execution flag if set to 1 continuously read
% data from system. Can be used for comunication tests. Default is 0
% [order]: is the order of numbering either sab or mirror; default is mirror
%
% example:
% readSabByIndex([1:10],4,aoGetAddress('sabu8_rFIFO'),'uint16')


% Author(s): D.Pescoller
%
% Copyright 2014-2018 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2014/02/17

sys_data=evalin('base','sys_data');


% test if sabArray has unique values
if length(unique(sabArray))~=length(sabArray)
    error('sabArray must have unique values')
end

if ~isnumeric(startAddress)
    startAddress=aoGetAddress(startAddress);
end

dataType='uint16';
if nargin>=4
    dataType=varargin{1};
end

contExecFlag=0;
if nargin>=5
    contExecFlag=varargin{2};
end

order='mirror';
if nargin>=6
    order=varargin{3};
end

% reorder in mirror numbering
if strcmpi(order,'mirror')
    sabArray=sys_data.DSP_CONF_MAT(sabArray+1,1);
end

% computes size of dataType in bytes
typeSize=8/length(typecast(0,dataType));
len=nrItem*typeSize;

aoWrite('_dspub_ContinuousExec',contExecFlag,255);
aoWrite('_dspub_ClearAnswerBuff',1,255);

var=aoGetVar('_dspub_MatLabSharedMemory1a');
InputData1=zeros(var.nrItem,1);
InputData2=zeros(var.nrItem,1);

aIdx=0;
bIdx=0;
for ii = 1:length(sabArray)
    for jj=1:4:len
        sabId=sabArray(ii);
        sabIdHex=dec2hex(sabId+256);
        tmpAddr=typecast(uint16(startAddress+jj-1),'uint8');
        addrLB=dec2hex(tmpAddr(1));
        addrHB=dec2hex(tmpAddr(2));
        %we always read 32 bits
        tmpAddr=typecast(uint16(startAddress+jj+1),'uint8');
        addrLB_2=dec2hex(tmpAddr(1));
        addrHB_2=dec2hex(tmpAddr(2));
        [ConcWord1out,ConcWord2out] = hexConcat (0,sabIdHex, '82', addrLB,addrHB,addrLB_2,addrHB_2, '0','0');
        if ~isempty(find(sys_data.busASabIds==sabId,1))
            InputData1(aIdx*2+1:aIdx*2+2)  =[hex2dec(ConcWord1out) hex2dec(ConcWord2out)]; % Write SAB
            aIdx=aIdx+1;
        end
        if ~isempty(find(sys_data.busBSabIds==sabId,1))
            InputData2(bIdx*2+1:bIdx*2+2)  =[hex2dec(ConcWord1out) hex2dec(ConcWord2out)]; % Write SAB
            bIdx=bIdx+1;
        end
    end
end

aoWrite('_dspub_NrPackets1a',aIdx,255);
aoWrite('_dspub_NrPackets1b',bIdx,255);
aoWrite('_dspub_MatLabSharedMemory1a',InputData1,255);
aoWrite('_dspub_MatLabSharedMemory1b',InputData2,255);
startCmd=0;
if aIdx>0
    startCmd=startCmd+1;
end
if bIdx>0
    startCmd=startCmd+4;
end
aoWrite('_dspub_CommandReady',startCmd,255);

if contExecFlag
    return
end
% wait command execution
while aoRead('_dspub_CommandReady',255)~=0
    pause(.2)
end

rep = {'Addr','Command','Data1','Data2','Data3','Data4','CRC','BusNr'};
if aIdx>0
    repBufa=dec2hex(aoRead('_dspub_AnswerBufferMemorya',255));
end
if bIdx>0
    repBufb=dec2hex(aoRead('_dspub_AnswerBufferMemoryb',255));
end

aIdx=0;
bIdx=0;
xTmp=uint8(zeros(length(unique(sabArray)),len));
for ii=1:length(sabArray)
    sabId=sabArray(ii);
    for jj=1:4:len
        if ~isempty(find(sys_data.busASabIds==sabId,1))
            [rep{ii,1}, rep{ii,2}, rep{ii,3}, rep{ii,4}, rep{ii,5}, rep{ii,6}, rep{ii,7},rep{ii,8}] = hexConcat (1, repBufa(aIdx*2+1,:),repBufa(aIdx*2+2,:));
            aIdx=aIdx+1;
        end
        if ~isempty(find(sys_data.busBSabIds==sabId,1))
            [rep{ii,1}, rep{ii,2}, rep{ii,3}, rep{ii,4}, rep{ii,5}, rep{ii,6}, rep{ii,7},rep{ii,8}] = hexConcat (1, repBufb(bIdx*2+1,:),repBufb(bIdx*2+2,:));
            bIdx=bIdx+1;
        end
        tmp=typecast([uint8(hex2dec(rep{ii,3})),uint8(hex2dec(rep{ii,4})),uint8(hex2dec(rep{ii,5})),uint8(hex2dec(rep{ii,6}))],'uint8');
        if jj+3<=len
            xTmp(ii,jj:jj+3)=tmp(1:4);
        else
            xTmp(ii,jj:len)=tmp(1:len-jj+1);
        end
    end
end

x=zeros(length(unique(sabArray)),nrItem);
for ii=1:length(sabArray)
    x(ii,1:nrItem)=typecast(xTmp(ii,:),dataType);
end
