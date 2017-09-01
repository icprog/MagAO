function data=mgp_op_rdseq_dsp(firstDsp,lastDsp,len,startAddress,varargin)
% MGP_OP_RDSEQ_DSP(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
% connectionNr: default is 1
% dataType: default is 'uint32'


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004 (Diti)
% xx/xx/xxxx eliminated the DSP memory in range test (needed for compatibility with new ADSP TS201) (Mario)
% xx/xx/xxxx modified by mario to support tcp connection via LCU

MGP_DEFINE();
if firstDsp ~= lastDsp && len>8
    error('not supported for len > 8 and multiple dsp needs to be fixed');
end
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

readPar=getReadPar(startAddress,len);
data=[];
for i=1:length(readPar)
    data=[data;readDsp(firstDsp,lastDsp,readPar(i).l,startAddress+readPar(i).offset,connectionNr,dataType)'];
end

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

% if comunication fails for timeout serveral trys are made 
if(nargin==6)
    livelloRicorsione=1;
else
    livelloRicorsione=varargin{1}+1;
end;
if (livelloRicorsione > 10)
    error('Comunication failed too many retry')
end;
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
      if COMMAND_ID==255
          COMMAND_ID=0;
      end
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
      pnet(socket,'writepacket');   % Send buffer as UDP packet
      %read back data
      bufferLen=pnet(socket,'readpacket');
      header=pnet(socket,'read',14,'uint8','noblock');
      data=pnet(socket,'read',1000,dataType,'intel');
      header(1:2)=[];%remove dummy
      %test header if it is ok
      if header(4)~=MGP_OP_CMD_SUCCESS
         comunicationError=header(4)
         % if there was an error we retry again
         data=readDsp(firstDsp,lastDsp,len,startAddress,connectionNr,dataType,livelloRicorsione);
         return;
      end
      readLen=bitor(uint16(header(5)),bitshift(uint16(header(6)),8));
      if readLen~=len*(lastDsp-firstDsp+1)
         readLen
         error('communication failed: wrong data length');
      end
      if header(8)~=COMMAND_ID
         header(8)
         error('communication failed: wrong command ID')
      end
   case 'tcp'
%       len=ceil(len/2*strcmp(dataType,'double')+...
%                len*1*strcmp(dataType,'single')+...
%                len*4*strcmp(dataType,'char')+...
%                len*4*strcmp(dataType,'int8')+...
%                len*4*strcmp(dataType,'uint8')+...
%                len*2*strcmp(dataType,'int16')+...
%                len*2*strcmp(dataType,'uint16')+...
%                len*1*strcmp(dataType,'int32')+...
%                len*1*strcmp(dataType,'uint32')+...
%                len/2*strcmp(dataType,'int64')+...
%                len/2*strcmp(dataType,'uint64'));
      type_len=0.5*strcmp(dataType,'double')+...
               1*strcmp(dataType,'single')+...
               4*strcmp(dataType,'char')+...
               4*strcmp(dataType,'int8')+...
               4*strcmp(dataType,'uint8')+...
               2*strcmp(dataType,'int16')+...
               2*strcmp(dataType,'uint16')+...
               1*strcmp(dataType,'int32')+...
               1*strcmp(dataType,'uint32')+...
               0.5*strcmp(dataType,'int64')+...
               0.5*strcmp(dataType,'uint64');
      buff=[bitshift(COMMAND,24)+bitshift(lastDsp,16)+bitshift(firstDsp,4) bitshift(1,16)+len startAddress];
      buff=[0 0 typecast(uint32(buff),'uint8')];
      [data,res_stat]=m2dsmHKLCommand('DSM_MGP',connectionNr,buff);
      if     strcmp(res_stat(1:2),'NO') || strcmp(res_stat(1:3),'BAD')
         error('Invalid LCU reply: %s\n',res_stat);
      end
      data=typecast(data(:),dataType);
      data=reshape(data,len*type_len,lastDsp-firstDsp+1);
      % for back compatibility with the udp transport socket reshape the output data as a unidimensional vertical vector
      data=data(:)';
   otherwise
   error('Invalid trasport socket type');
end

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

