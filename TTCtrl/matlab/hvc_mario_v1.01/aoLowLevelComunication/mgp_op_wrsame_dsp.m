function mgp_op_wrsame_dsp(firstDsp,lastDsp,len,startAddress,data,varargin)
% mgp_op_wrseq_dsp(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType],[FLAGS])
% connectionNr: default is 1
% dataType: default is 'uint32'
% FLAGS: default is 1
% len must be less then MAX_DWORD_LEN 


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
data=getNumeric(data);

%set default parameter values
connectionNr=1;
dataType='uint32';
FLAGS=1;

%check variable parameters
if nargin==1+5
    if(isnumeric(varargin{1}))
        connectionNr=varargin{1};
    else
        dataType=varargin{1};
    end
elseif nargin==2+5
    connectionNr=varargin{1};
    dataType=varargin{2};
elseif nargin==3+5
    connectionNr=varargin{1};
    dataType=varargin{2};
    FLAGS=varargin{3};
elseif nargin==5
else
    error('wrong number of parameters');
end;

%convert data to datatype
data=eval([dataType,'(data)']);


%test if in MEM
inbanco=0;
for i=1:4
if((startAddress>=DSP_MEM_BASE(i))&(len+startAddress-DSP_MEM_BASE(i)<=DSP_MEM_SIZE(i)))
    inbanco=1;
    break;
end;
end;
if(inbanco==0)
    error('write not within DSP Memory Block');
end;
    
writePar=getWritePar(startAddress,len);
for i=1:length(writePar)
    dataBlock=data(writePar(i).offset+1:writePar(i).offset+writePar(i).l);
    writeDsp(firstDsp,lastDsp,writePar(i).l,startAddress+writePar(i).offset,dataBlock,connectionNr,dataType,FLAGS);
end;




function data=writeDsp(firstDsp,lastDsp,len,startAddress,data,connectionNr,dataType,FLAGS)
global COMMAND_ID
MGP_DEFINE;

if len>MAX_DWORD_LEN
    error(['len must be <=',num2str(MAX_DWORD_LEN)]);
end;
if(len<=0)
    error('len must be >0');
end;
if((len>7)&(bitand(len,3)|bitand(startAddress,3)))
    error('errore di allineamento');
end;

if(isempty(COMMAND_ID))
    COMMAND_ID=0;
end;
COMMAND=0;

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

COMMAND_ID=COMMAND_ID+1;
COMMAND_ID= uint8(COMMAND_ID);
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



function writePar=getWritePar(startAddress,len)
i=1;
lenLeft=len;
offset=0;
%allign startaddress
if((lenLeft>7)&(bitand(startAddress,3)))
     writePar(i).l=4-bitand(startAddress,3);
     writePar(i).offset=offset;
     lenLeft=lenLeft-writePar(i).l;
     offset=offset+writePar(i).l;
     i=i+1;
end;

%qui sono allineato
while lenLeft>=364
 writePar(i).l=364;
 writePar(i).offset=offset;
 lenLeft=lenLeft-364;
 offset=offset+364;
 i=i+1;
end;
if lenLeft==0
    return;
end;
%qui sono allineato in come indirizzo
if ((lenLeft>7)&(bitand(lenLeft,3)))
    writePar(i).l=lenLeft-bitand(lenLeft,3);
    writePar(i).offset=offset;
    lenLeft=lenLeft-writePar(i).l;
    offset=offset+writePar(i).l;
    i=i+1;
end;

%verifico se serve ancora la parte finale
if lenLeft>0
   writePar(i).l=lenLeft;
   writePar(i).offset=offset;
end;

