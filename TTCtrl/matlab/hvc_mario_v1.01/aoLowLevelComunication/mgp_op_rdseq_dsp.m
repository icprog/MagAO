function data=mgp_op_rdseq_dsp(firstDsp,lastDsp,len,startAddress,varargin)
% MGP_OP_RDSEQ_DSP(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
% connectionNr: default is 1
% dataType: default is 'uint32'


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

MGP_DEFINE;

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);

%check variable parameters
if nargin==0+4
    connectionNr=1;
    dataType='uint32';
elseif nargin==1+4
    if(isnumeric(varargin{1}))
        connectionNr=varargin{1};
        dataType='uint32';
    else
        connectionNr=1;
        dataType=varargin{1};
    end
elseif nargin==2+4
    connectionNr=varargin{1};
    dataType=varargin{2};
else
    error('wrong number of parameters');
end;

%test if in MEM
inbanco=0;
for i=1:4
if((startAddress>=DSP_MEM_BASE(i))&&(len+startAddress-DSP_MEM_BASE(i)<=DSP_MEM_SIZE(i)))
    inbanco=1;
    break;
end;
end;
if(inbanco==0)
    error('write not within DSP Memory Block');
end;

readPar=getReadPar(startAddress,len);
data=[];
for i=1:length(readPar)
    data=[data;readDsp(firstDsp,lastDsp,readPar(i).l,startAddress+readPar(i).offset,connectionNr,dataType)'];
end;



function data=readDsp(firstDsp,lastDsp,len,startAddress,connectionNr,dataType,varargin)
global COMMAND_ID
MGP_DEFINE();
if(isempty(COMMAND_ID))
    COMMAND_ID=0;
end;
COMMAND=2;
FLAGS=1;
if(len>7&&(bitand(len,3)||bitand(startAddress,3)))
    error('errore di allineamento');
end;

%if comunication fails for timeout serveral trys are made 
if(nargin==6)
    livelloRicorsione=1;
else
    livelloRicorsione=varargin{1}+1;
end;
if (livelloRicorsione >3)
    error('Comunication failed too many retry')
end;
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
pnet(udpSocket,'writepacket');   % Send buffer as UDP packet
%read back data
bufferLen=pnet(udpSocket,'readpacket');
header=pnet(udpSocket,'read',14,'uint8','noblock');
data=pnet(udpSocket,'read',1000,dataType,'intel');
header(1:2)=[];%remove dummy
%test header if it is ok
if(header(4)~=MGP_OP_CMD_SUCCESS)
    comunicationError=header(4)
    %if there was an error we retry again
        data=readDsp(firstDsp,lastDsp,len,startAddress,connectionNr,dataType,livelloRicorsione);
        return;
end;
readLen=bitor(uint16(header(5)),bitshift(uint16(header(6)),8));
if(readLen~=len*(lastDsp-firstDsp+1))
    readLen
    error('communication failed: wrong data length');
end;
if(header(8)~=COMMAND_ID)
    header(8)
    error('communication failed: wrong command ID')
end;





function readPar=getReadPar(startAddress,len)
i=1;
lenLeft=len;
offset=0;
%allign startaddress
if((lenLeft>7)&&(bitand(startAddress,3)))
     readPar(i).l=4-bitand(startAddress,3);
     readPar(i).offset=offset;
     lenLeft=lenLeft-readPar(i).l;
     offset=offset+readPar(i).l;
     i=i+1;
end;

%qui sono allineato
while lenLeft>=364
 readPar(i).l=364;
 readPar(i).offset=offset;
 lenLeft=lenLeft-364;
 offset=offset+364;
 i=i+1;
end;
if lenLeft==0
    return;
end;
%qui sono allineato in come indirizzo
if ((lenLeft>7)&&(bitand(lenLeft,3)))
    readPar(i).l=lenLeft-bitand(lenLeft,3);
    readPar(i).offset=offset;
    lenLeft=lenLeft-readPar(i).l;
    offset=offset+readPar(i).l;
    i=i+1;
end;

%verifico se serve ancora la parte finale
if lenLeft>0
   readPar(i).l=lenLeft;
   readPar(i).offset=offset;
end;

