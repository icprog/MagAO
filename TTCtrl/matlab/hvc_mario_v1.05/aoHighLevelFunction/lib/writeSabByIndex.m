function writeSabByIndex(sabArray,nrItem,startAddress,data,varargin)
% writes data to SAB
% usage:
% x = writeSabByIndex(sabArray,len,startAddress|varName,data,[dataType],[order])
%
% sabArray: is an array containing all sabId to write to. 'ALL' is
% broadcast
% nrItem: number of items to write
% startAddress: start address to write
% data: is a matrix of values to write
% [dataType]: optional argument default is 'uint16' other possible values
% are  'int16' 'double' 'single' 'int64' i.e. all matlab basic types
% [order]: is the order of numbering either sab or mirror; default is mirror
%
% example:
% writeSabByIndex([1,10],4,aoGetAddress('sabu8_rFIFO'),[1 2 3 4;5 5 5 5],'uint16')
% writes 1 2 3 4 to sab 1 and 5 5 5 5 to sab 10


% Author(s): D.Pescoller
%
% Copyright 2014-2018 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2014/02/17


sys_data=evalin('base','sys_data');

if ~isnumeric(startAddress)
    startAddress=aoGetAddress(startAddress);
end

% test if sabArray has unique values
if strcmpi(sabArray,'all')
    sabArray=255;
end

if length(unique(sabArray))~=length(sabArray)
    error('sabArray must have unique values')
end

if length(sabArray)*nrItem~=length(data) && nrItem~=length(data)
    error('dspArrayLen*len must be equal to dataLen')
end

dataType='uint16';
if nargin>=5
    dataType=varargin{1};
end

order='mirror';
if nargin>=6
    order=varargin{2};
end

% reorder in mirror numbering
 
if strcmpi(order,'mirror') && sabArray(1)~=255         % only if we are not in broadcast
    sabArray=sys_data.DSP_CONF_MAT(sabArray+1,1);
end

% computes size of dataType in bytes
typeSize=8/length(typecast(0,dataType));
len=nrItem*typeSize;

aoWrite('_dspub_ContinuousExec',0,255);
aoWrite('_dspub_ClearAnswerBuff',1,255);

var=aoGetVar('_dspub_MatLabSharedMemory1a');
InputData1=zeros(var.nrItem,1);
InputData2=zeros(var.nrItem,1);

aIdx=0;
bIdx=0;
dataTmp=uint8(zeros(length(sabArray),len));

for ii = 1:length(sabArray)
    dataTmp(ii,:)=typecast(feval(dataType,data(ii,:)),'uint8');
    for jj=1:2:len
        sabId=sabArray(ii);
        sabIdHex=dec2hex(sabId+256);
        tmpAddr=typecast(uint16(startAddress+jj-1),'uint8');
        addrLB=dec2hex(tmpAddr(1));
        addrHB=dec2hex(tmpAddr(2));
        valLB=dec2hex(dataTmp(ii,jj));
        if  jj+1<=len
            valHB=dec2hex(dataTmp(ii,jj+1));
            cmd='E0';
        else
            valHB=valLB;
            cmd='E1';
        end
        
        [ConcWord1out,ConcWord2out] = hexConcat (0,sabIdHex, cmd, addrLB,addrHB,valLB,valHB, '0','0');
        if ( ~isempty(find(sys_data.busASabIds==sabId,1)) || sabId == 255 )
            InputData1(aIdx*2+1:aIdx*2+2)  =[hex2dec(ConcWord1out) hex2dec(ConcWord2out)]; % Write SAB
            aIdx=aIdx+1;
        end
        if ( ~isempty(find(sys_data.busBSabIds==sabId,1)) || sabId == 255)
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
% wait command execution
while aoRead('_dspub_CommandReady',255)~=0
    pause(.2)
end
