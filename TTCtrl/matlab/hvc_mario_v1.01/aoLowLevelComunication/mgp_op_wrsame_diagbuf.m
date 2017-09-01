function mgp_op_wrsame_diagbuf(firstDsp,lastDsp,len,startAddress,data,varargin)
% mgp_op_wrsame_diagbuf(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
% connectionNr: default is 1
% dataType: default is 'uint32'
% len must be less then 364 


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
global COMMAND_ID

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);
data=getNumeric(data);

if len>364
    error('len must be <=364');
end;

if(isempty(COMMAND_ID))
    COMMAND_ID=0;
end;
MGP_DEFINE();
COMMAND=150;
FLAGS=1;
%check variable parameters
if nargin==0+5
    connectionNr=1;
    dataType='uint32';
elseif nargin==1+5
    if(isnumeric(varargin{1}))
        connectionNr=varargin{1};
        dataType='uint32';
    else
        connectionNr=1;
        dataType=varargin{1};
    end
elseif nargin==2+5
    connectionNr=varargin{1};
    dataType=varargin{2};
else
    error('wrong number of parameters');
end;

%convert data to datatype
data=eval([dataType,'(data)']);

%get communication socket
udpSocket=getSocket(connectionNr);

%create header
%dword 1
dword=uint32([0,0,0]);
dword(1)=bitshift(uint32(COMMAND),24);
dummy=bitshift(uint32(lastDsp),16);
dword(1)=bitor(dword(1),dummy);
dummy=bitshift(uint32(firstDsp),4);
dword(1)=bitor(dword(1),dummy);

COMMAND_ID=uint8(COMMAND_ID+1);
if(COMMAND_ID==255)
    COMMAND_ID=0;
end;
%dword 2
len=uint32(len);
dword(2)=bitand(len,hex2dec('FF'));
dword(2)=bitor(bitshift(dword(2),8),bitshift(len,-8));
dword(2)=bitshift(dword(2),16);
dummy=bitshift(uint32(FLAGS),8);
dword(2)=bitor(dword(2),dummy);
dword(2)=bitor(dword(2),uint32(COMMAND_ID));
%dword 3
dword(3)=uint32(startAddress);

%write data
pnet(udpSocket,'write',uint8([0,0])); % write dummy
pnet(udpSocket,'write',uint32(dword(1)),'intel'); % write header
pnet(udpSocket,'write',uint32(dword(2))); % write header
pnet(udpSocket,'write',uint32(dword(3)),'intel'); % write header
pnet(udpSocket,'write',data,'intel'); % write header
pnet(udpSocket,'writepacket');   % Send buffer as UDP packet
%read back data
bufferLen=pnet(udpSocket,'readpacket');
header=pnet(udpSocket,'read',14,'uint8','noblock');
%data=pnet(udpSocket,'read',1000,dataType,'intel');
header(1:2)=[];%remove dummy
%test header if it is ok
if(header(4)~=MGP_OP_CMD_SUCCESS)
    header(4)
    error('communication failed MGP_OP_CMD_FAULT')
end;
if(header(8)~=COMMAND_ID)
    header(8)
    error('communication failed: wrong command ID')
end;


