function mgp_op_wrsame_dsp(firstDsp,lastDsp,len,startAddress,data,varargin)
% mgp_op_wrseq_dsp(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType],[FLAGS])
% [connectionNr]: default is 1
% [dataType]:     default is 'uint32'
% [FLAGS]:        default is 1
% len must be less then MAX_DWORD_LEN 


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004 (Diti)
% xx/xx/xxxx eliminated the DSP memory in range test (needed for compatibility with new ADSP TS201) (Mario)
% 09/03/2012 modified by mario to support tcp connection via LCU

MGP_DEFINE();

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

writePar=getWritePar(startAddress,len);
for i=1:length(writePar)
    dataBlock=data(writePar(i).offset+1:writePar(i).offset+writePar(i).l);
    writeDsp(firstDsp,lastDsp,double(writePar(i).l),startAddress+writePar(i).offset,dataBlock,connectionNr,dataType,FLAGS);
end;




function data=writeDsp(firstDsp,lastDsp,len,startAddress,data,connectionNr,dataType,FLAGS)
global COMMAND_ID
MGP_DEFINE();

if len>MAX_DWORD_LEN
    error(['len must be <=',num2str(MAX_DWORD_LEN)]);
end;
if(len<=0)
    error('len must be >0');
end;
if((len>7)&&(bitand(len,3)||bitand(startAddress,3)))
    error('errore di allineamento');
end;

if(isempty(COMMAND_ID))
    COMMAND_ID=0;
end;
COMMAND=0;

% get communication socket
[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'udp'
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
      pnet(socket,'write',uint8([0,0])); % write dummy
      pnet(socket,'write',uint32(dword(1)),'intel'); % write header
      pnet(socket,'write',uint32(dword(2))); % write header
      pnet(socket,'write',uint32(dword(3)),'intel'); % write header
      pnet(socket,'write',data,'intel'); % write header
      pnet(socket,'writepacket');   % Send buffer as UDP packet
      %read back data
      bufferLen=pnet(socket,'readpacket');
      header=pnet(socket,'read',14,'uint8','noblock');
      %data=pnet(socket,'read',1000,dataType,'intel');
      header(1:2)=[];%remove dummy
      %test header if it is ok
      if(header(4)~=MGP_OP_CMD_SUCCESS)
          header(4);
          error('communication failed MGP_OP_CMD_FAULT')
      end;
      if(header(8)~=COMMAND_ID)
          header(8);
          error('communication failed: wrong command ID')
      end
   case 'tcp'
      len=ceil(len/2*strcmp(dataType,'double')+...
               len*1*strcmp(dataType,'single')+...
               len*4*strcmp(dataType,'char')+...
               len*4*strcmp(dataType,'int8')+...
               len*4*strcmp(dataType,'uint8')+...
               len*2*strcmp(dataType,'int16')+...
               len*2*strcmp(dataType,'uint16')+...
               len*1*strcmp(dataType,'int32')+...
               len*1*strcmp(dataType,'uint32')+...
               len/2*strcmp(dataType,'int64')+...
               len/2*strcmp(dataType,'uint64'));
      buff=[bitshift(COMMAND,24)+bitshift(lastDsp,16)+bitshift(firstDsp,4) bitshift(1,16)+len startAddress];
      buff=[0;0;typecast(uint32(buff(:)),'uint8');typecast(data(:),'uint8')'];
      [res_data,res_stat]=m2dsmHKLCommand('DSM_MGP',connectionNr,buff);
      if     strcmp(res_stat(1:2),'NO') || strcmp(res_stat(1:3),'BAD')
         error('Invalid LCU reply: %s\n',res_stat);
      end
   otherwise
      error('Invalid trasport socket type');
end



function writePar=getWritePar(startAddress,len)
i=1;
lenLeft=uint32(len);
offset=0;
%allign startaddress
if((lenLeft>7)&&(bitand(startAddress,3)))
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
if ((lenLeft>7)&&(bitand(lenLeft,3)))
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

