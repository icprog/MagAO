function mgp_op_wrsame_sram(firstDsp,lastDsp,len,startAddress,data,varargin)
% mgp_op_wrsame_sram(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
% [connectionNr]: default is 1
% [dataType]:     default is 'uint32'
% len must be less then 364 dword
%
% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
%            modified by mario to support tcp connection via LCU

MGP_DEFINE();

%get parameters
firstDsp=getNumeric(firstDsp);
lastDsp=getNumeric(lastDsp);
len=getNumeric(len);
startAddress=getNumeric(startAddress);
connectionNr=1;
dataType='uint32';
data=getNumeric(data);

%check variable parameters
if nargin==6
   if(isnumeric(varargin{1}))
      connectionNr=varargin{1};
      dataType='uint32';
   else
      connectionNr=1;
      dataType=varargin{1};
   end
elseif nargin==7
   connectionNr=varargin{1};
   dataType=varargin{2};
elseif nargin>7
   error('wrong number of parameters');
end

%convert data to datatype
data=eval([dataType,'(data)']);

[socket,trasport]=getSocket(connectionNr);

switch lower(trasport)
   case 'udp'
      pnet(socket,'WRITEMGPPACKET',firstDsp,lastDsp,startAddress,146,1,len,data,'intel');
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
      if length(data)~=len
         error('Invalid data buffer size\n');
      end
      data=data(:);
      counterAddr=startAddress;
      for startPtr=1:MAX_DWORD_LEN:len
         endPtr=min(startPtr+MAX_DWORD_LEN-1,len);
         packetLen=min(MAX_DWORD_LEN,endPtr-startPtr+1);
%          if len>MAX_DWORD_LEN
%             error(['len must be <= ',num2str(MAX_DWORD_LEN)]);
%          end
         buff=[bitshift(146,24)+bitshift(lastDsp,16)+bitshift(firstDsp,4) bitshift(1,16)+packetLen counterAddr data(startPtr:endPtr)'];
         buff=[0 0 typecast(uint32(buff),'uint8')];
         [res_data,res_stat]=m2dsmHKLCommand('DSM_MGP',connectionNr,buff);
         if strcmp(res_stat(1:2),'NO') || strcmp(res_stat(1:3),'BAD')
            error('Invalid LCU reply: %s\n',res_stat);
         end
         counterAddr=counterAddr+MAX_DWORD_LEN;
      end
   otherwise
      error('Invalid trasport socket type');
end
